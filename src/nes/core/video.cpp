/* FCE Ultra - NES/Famicom Emulator
*
* Copyright notice for this file:
*  Copyright (C) 2002 Xodnizel
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License,  or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful, 
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not,  write to the Free Software
* Foundation,  Inc.,  59 Temple Place,  Suite 330,  Boston,  MA  02111-1307  USA
*/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "types.h"
#include "video.h"
#include "fceu.h"
#include "file.h"
#include "utils/memory.h"
#include "utils/crc32.h"
#include "state.h"
#include "palette.h"
#include "input.h"
#include "vsuni.h"
#include "drawing.h"
#include "driver.h"

#ifdef CREATE_AVI
#include "drivers/videolog/nesvideos-piece.h"
#endif

uint8 *XBuf=NULL;
uint8 *XBackBuf=NULL;
int ClipSidesOffset=0;	//Used to move displayed messages when Clips left and right sides is checked
static uint8 *xbsave=NULL;

GUIMESSAGE guiMessage;
GUIMESSAGE subtitleMessage;

int FCEU_InitVirtualVideo(void)
{
	if(!XBuf)
	{
		XBuf = (uint8*) (FCEU_malloc(256 * 256 + 16));
		XBackBuf = (uint8*) (FCEU_malloc(256 * 256 + 16));

		if(!XBuf || !XBackBuf)
		{
			return 0;
		}
	}

	xbsave = XBuf;

	//if( sizeof(uint8*) == 4 )
	{
		uintptr_t m = (uintptr_t)XBuf;
		m = ( 8 - m) & 7;
		XBuf += m;
	}

	memset(XBuf, 128, 256*256); //*240);
	memset(XBackBuf, 128, 256*256);

	return 1;
}

void FCEU_PutImage(void)
{
	memcpy(XBackBuf,  XBuf,  256*256);
}

void FCEU_DispMessageOnMovie(char *format,  ...)
{
	va_list ap;

	va_start(ap, format);
	vsnprintf(guiMessage.errmsg, sizeof(guiMessage.errmsg), format, ap);
	va_end(ap);

	guiMessage.howlong = 180;
	guiMessage.isMovieMessage = true;
}

void FCEU_ResetMessages()
{
	guiMessage.howlong = 0;
	guiMessage.isMovieMessage = false;
}
