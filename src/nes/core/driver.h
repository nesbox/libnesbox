#ifndef __DRIVER_H_
#define __DRIVER_H_

#include <stdio.h>
#include <string>
#include <iosfwd>

#include "types.h"
#include "git.h"
#include "file.h"

//FILE *FCEUD_UTF8fopen(const char *fn, const char *mode);
//inline FILE *FCEUD_UTF8fopen(const std::string &n, const char *mode) { return FCEUD_UTF8fopen(n.c_str(), mode); }
//std::iostream* FCEUD_UTF8_fstream(const char *n, const char *m);
//inline std::iostream* FCEUD_UTF8_fstream(const std::string &n, const char *m) { return FCEUD_UTF8_fstream(n.c_str(), m); }
//FCEUFILE* FCEUD_OpenArchiveIndex(ArchiveScanRecord& asr, std::string& fname, int innerIndex);
//FCEUFILE* FCEUD_OpenArchive(ArchiveScanRecord& asr, std::string& fname, std::string* innerFilename);
//ArchiveScanRecord FCEUD_ScanArchive(std::string fname);

//Video interface
void FCEUD_SetPalette(uint8 index, uint8 r, uint8 g, uint8 b);

void FCEUI_ResetNES(void);
void FCEUI_PowerNES(void);

void FCEUI_NTSCSELHUE(void);
void FCEUI_NTSCSELTINT(void);
void FCEUI_NTSCDEC(void);
void FCEUI_NTSCINC(void);
void FCEUI_GetNTSCTH(int *tint, int *hue);
void FCEUI_SetNTSCTH(int n, int tint, int hue);

void FCEUI_SetInput(int port, ESI type, void *ptr, int attrib);
void FCEUI_SetInputFC(ESIFC type, void *ptr, int attrib);

//tells the emulator whether a fourscore is attached
void FCEUI_SetInputFourscore(bool attachFourscore);
//tells whether a fourscore is attached
bool FCEUI_GetInputFourscore();


//New interface functions

//0 to order screen snapshots numerically(0.png), 1 to order them file base-numerically(smb3-0.png).
//this variable isn't used at all, snap is always name-based
//void FCEUI_SetSnapName(bool a);

//0 to keep 8-sprites limitation, 1 to remove it
void FCEUI_DisableSpriteLimitation(int a);

void FCEUI_SetRenderPlanes(bool sprites, bool bg);
void FCEUI_GetRenderPlanes(bool& sprites, bool& bg);

//name=path and file to load.  returns null if it failed
FCEUGI *FCEUI_LoadGame(const char *name, int OverwriteVidMode);

//same as FCEUI_LoadGame, except that it can load from a tempfile. 
//name is the logical path to open; archiveFilename is the archive which contains name
FCEUGI *FCEUI_LoadGameVirtual(std::iostream* stream, int OverwriteVidMode);

//general purpose emulator initialization. returns true if successful
bool FCEUI_Initialize();

//Emulates a frame.
void FCEUI_Emulate(uint8 **, int);

void FCEUI_EmulateSound(int32 **, int32 *);

//Closes currently loaded game
void FCEUI_CloseGame(void);

//Deallocates all allocated memory.  Call after FCEUI_Emulate() returns.
void FCEUI_Kill(void);

//Set video system a=0 NTSC, a=1 PAL
void FCEUI_SetVidSystem(int a);

//Convenience function; returns currently emulated video system(0=NTSC, 1=PAL).
int FCEUI_GetCurrentVidSystem(int *slstart, int *slend);

#ifdef FRAMESKIP
void FCEUI_FrameSkip(int x);
#endif

//First and last scanlines to render, for ntsc and pal emulation.
void FCEUI_SetRenderedLines(int ntscf, int ntscl, int palf, int pall);

//Sets the base directory(save states, snapshots, etc. are saved in directories below this directory.
void FCEUI_SetBaseDirectory(std::string const & dir);

//Tells FCE Ultra to copy the palette data pointed to by pal and use it.
//Data pointed to by pal needs to be 64*3 bytes in length.
void FCEUI_SetPaletteArray(uint8 *pal);

void FCEUI_Sound(int Rate);

//at the minimum, you should call FCEUI_SetInput, FCEUI_SetInputFC, and FCEUI_SetInputFourscore
//you may also need to maintain your own internal state
void FCEUD_SetInput(bool fourscore, ESI port0, ESI port1, ESIFC fcexp);


int32 FCEUI_GetDesiredFPS(void);

int FCEUI_DecodePAR(const char *code, int *a, int *v, int *c, int *type);
int FCEUI_DecodeGG(const char *str, int *a, int *v, int *c);
int FCEUI_AddCheat(const char *name, uint32 addr, uint8 val, int compare, int type);
int FCEUI_DelCheat(uint32 which);
int FCEUI_ToggleCheat(uint32 which);

int32 FCEUI_CheatSearchGetCount(void);
void FCEUI_CheatSearchGetRange(uint32 first, uint32 last, int (*callb)(uint32 a, uint8 last, uint8 current));
void FCEUI_CheatSearchGet(int (*callb)(uint32 a, uint8 last, uint8 current, void *data), void *data);
void FCEUI_CheatSearchBegin(void);
void FCEUI_CheatSearchEnd(int type, uint8 v1, uint8 v2);
void FCEUI_ListCheats(int (*callb)(char *name, uint32 a, uint8 v, int compare, int s, int type, void *data), void *data);

int FCEUI_GetCheat(uint32 which, char **name, uint32 *a, uint8 *v, int *compare, int *s, int *type);
int FCEUI_SetCheat(uint32 which, const char *name, int32 a, int32 v, int compare, int s, int type);

void FCEUI_CheatSearchShowExcluded(void);
void FCEUI_CheatSearchSetCurrentAsOriginal(void);

//.rom
#define FCEUIOD_ROMS    0	//Roms
#define FCEUIOD_NV      1	//NV = nonvolatile. save data.	
#define FCEUIOD_STATES  2	//savestates	
#define FCEUIOD_FDSROM  3	//disksys.rom
#define FCEUIOD_SNAPS   4	//screenshots
#define FCEUIOD_CHEATS  5	//cheats
#define FCEUIOD_MOVIES  6	//.fm2 files
#define FCEUIOD_MEMW    7	//memory watch fiels
#define FCEUIOD_BBOT    8	//basicbot, obsolete
#define FCEUIOD_MACRO   9	//macro files - tasedit, currently not implemented
#define FCEUIOD_INPUT   10	//input presets
#define FCEUIOD__COUNT  11	//base directory override?

void FCEUI_SetDirOverride(int which, char *n);

void FCEUI_MemDump(uint16 a, int32 len, void (*callb)(uint16 a, uint8 v));
uint8 FCEUI_MemSafePeek(uint16 A);
void FCEUI_MemPoke(uint16 a, uint8 v, int hl);
void FCEUI_NMI(void);
void FCEUI_IRQ(void);
uint16 FCEUI_Disassemble(void *XA, uint16 a, char *stringo);
void FCEUI_GetIVectors(uint16 *reset, uint16 *irq, uint16 *nmi);

uint32 FCEUI_CRC32(uint32 crc, uint8 *buf, uint32 len);

void FCEUI_ToggleTileView(void);

void FCEUI_NSFSetVis(int mode);
int FCEUI_NSFChange(int amount);
int FCEUI_NSFGetInfo(uint8 *name, uint8 *artist, uint8 *copyright, int maxlen);

void FCEUI_VSUniToggleDIPView(void);
uint8 FCEUI_VSUniGetDIPs(void);
void FCEUI_VSUniSetDIP(int w, int state);
void FCEUI_VSUniCoin(void);

void FCEUI_FDSInsert(void); //mbg merge 7/17/06 changed to void fn(void) to make it an EMUCMDFN
//int FCEUI_FDSEject(void);
void FCEUI_FDSSelect(void);

int FCEUI_DatachSet(const uint8 *rcode);

#endif //__DRIVER_H_
