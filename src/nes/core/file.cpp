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

#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "types.h"
#include "file.h"
#include "utils/endian.h"
#include "utils/memory.h"
#include "utils/md5.h"
#include "driver.h"
#include "types.h"
#include "fceu.h"
#include "state.h"
#include "driver.h"
//#include "utils/xstring.h"
#include "utils/memorystream.h"

using namespace std;

bool bindSavestate = true;	//Toggle that determines if a savestate filename will include the movie filename
static std::string BaseDirectory;
static char FileExt[2048];	//Includes the . character, as in ".nes"
char FileBase[2048];
static char FileBaseDirectory[2048];


FileBaseInfo CurrentFileBase() {
	return FileBaseInfo(FileBaseDirectory, FileBase, FileExt);
}

//FileBaseInfo DetermineFileBase(const char *f) {
//
//	char drv[PATH_MAX], dir[PATH_MAX], name[PATH_MAX], ext[PATH_MAX];
//	splitpath(f, drv, dir, name, ext);
//
//        if(dir[0] == 0) strcpy(dir, ".");
//
//	return FileBaseInfo((std::string)drv + dir, name, ext);
//
//}

//inline FileBaseInfo DetermineFileBase(const std::string& str) { return DetermineFileBase(str.c_str()); }

FCEUFILE * FCEU_fopen(std::iostream* stream, const char *ipsfn, char *mode, char *ext, int index, const char** extensions)
{
	FILE *ipsfile=0;
	FCEUFILE *fceufp=0;

	bool read = (std::string)mode == "rb";
	bool write = (std::string)mode == "wb";
	if(read&&write || (!read&&!write))
	{
		return 0;
	}

	std::string archive, fname, fileToOpen;

	if(read)
	{
		{
			//if the archive contained no files, try to open it the old fashioned way
			std::iostream* fp = stream;//FCEUD_UTF8_fstream(fileToOpen, mode);
			if(!fp)
			{
				return 0;
			}

			//open a plain old file
			fceufp = new FCEUFILE();
			fceufp->stream = (std::iostream*)fp;
			FCEU_fseek(fceufp, 0, SEEK_END);
			fceufp->size = FCEU_ftell(fceufp);
			FCEU_fseek(fceufp, 0, SEEK_SET);
		}

		return fceufp;
	}
	return 0;
}

int FCEU_fclose(FCEUFILE *fp)
{
//	delete fp;
	return 1;
}

uint64 FCEU_fread(void *ptr, size_t size, size_t nmemb, FCEUFILE *fp)
{
	fp->stream->read((char*)ptr, size*nmemb);
	uint32 read = fp->stream->gcount();
	return read/size;
}

uint64 FCEU_fwrite(void *ptr, size_t size, size_t nmemb, FCEUFILE *fp)
{
	fp->stream->write((char*)ptr, size*nmemb);
	//todo - how do we tell how many bytes we wrote?
	return nmemb;
}

int FCEU_fseek(FCEUFILE *fp, long offset, int whence)
{
	//if(fp->type==1)
//{
	//	return( (gzseek(fp->fp, offset, whence)>0)?0:-1);
//}
	//else if(fp->type>=2)
	//{
	//	MEMWRAP *wz;
	//	wz=(MEMWRAP*)fp->fp;

	//	switch(whence)
//{
	//	case SEEK_SET:if(offset>=(long)wz->size) //mbg merge 7/17/06 - added cast to long
	//					  return(-1);
	//		wz->location=offset;break;
	//	case SEEK_CUR:if(offset+wz->location>wz->size)
	//					  return (-1);
	//		wz->location+=offset;
	//		break;
	//	}
	//	return 0;
//}
	//else
	//	return fseek((FILE *)fp->fp, offset, whence);

	fp->stream->seekg(offset, (std::ios_base::seekdir)whence);
	fp->stream->seekp(offset, (std::ios_base::seekdir)whence);

	return FCEU_ftell(fp);
}

uint64 FCEU_ftell(FCEUFILE *fp)
{
	if(fp->mode == FCEUFILE::READ)
		return fp->stream->tellg();
	else
		return fp->stream->tellp();
}

int FCEU_read16le(uint16 *val, FCEUFILE *fp)
{
	return read16le(val, fp->stream);
}

int FCEU_read32le(uint32 *Bufo, FCEUFILE *fp)
{
	return read32le(Bufo, fp->stream);
}

int FCEU_fgetc(FCEUFILE *fp)
{
	return fp->stream->get();
}

uint64 FCEU_fgetsize(FCEUFILE *fp)
{
	return fp->size;
}

//int FCEU_fisarchive(FCEUFILE *fp)
//{
//	if(fp->archiveIndex==0) return 0;
//	else return 1;
//}

/// Updates the base directory
void FCEUI_SetBaseDirectory(std::string const & dir)
{
	BaseDirectory = dir;
}

static char *odirs[FCEUIOD__COUNT]={0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};     // odirs, odors. ^_^

void FCEUI_SetDirOverride(int which, char *n)
{
	if (which < FCEUIOD__COUNT)
	{
		odirs[which] = n;
	}
}

//	#ifndef HAVE_ASPRINTF
//	static int asprintf(char **strp, const char *fmt, ...)
//	{
//		va_list ap;
//		int ret;
//
//		va_start(ap, fmt);
//		if(!(*strp=(char*)malloc(2048))) //mbg merge 7/17/06 cast to char*
//			return(0);
//		ret=vsnprintf(*strp, 2048, fmt, ap);
//		va_end(ap);
//		return(ret);
//	}
//	#endif

std::string  FCEU_GetPath(int type)
{
	char ret[FILENAME_MAX];
	switch(type)
	{
		case FCEUMKF_STATE:
			if(odirs[FCEUIOD_STATES])
				return (odirs[FCEUIOD_STATES]);
			else
				return BaseDirectory + PSS + "fcs";
			break;
		case FCEUMKF_MEMW:
			if(odirs[FCEUIOD_MEMW])
				return (odirs[FCEUIOD_MEMW]);
			else
				return "";	//adelikat: 03/02/09 - return null so it defaults to last directory used
				//return BaseDirectory + PSS + "tools";
			break;
		//adelikat: TODO: this no longer exist and could be removed (but that would require changing a lot of other directory arrays
		case FCEUMKF_BBOT:
			if(odirs[FCEUIOD_BBOT])
				return (odirs[FCEUIOD_BBOT]);
			else
				return BaseDirectory + PSS + "tools";
			break;
		case FCEUMKF_ROMS:
			if(odirs[FCEUIOD_ROMS])
				return (odirs[FCEUIOD_ROMS]);
			else
				return "";	//adelikat: removing base directory return, should return null it goes to last used directory
			break;
		case FCEUMKF_INPUT:
			if(odirs[FCEUIOD_INPUT])
				return (odirs[FCEUIOD_INPUT]);
			else
				return BaseDirectory + PSS + "tools";
			break;
	}

	return ret;
}

std::string FCEU_MakePath(int type, const char* filebase)
{
	char ret[FILENAME_MAX];

	switch(type)
	{
		case FCEUMKF_STATE:
			if(odirs[FCEUIOD_STATES])
				return (string)odirs[FCEUIOD_STATES] + PSS + filebase;
			else
				return BaseDirectory + PSS + "fcs" + PSS + filebase;
			break;
	}
	return ret;
}

std::string FCEU_MakeFName(int type, int id1, const char *cd1)
{
	char ret[FILENAME_MAX] = "";
	struct stat tmpstat;
	std::string mfnString;
	const char* mfn;

	switch(type)
	{
		case FCEUMKF_STATE:
			{
				mfnString = "";

				if (mfnString.length() < 60)	//This caps the movie filename length before adding it to the savestate filename.
					mfn = mfnString.c_str();	//This helps prevent possible crashes from savestate filenames of excessive length.

				else
					{
					std::string mfnStringTemp = mfnString.substr(0, 60);
					mfn = mfnStringTemp.c_str();	//mfn is the movie filename
					}



				if(odirs[FCEUIOD_STATES])
				{
					//sprintf(ret, "%s"PSS"%s%s.fc%d", odirs[FCEUIOD_STATES], FileBase, mfn, id1);
				}
				else
				{
					//sprintf(ret, "%s"PSS"fcs"PSS"%s%s.fc%d", BaseDirectory.c_str(), FileBase, mfn, id1);
				}
				if(stat(ret, &tmpstat)==-1)
				{
					if(odirs[FCEUIOD_STATES])
					{
						//sprintf(ret, "%s"PSS"%s%s.fc%d", odirs[FCEUIOD_STATES], FileBase, mfn, id1);
					}
					else
					{
						//sprintf(ret, "%s"PSS"fcs"PSS"%s%s.fc%d", BaseDirectory.c_str(), FileBase, mfn, id1);
					}
				}
			}
			break;
		case FCEUMKF_SAV:
			//if(odirs[FCEUIOD_NV])
				//sprintf(ret, "%s"PSS"%s.%s", odirs[FCEUIOD_NV], FileBase, cd1);
			//else
				//sprintf(ret, "%s"PSS"sav"PSS"%s.%s", BaseDirectory.c_str(), FileBase, cd1);
			if(stat(ret, &tmpstat)==-1)
			{
				//if(odirs[FCEUIOD_NV])
					//sprintf(ret, "%s"PSS"%s.%s", odirs[FCEUIOD_NV], FileBase, cd1);
				//else
					//sprintf(ret, "%s"PSS"sav"PSS"%s.%s", BaseDirectory.c_str(), FileBase, cd1);
			}
			break;
		case FCEUMKF_AUTOSTATE:
			mfnString = "Autosave";
			mfn = mfnString.c_str();
			if(odirs[FCEUIOD_STATES])
			{
				//sprintf(ret, "%s"PSS"%s%s-autosave%d.fcs", odirs[FCEUIOD_STATES], FileBase, mfn, id1);
			}
			else
			{
				//sprintf(ret, "%s"PSS"fcs"PSS"%s%s-autosave%d.fcs", BaseDirectory.c_str(), FileBase, mfn, id1);
			}
			if(stat(ret, &tmpstat)==-1)
			{
				if(odirs[FCEUIOD_STATES])
				{
					//sprintf(ret, "%s"PSS"%s%s-autosave%d.fcs", odirs[FCEUIOD_STATES], FileBase, mfn, id1);
				}
				else
				{
					//sprintf(ret, "%s"PSS"fcs"PSS"%s%s-autosave%d.fcs", BaseDirectory.c_str(), FileBase, mfn, id1);
				}
			}
			break;
		case FCEUMKF_PALETTE://sprintf(ret, "%s"PSS"%s.pal", BaseDirectory.c_str(), FileBase);break;
		case FCEUMKF_STATEGLOB:
			//if(odirs[FCEUIOD_STATES])
				//sprintf(ret, "%s"PSS"%s*.fc?", odirs[FCEUIOD_STATES], FileBase);
			//else
				//sprintf(ret, "%s"PSS"fcs"PSS"%s*.fc?", BaseDirectory.c_str(), FileBase);
			break;
	}

	//convert | to . for archive filenames.
	return ret;//mass_replace(ret, "|", ".");
}

//void GetFileBase(const char *f)
//{
//	FileBaseInfo fbi = DetermineFileBase(f);
//	strcpy(FileBase, fbi.filebase.c_str());
//	strcpy(FileBaseDirectory, fbi.filebasedirectory.c_str());
//}

//bool FCEU_isFileInArchive(const char *path)
//{
//	bool isarchive = false;
//	FCEUFILE* fp = FCEU_fopen(path, 0, "rb", 0, 0);
//	if(fp) {
//		isarchive = fp->isArchive();
//		delete fp;
//	}
//	return isarchive;
//}



//void FCEUARCHIVEFILEINFO::FilterByExtension(const char** ext)
//{
//	if(!ext) return;
//	int count = size();
//	for(int i=count-1;i>=0;i--) {
//		std::string fext = getExtension((*this)[i].name.c_str());
//		const char** currext = ext;
//		while(*currext) {
//			if(fext == *currext)
//				goto ok;
//			currext++;
//		}
//		this->erase(begin()+i);
//	ok: ;
//	}
//}
