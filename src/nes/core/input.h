#ifndef _INPUT_H_
#define _INPUT_H_

#include <ostream>

#include "git.h"

//MBG TODO - COMBINE THESE INPUTC AND INPUTCFC

//The interface for standard joystick port device drivers
struct INPUTC
{
	//these methods call the function pointers (or not, if they are null)
	uint8 Read(int w) { if(_Read) return _Read(w); else return 0; }
	void Write(uint8 w) { if(_Write) _Write(w); }
	void Strobe(int w) { if(_Strobe) _Strobe(w); }
	void Update(int w, void *data, int arg) { if(_Update) _Update(w, data, arg); }
	void SLHook(int w, uint8 *bg, uint8 *spr, uint32 linets, int final) { if(_SLHook) _SLHook(w, bg, spr, linets, final); }

	uint8 (*_Read)(int w);
	void (*_Write)(uint8 v);
	void (*_Strobe)(int w);
	//update will be called if input is coming from the user. refresh your logical state from user input devices
	void (*_Update)(int w, void *data, int arg);
	void (*_SLHook)(int w, uint8 *bg, uint8 *spr, uint32 linets, int final);

};

//The interface for the expansion port device drivers
struct INPUTCFC
{
	//these methods call the function pointers (or not, if they are null)
	uint8 Read(int w, uint8 ret) { if(_Read) return _Read(w, ret); else return ret; }
	void Write(uint8 v) { if(_Write) _Write(v); }
	void Strobe() { if(_Strobe) _Strobe(); }
	void Update(void *data, int arg) { if(_Update) _Update(data, arg); }
	void SLHook(uint8 *bg, uint8 *spr, uint32 linets, int final) { if(_SLHook) _SLHook(bg, spr, linets, final); }
	void Draw(uint8 *buf, int arg) { if(_Draw) _Draw(buf, arg); }

	uint8 (*_Read)(int w, uint8 ret);
	void (*_Write)(uint8 v);
	void (*_Strobe)();
	//update will be called if input is coming from the user. refresh your logical state from user input devices
	void (*_Update)(void *data, int arg);
	void (*_SLHook)(uint8 *bg, uint8 *spr, uint32 linets, int final);
	void (*_Draw)(uint8 *buf, int arg);

};

extern struct JOYPORT
{
	JOYPORT(int _w)
		: w(_w) 
	{}

	int w;
	int attrib;
	ESI type;
	void* ptr;
	INPUTC* driver;

} joyports[2];

extern struct FCPORT
{
	int attrib;
	ESIFC type;
	void* ptr;
	INPUTCFC* driver;
} portFC;


void FCEU_UpdateInput(void);
void InitializeInput(void);
void FCEU_UpdateBot(void);
extern void (*PStrobe[2])(void);

//called from PPU on scanline events.
extern void InputScanlineHook(uint8 *bg, uint8 *spr, uint32 linets, int final);

void FCEU_DoSimpleCommand(int cmd);

extern const char* FCEUI_CommandTypeNames[];

typedef void EMUCMDFN(void);

enum EMUCMDFLAG
{
	EMUCMDFLAG_NONE = 0, 
	EMUCMDFLAG_TASEDIT = 1, 
};

struct EMUCMDTABLE
{
	int cmd;
	int type;
	EMUCMDFN* fn_on;
	EMUCMDFN* fn_off;
	int state;
	char* name;
	int flags; //EMUCMDFLAG
};

extern struct EMUCMDTABLE FCEUI_CommandTable[];

#endif //_INPUT_H_

