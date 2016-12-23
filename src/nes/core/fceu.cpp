/* FCE Ultra - NES/Famicom Emulator
*
* Copyright notice for this file:
*  Copyright (C) 2003 Xodnizel
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

#include <string>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include "types.h"
#include "x6502.h"
#include "fceu.h"
#include "ppu.h"
#include "sound.h"
#include "file.h"
#include "utils/endian.h"
#include "utils/memory.h"
#include "utils/crc32.h"

#include "cart.h"
#include "ines.h"
#include "unif.h"
#include "palette.h"
#include "state.h"
#include "video.h"
#include "input.h"
#include "file.h"
#include "vsuni.h"
#include "ines.h"

#include <sstream>

using namespace std;

int AFon = 1, AFoff = 1, AutoFireOffset = 0; //For keeping track of autofire settings
bool justLagged = false;
bool AutoSS = false;		//Flagged true when the first auto-savestate is made while a game is loaded, flagged false on game close
bool movieSubtitles = true; //Toggle for displaying movie subtitles

FCEUGI::FCEUGI()
: filename(0)
, archiveFilename(0)
{
	printf("%08x", opsize);
}

FCEUGI::~FCEUGI()
{
	if(filename) delete filename;
	if(archiveFilename) delete archiveFilename;
}

static void CloseGame(void)
{
	if(GameInfo)
	{
		if(GameInfo->name)
		{
			free(GameInfo->name);
			GameInfo->name=0;
		}

		GameInterface(GI_CLOSE);

		ResetExState(0, 0);

		//mbg 5/9/08 - clear screen when game is closed
		//http://sourceforge.net/tracker/index.php?func=detail&aid=1787298&group_id=13536&atid=113536
		extern uint8 *XBuf;
		if(XBuf)
			memset(XBuf, 0, 256*256);

		delete GameInfo;
		GameInfo = 0;

		//Reset flags for Undo/Redo/Auto Savestating
		//lastSavestateMade[0] = 0;
		//undoSS = false;
		//redoSS = false;
		//lastLoadstateMade[0] = 0;
		//undoLS = false;
		//redoLS = false;
		//AutoSS = false;
	}
}


uint64 timestampbase;


FCEUGI *GameInfo = 0;

void (*GameInterface)(GI h);
void (*GameStateRestore)(int version);

readfunc ARead[0x10000];
writefunc BWrite[0x10000];
static readfunc *AReadG;
static writefunc *BWriteG;
static int RWWrap=0;

static DECLFW(BNull)
{

}

static DECLFR(ANull)
{
	return(X.DB);
}

readfunc GetReadHandler(int32 a)
{
	if(a>=0x8000 && RWWrap)
		return AReadG[a-0x8000];
	else
		return ARead[a];
}

void SetReadHandler(int32 start, int32 end, readfunc func)
{
	int32 x;

	if(!func)
		func=ANull;

	if(RWWrap)
		for(x=end;x>=start;x--)
		{
			if(x>=0x8000)
				AReadG[x-0x8000]=func;
			else
				ARead[x]=func;
		}
	else

		for(x=end;x>=start;x--)
			ARead[x]=func;
}

writefunc GetWriteHandler(int32 a)
{
	if(RWWrap && a>=0x8000)
		return BWriteG[a-0x8000];
	else
		return BWrite[a];
}

void SetWriteHandler(int32 start, int32 end, writefunc func)
{
	int32 x;

	if(!func)
		func=BNull;

	if(RWWrap)
		for(x=end;x>=start;x--)
		{
			if(x>=0x8000)
				BWriteG[x-0x8000]=func;
			else
				BWrite[x]=func;
		}
	else
		for(x=end;x>=start;x--)
			BWrite[x]=func;
}

uint8 *GameMemBlock;
uint8 *RAM;

//---------
//windows might need to allocate these differently, so we have some special code

static void AllocBuffers()
{

	GameMemBlock = (uint8*)FCEU_gmalloc(131072);
	RAM = (uint8*)FCEU_gmalloc(0x800);

}

static void FreeBuffers() {
	FCEU_free(GameMemBlock);
	FCEU_free(RAM);
}
//------

uint8 PAL=0;

static DECLFW(BRAML)
{
	RAM[A]=V;
}

static DECLFW(BRAMH)
{
	RAM[A&0x7FF]=V;
}

static DECLFR(ARAML)
{
	return RAM[A];
}

static DECLFR(ARAMH)
{
	return RAM[A&0x7FF];
}


void ResetGameLoaded(void)
{
	if(GameInfo) CloseGame();
	GameStateRestore=0;
	PPU_hook=0;
	GameHBIRQHook=0;
	FFCEUX_PPURead = 0;
	FFCEUX_PPUWrite = 0;
	MapIRQHook=0;
	MMC5Hack=0;
	PAL&=1;
	pale=0;
}

int iNESLoad(FCEUFILE *fp, int OverwriteVidMode);

FCEUGI *FCEUI_LoadGameVirtual(std::iostream* stream, int OverwriteVidMode)
{
	//mbg merge 7/17/07 - why is this here
	//#ifdef WIN32
	//	StopSound();
	//#endif

	//----------
	//attempt to open the files
	FCEUFILE *fp;

	const char* romextensions[] = {"nes", "fds", 0};

	fp = FCEU_fopen(stream, 0, "rb", 0, -1, romextensions);

	if(!fp)
	{
		return 0;
	}

	ResetGameLoaded();

	CloseGame();
	GameInfo = new FCEUGI();
	memset(GameInfo, 0, sizeof(FCEUGI));

	GameInfo->soundchan = 0;
	GameInfo->soundrate = 0;
	GameInfo->name=0;
	GameInfo->type=GIT_CART;
	GameInfo->vidsys=GIV_USER;
	GameInfo->input[0]=GameInfo->input[1]=SI_UNSET;
	GameInfo->inputfc=SIFC_UNSET;
	GameInfo->cspecial=SIS_NONE;

	if(iNESLoad(fp, OverwriteVidMode))
		goto endlseq;

	delete GameInfo;
	GameInfo = 0;

	return 0;

endlseq:

	FCEU_ResetVidSys();

	PowerNES();

	FCEU_ResetPalette();
	FCEU_ResetMessages();	// Save state, status messages, etc.

	return GameInfo;
}

FCEUGI *FCEUI_LoadGame(const char *name, int OverwriteVidMode)
{
	return NULL;//FCEUI_LoadGameVirtual(name, OverwriteVidMode);
}


//Return: Flag that indicates whether the function was succesful or not.
bool FCEUI_Initialize()
{
	srand(time(0));

	if(!FCEU_InitVirtualVideo())
	{
		return false;
	}

	AllocBuffers();

	// Initialize some parts of the settings structure
	//mbg 5/7/08 - I changed the ntsc settings to match pal.
	//this is more for precision emulation, instead of entertainment, which is what fceux is all about nowadays
	memset(&FSettings, 0, sizeof(FSettings));
	//FSettings.UsrFirstSLine[0]=8;
	FSettings.UsrFirstSLine[0]=0;
	FSettings.UsrFirstSLine[1]=0;
	//FSettings.UsrLastSLine[0]=231;
	FSettings.UsrLastSLine[0]=239;
	FSettings.UsrLastSLine[1]=239;

	FCEUPPU_Init();

	X6502_Init();

	return true;
}

void FCEUI_Kill(void)
{
	FreeBuffers();
}

///Emulates a single frame.

void FCEUI_EmulateSound(int32 **SoundBuf, int32 *SoundBufSize)
{
	int ssize;

	ssize=FlushEmulateSound();

	timestampbase += timestamp;
	timestamp = 0;

	*SoundBuf=WaveFinal;
	*SoundBufSize=ssize;
}

///Skip may be passed in, if FRAMESKIP is #defined, to cause this to emulate more than one frame
void FCEUI_Emulate(uint8 **pXBuf, int skip)
{
	FCEU_UpdateInput();
	FCEUPPU_Loop(skip);

	*pXBuf = XBuf;
}

void FCEUI_CloseGame(void)
{
	CloseGame();
}

void ResetNES(void)
{
	//FCEUMOV_AddCommand(FCEUNPCMD_RESET);
	if(!GameInfo) return;
	GameInterface(GI_RESETM2);
	FCEUSND_Reset();
	FCEUPPU_Reset();
	X6502_Reset();

	// clear back baffer
	extern uint8 *XBackBuf;
	memset(XBackBuf, 0, 256*256);
}

void FCEU_MemoryRand(uint8 *ptr, uint32 size)
{
	int x=0;
	while(size)
	{
		*ptr=(x&4)?0xFF:0x00;
		x++;
		size--;
		ptr++;
	}
}

void hand(X6502 *X, int type, unsigned int A)
{

}

int suppressAddPowerCommand=0; // hack... yeah, I know...
void PowerNES(void)
{
	//void MapperInit();
	//MapperInit();

	if(!suppressAddPowerCommand)
		//FCEUMOV_AddCommand(FCEUNPCMD_POWER);
	if(!GameInfo) return;

	FCEU_MemoryRand(RAM, 0x800);
	//memset(RAM, 0xFF, 0x800);

	SetReadHandler(0x0000, 0xFFFF, ANull);
	SetWriteHandler(0x0000, 0xFFFF, BNull);

	SetReadHandler(0, 0x7FF, ARAML);
	SetWriteHandler(0, 0x7FF, BRAML);

	SetReadHandler(0x800, 0x1FFF, ARAMH); // Part of a little
	SetWriteHandler(0x800, 0x1FFF, BRAMH); //hack for a small speed boost.

	InitializeInput();
	FCEUSND_Power();
	FCEUPPU_Power();

	//Have the external game hardware "powered" after the internal NES stuff.  Needed for the NSF code and VS System code.
	GameInterface(GI_POWER);
	if(GameInfo->type==GIT_VSUNI)
		FCEU_VSUniPower();

	//if we are in a movie, then reset the saveram
	extern int disableBatteryLoading;
	if(disableBatteryLoading)
		GameInterface(GI_RESETSAVE);


	timestampbase=0;

	X6502_Power();
	// clear back baffer
	extern uint8 *XBackBuf;
	memset(XBackBuf, 0, 256*256);
}

void FCEU_ResetVidSys(void)
{
	int w;

	if(GameInfo->vidsys==GIV_NTSC)
		w=0;
	else if(GameInfo->vidsys==GIV_PAL)
		w=1;
	else
		w=FSettings.PAL;

	PAL=w?1:0;
	FCEUPPU_SetVideoSystem(w);
	SetSoundVariables();
}

FCEUS FSettings;

void FCEUI_SetRenderedLines(int ntscf, int ntscl, int palf, int pall)
{
	FSettings.UsrFirstSLine[0]=ntscf;
	FSettings.UsrLastSLine[0]=ntscl;
	FSettings.UsrFirstSLine[1]=palf;
	FSettings.UsrLastSLine[1]=pall;
	if(PAL)
	{
		FSettings.FirstSLine=FSettings.UsrFirstSLine[1];
		FSettings.LastSLine=FSettings.UsrLastSLine[1];
	}
	else
	{
		FSettings.FirstSLine=FSettings.UsrFirstSLine[0];
		FSettings.LastSLine=FSettings.UsrLastSLine[0];
	}

}

void FCEUI_SetVidSystem(int a)
{
	FSettings.PAL=a?1:0;
	if(GameInfo)
	{
		FCEU_ResetVidSys();
		FCEU_ResetPalette();
	}
}

int FCEUI_GetCurrentVidSystem(int *slstart, int *slend)
{
	if(slstart)
		*slstart=FSettings.FirstSLine;
	if(slend)
		*slend=FSettings.LastSLine;
	return(PAL);
}

//this variable isn't used at all, snap is always name-based
//void FCEUI_SetSnapName(bool a)
//{
//	FSettings.SnapName = a;
//}

int32 FCEUI_GetDesiredFPS(void)
{
	if(PAL)
		return(838977920); // ~50.007
	else
		return(1008307711);	// ~60.1
}

int FCEU_TextScanlineOffset(int y)
{
	return FSettings.FirstSLine*256;
}
int FCEU_TextScanlineOffsetFromBottom(int y)
{
	return (FSettings.LastSLine-y)*256;
}

//---------------------
//experimental new mapper and ppu system follows

class FCEUXCart {
public:
	int mirroring;
	int chrPages, prgPages;
	uint32 chrSize, prgSize;
	char* CHR, *PRG;

	FCEUXCart()
		: CHR(0)
		, PRG(0)
	{}

	~FCEUXCart() {
		if(CHR) delete[] CHR;
		if(PRG) delete[] PRG;
	}

	virtual void Power() {
	}

protected:
	//void SetReadHandler(int32 start, int32 end, readfunc func) {
};

FCEUXCart* cart = 0;

//uint8 Read_ByteFromRom(uint32 A) {
//	if(A>=cart->prgSize) return 0xFF;
//	return cart->PRG[A];
//}
//
//uint8 Read_Unmapped(uint32 A) {
//	return 0xFF;
//}



class NROM : FCEUXCart {
public:
	virtual void Power() {
		SetReadHandler(0x8000, 0xFFFF, CartBR);
		setprg16(0x8000, 0);
		setprg16(0xC000, ~0);
		setchr8(0);

		vnapage[0] = NTARAM;
		vnapage[2] = NTARAM;
		vnapage[1] = NTARAM+0x400;
		vnapage[3] = NTARAM+0x400;
		PPUNTARAM=0xF;
	}
};

void FCEUXGameInterface(GI command) {
	switch(command) {
		case GI_POWER:
			cart->Power();
	}
}

uint8 FCEU_ReadRomByte(uint32 i) {
	extern iNES_HEADER head;
	if(i < 16) return *((unsigned char *)&head+i);
	if(i < 16+PRGsize[0])return PRGptr[0][i-16];
	if(i < 16+PRGsize[0]+CHRsize[0])return CHRptr[0][i-16-PRGsize[0]];
	return 0;
}
