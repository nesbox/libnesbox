/* FCE Ultra - NES/Famicom Emulator
*
* Copyright notice for this file:
*  Copyright (C) 2002 Xodnizel
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

//  TODO: Add (better) file io error checking

#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <vector>
#include <fstream>

#include "types.h"
#include "x6502.h"
#include "fceu.h"
#include "sound.h"
#include "utils/endian.h"
#include "utils/memory.h"
#include "utils/memorystream.h"
#include "file.h"
#include "state.h"
#include "ppu.h"
#include "video.h"
#include "input.h"
#include "driver.h"

using namespace std;

static void (*SPreSave)(void);
static void (*SPostSave)(void);

#define SFMDATA_SIZE (64)
static SFORMAT SFMDATA[SFMDATA_SIZE];
static int SFEXINDEX;

#define RLSB 		FCEUSTATE_RLSB	//0x80000000


extern SFORMAT FCEUPPU_STATEINFO[];
extern SFORMAT FCEUSND_STATEINFO[];

SFORMAT SFCPU[]={
	{ &X.PC, 2|RLSB, "PC\0"},
	{ &X.A, 1, "A\0\0"},
	{ &X.P, 1, "P\0\0"},
	{ &X.X, 1, "X\0\0"},
	{ &X.Y, 1, "Y\0\0"},
	{ &X.S, 1, "S\0\0"},
	{ &RAM, 0x800 | FCEUSTATE_INDIRECT, "RAM", },
	{ 0 }
};

SFORMAT SFCPUC[]={
	{ &X.jammed, 1, "JAMM"},
	{ &X.IRQlow, 4|RLSB, "IQLB"},
	{ &X.tcount, 4|RLSB, "ICoa"},
	{ &X.count,  4|RLSB, "ICou"},
	{ &timestampbase, sizeof(timestampbase) | RLSB, "TSBS"},
	{ &X.mooPI, 1, "MooP"}, // alternative to the "quick and dirty hack"
	{ 0 }
};

void foo(uint8* test) { (void)test; }

static int SubWrite(std::ostream* os, SFORMAT *sf)
{
	uint32 acc=0;

	while(sf->v)
	{
		if(sf->s==~0)		//Link to another struct
		{
			uint32 tmp;

			if(!(tmp=SubWrite(os,(SFORMAT *)sf->v)))
				return(0);
			acc+=tmp;
			sf++;
			continue;
		}

		acc+=8;			//Description + size
		acc+=sf->s&(~FCEUSTATE_FLAGS);

		if(os)			//Are we writing or calculating the size of this block?
		{
			os->write(sf->desc,4);
			write32le(sf->s&(~FCEUSTATE_FLAGS),os);

			if(sf->s&FCEUSTATE_INDIRECT)
				os->write(*(char **)sf->v,sf->s&(~FCEUSTATE_FLAGS));
			else
				os->write((char*)sf->v,sf->s&(~FCEUSTATE_FLAGS));

		}
		sf++;
	}

	return(acc);
}

static int WriteStateChunk(std::ostream* os, int type, SFORMAT *sf)
{
	os->put(type);
	int bsize = SubWrite((std::ostream*)0,sf);
	write32le(bsize,os);

	if(!SubWrite(os,sf))
	{
		return 5;
	}
	return (bsize+5);
}

static SFORMAT *CheckS(SFORMAT *sf, uint32 tsize, char *desc)
{
	while(sf->v)
	{
		if(sf->s==~0)		// Link to another SFORMAT structure.
		{
			SFORMAT *tmp = CheckS((SFORMAT *)sf->v, tsize, desc) ;

			if(tmp)
			{
				return(tmp);
			}

			sf++;
			continue;
		}
		if(!memcmp(desc,sf->desc,4))
		{
			if(tsize!=(sf->s&(~FCEUSTATE_FLAGS)))
				return(0);
			return(sf);
		}
		sf++;
	}
	return(0);
}

static bool ReadStateChunk(std::istream* is, SFORMAT *sf, int size)
{
	SFORMAT *tmp;
	int temp = is->tellg();

	while(is->tellg()<temp+size)
	{
		uint32 tsize;
		char toa[4];
		if(is->read(toa,4).gcount()<4)
			return false;

		read32le(&tsize,is);

		if((tmp=CheckS(sf,tsize,toa)))
		{
			if(tmp->s&FCEUSTATE_INDIRECT)
				is->read(*(char **)tmp->v,tmp->s&(~FCEUSTATE_FLAGS));
			else
				is->read((char *)tmp->v,tmp->s&(~FCEUSTATE_FLAGS));

		}
		else
			is->seekg(tsize,std::ios::cur);
	} // while(...)
	return true;
}

static int read_sfcpuc=0, read_snd=0;

static bool ReadStateChunks(std::istream* is, int32 totalsize)
{
	int t;
	uint32 size;
	bool ret=true;
	bool warned=false;

	read_sfcpuc=0;
	read_snd=0;

	while(totalsize > 0)
	{
		t=is->get();
		if(t==EOF) break;
		if(!read32le(&size,is)) break;
		totalsize -= size + 5;

		switch(t)
		{
		case 1:if(!ReadStateChunk(is,SFCPU,size)) ret=false;break;
		case 3:if(!ReadStateChunk(is,FCEUPPU_STATEINFO,size)) ret=false;break;
		case 7:
			break;
		case 0x10:if(!ReadStateChunk(is,SFMDATA,size)) ret=false; break;

			// now it gets hackier:
		case 5:
			if(!ReadStateChunk(is,FCEUSND_STATEINFO,size))
				ret=false;
			else
				read_snd=1;
			break;
		case 6:
			{
				is->seekg(size,std::ios::cur);
			}
			break;
		case 2:
			{
				if(!ReadStateChunk(is,SFCPUC,size))
					ret=false;
				else
					read_sfcpuc=1;
			}  break;
		default:
			// for somebody's sanity's sake, at least warn about it:
			if(!warned)
			{
				warned=true;
			}
			//if(fseek(st,size,SEEK_CUR)<0) goto endo;break;
			is->seekg(size,std::ios::cur);
		}
	}


	extern int resetDMCacc;
	if(read_snd)
		resetDMCacc=0;
	else
		resetDMCacc=1;

	return ret;
}

int CurrentState=1;

bool FCEUSS_SaveMS(std::ostream* outstream)
{
	//a temp memory stream. we'll dump some data here and then compress
	//TODO - support dumping directly without compressing to save a buffer copy

	memorystream ms;
	std::ostream* os = (std::ostream*)&ms;

	size_t totalsize = 0;

	FCEUPPU_SaveState();
	FCEUSND_SaveState();
	totalsize=WriteStateChunk(os, 1, SFCPU);
	totalsize+=WriteStateChunk(os, 2, SFCPUC);
	totalsize+=WriteStateChunk(os, 3, FCEUPPU_STATEINFO);
	totalsize+=WriteStateChunk(os, 4, FCEUSND_STATEINFO);

	if(SPreSave) SPreSave();
	totalsize+=WriteStateChunk(os, 0x10, SFMDATA);
	if(SPreSave) SPostSave();

	//save the length of the file
	size_t len = ms.size();

	//sanity check: len and totalsize should be the same
	if(len != totalsize)
	{
		return false;
	}

	uint8* cbuf = (uint8*)ms.buf();

	//dump the header
	uint8 header[16]="FCSX";
	FCEU_en32lsb(header+4, totalsize);
	FCEU_en32lsb(header+8, FCEU_VERSION_NUMERIC);

	//dump it to the destination file
	outstream->write((char*)header, 16);
	outstream->write((char*)cbuf, totalsize);

	if(cbuf != (uint8*)ms.buf())
	{
		delete[] cbuf;
	}

	return true;
}

bool FCEUSS_LoadFP(std::istream* is)
{
	uint8 header[16];

	//read and analyze the header
	is->read((char*)&header, 16);
	if(memcmp(header, "FCSX", 4))
	{
		return false;
	}

	int totalsize = FCEU_de32lsb(header + 4);
	int stateversion = FCEU_de32lsb(header + 8);

	std::vector<char> buf(totalsize);

	is->read((char*)&buf[0], totalsize);

	memorystream mstemp(&buf);
	bool x = ReadStateChunks(&mstemp, totalsize)!=0;

	if(GameStateRestore)
	{
		GameStateRestore(stateversion);
	}
	if(x)
	{
		FCEUPPU_LoadState(stateversion);
		FCEUSND_LoadState(stateversion);
	}

	return x;
}

void ResetExState(void (*PreSave)(void), void (*PostSave)(void))
{
	int x;
	for(x=0;x<SFEXINDEX;x++)
	{
		if(SFMDATA[x].desc)
			free(SFMDATA[x].desc);
	}
	// adelikat, 3/14/09:  had to add this to clear out the size parameter.  NROM(mapper 0) games were having savestate crashes if loaded after a non NROM game	because the size variable was carrying over and causing savestates to save too much data
	SFMDATA[0].s = 0;		
	
	SPreSave = PreSave;
	SPostSave = PostSave;
	SFEXINDEX=0;
}

void AddExState(void *v, uint32 s, int type, char *desc)
{
	if(desc)
	{
		SFMDATA[SFEXINDEX].desc=(char *)FCEU_malloc(5);
		strcpy(SFMDATA[SFEXINDEX].desc,desc);
	}
	else
		SFMDATA[SFEXINDEX].desc=0;
	SFMDATA[SFEXINDEX].v=v;
	SFMDATA[SFEXINDEX].s=s;
	if(type) SFMDATA[SFEXINDEX].s|=RLSB;
	if(SFEXINDEX<SFMDATA_SIZE-1)
		SFEXINDEX++;
	else
	{
		static int once=1;
		if(once)
		{
			once=0;
			//FCEU_PrintError("Error in AddExState: SFEXINDEX overflow.\nSomebody made SFMDATA_SIZE too small.");
		}
	}
	SFMDATA[SFEXINDEX].v=0;		// End marker.
}
