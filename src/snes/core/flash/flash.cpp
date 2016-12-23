#include "AS3/AS3.h"

#include "snes9x.h"
#include "memmap.h"
#include "debug.h"
#include "cpuexec.h"
#include "ppu.h"
#include "snapshot.h"
#include "apu.h"
#include "display.h"
#include "gfx.h"
#include "soundux.h"
#include "spc700.h"
#include "spc7110.h"
#include "controls.h"
#include "conffile.h"
#include "logger.h"

#include "flash.h"

/*
 * Flash buffers - screen and sound
 */
uint32 *FlashDisplayBuffer = NULL;
float* soundStream = NULL;

int bufferedSamples = 0;  // equal to bytes available

int SAMPLE_THRESHOLD = 2048;
int MAX_SAMPLES = 4096;


int main ()
{

    AS3_GoAsync();

    return (0);
}

__attribute__((annotate("as3sig:public function wrapGetSoundBuffer():int")))
void wrapGetSoundBuffer()
{
    {
        AS3_DeclareVar(asresult, int);
        AS3_CopyScalarToVar(asresult, soundStream);   
    }

    {
        AS3_ReturnAS3Var(asresult);
    }
}

__attribute__((annotate("as3sig:public function wrapBeginPaintSound():int")))
void wrapBeginPaintSound()
{
    int result = beginPaintSound();

    {
        AS3_DeclareVar(asresult, int);
        AS3_CopyScalarToVar(asresult, result);
    }

    {
        AS3_ReturnAS3Var(asresult);
    }
}

__attribute__((annotate("as3sig:public function wrapEndPaintSound():void")))
void wrapEndPaintSound()
{
    endPaintSound();

    {
        AS3_ReturnAS3Var(undefined);
    }
}

__attribute__((annotate("as3sig:public function wrapTick(firstPad:uint, secondPad:uint):int")))
void wrapTick()
{
    unsigned int arg1;
    unsigned int arg2;
    uint8* result;

    {
        AS3_GetScalarFromVar(arg1, firstPad);
    }

    {
        AS3_GetScalarFromVar(arg2, secondPad);
    }

    result = tick(arg1, arg2);

    {
        AS3_DeclareVar(asresult, int);
        AS3_CopyScalarToVar(asresult, result);
    }

    {
        AS3_ReturnAS3Var(asresult);
    }
}

__attribute__((annotate("as3sig:public function wrapSaveState(buffer:int):int")))
void wrapSaveState()
{
    unsigned char *arg1 = (unsigned char *) 0 ;
    int result;

    {
        AS3_GetScalarFromVar(arg1, buffer);
    }

    result = saveState(arg1);

    {
        AS3_DeclareVar(asresult, int);
        AS3_CopyScalarToVar(asresult, result);
    }

    {
        AS3_ReturnAS3Var(asresult);
    }
}

__attribute__((annotate("as3sig:public function wrapLoadState(buffer:int):void")))
void wrapLoadState()
{
    unsigned char *arg1 = (unsigned char *) 0 ;

    {
        AS3_GetScalarFromVar(arg1, buffer);
    }

    loadState(arg1);

    {
        AS3_ReturnAS3Var(undefined);
    }
}

__attribute__((annotate("as3sig:public function wrapLoadRom(rom:int, length:int, pal:int, region:int):int")))
void wrapLoadRom()
{
    uint8 *arg1 = (unsigned char *)0;
    int arg2;
    int arg3;
    char* arg4;
    int result;

    {
        AS3_GetScalarFromVar(arg1, rom);
    }

    {
        AS3_GetScalarFromVar(arg2, length);
    }

    {
        AS3_GetScalarFromVar(arg3, pal);
    }

    {
        AS3_GetScalarFromVar(arg4, region);
    }

    result = loadRom(arg1, arg2);

    {
        AS3_DeclareVar(asresult, int);
        AS3_CopyScalarToVar(asresult, result);  
    }

    {
        AS3_ReturnAS3Var(asresult);
    }
}

uint8* tick(unsigned int inputFlagsFirst, unsigned int inputFlagsSecond)
{
    S9xProcessEvents( inputFlagsFirst, inputFlagsSecond );
    S9xMainLoop();

    int numSamples = 44100/60;

    if ( bufferedSamples + numSamples <= MAX_SAMPLES )
    {
        Flash_S9xMixSamples( soundStream, numSamples << 1, bufferedSamples << 1 );
        bufferedSamples += numSamples;
    }

    return (uint8*)FlashDisplayBuffer;
}

int loadRom(unsigned char* romBuffer, int bytesAvailable)
{
    ZeroMemory(&Settings, sizeof (Settings));

    // General
    Settings.ShutdownMaster = true; // Optimization -- Disable if this appears to cause any compatability issues -- it's known to for some games
    Settings.BlockInvalidVRAMAccess = true;
    Settings.HDMATimingHack = 100;
    Settings.Transparency = true;
    Settings.SupportHiRes = false;
    Settings.SDD1Pack = true;

    // Sound
    Settings.SoundPlaybackRate = 44100;
    Settings.Stereo = true;
    Settings.SixteenBitSound = true;
    Settings.DisableSoundEcho = true;
    Settings.SoundEnvelopeHeightReading = true;
    Settings.DisableSampleCaching = true;
    Settings.InterpolatedSound = true;

    // Controllers
    Settings.JoystickEnabled = false;
    Settings.MouseMaster = false;
    Settings.SuperScopeMaster = false;
    Settings.MultiPlayer5Master = false;
    Settings.JustifierMaster = false;

    // old settings
    Settings.APUEnabled = Settings.NextAPUEnabled = true;

    Settings.Multi = false;
    Settings.StopEmulation = true;

    // So listen, snes9x, we don't have any controllers. That's OK, yeah?
    S9xReportControllers();

    // Initialize system memory
    if (!Memory.Init() || !S9xInitAPU()) // We'll add sound init here later!
        OutOfMemory ();

    Memory.PostRomInitFunc = S9xPostRomInit;

    // Further sound initialization
    S9xInitSound(7, Settings.Stereo, Settings.SoundBufferSize); // The 7 is our "mode" and isn't really explained anywhere. 7 ensures that OpenSoundDevice is called.

    //S9xInitInputDevices(); // Not necessary! In the unix port it's used to setup an optional joystick

    // Initialize GFX members
    GFX.Pitch = IMAGE_WIDTH * 2;
    GFX.Screen = (uint16 *) malloc (GFX.Pitch * IMAGE_HEIGHT);
    GFX.SubScreen = (uint16 *) malloc (GFX.Pitch * IMAGE_HEIGHT);
    GFX.ZBuffer = (uint8 *) malloc ((GFX.Pitch >> 1) * IMAGE_HEIGHT);
    GFX.SubZBuffer = (uint8 *) malloc ((GFX.Pitch >> 1) * IMAGE_HEIGHT);

    if (!GFX.Screen || !GFX.SubScreen)
        OutOfMemory();

    // Initialize 32-bit version of GFX.Screen
    FlashDisplayBuffer = (uint32 *) malloc ((GFX.Pitch << 1) * IMAGE_HEIGHT);

    ZeroMemory (FlashDisplayBuffer, (GFX.Pitch << 1) * IMAGE_HEIGHT);
    ZeroMemory (GFX.Screen, GFX.Pitch * IMAGE_HEIGHT);
    ZeroMemory (GFX.SubScreen, GFX.Pitch * IMAGE_HEIGHT);
    ZeroMemory (GFX.ZBuffer, (GFX.Pitch>>1) * IMAGE_HEIGHT);
    ZeroMemory (GFX.SubZBuffer, (GFX.Pitch>>1) * IMAGE_HEIGHT);

    if (!S9xGraphicsInit())
        OutOfMemory();

    uint32 saved_flags = CPU.Flags;

    uint32 loaded = Memory.LoadROM(romBuffer, bytesAvailable);

    Settings.StopEmulation = false;

    if (loaded == 0)
    {
        return 0;
    }
    
    CPU.Flags = saved_flags;

    S9xSetupDefaultKeymap();

    return loaded; 
}

void S9xProcessEvents(unsigned int first, unsigned int second)
{
    static const uint32 OriginalCodes[] = {87, 83, 65, 68, 75, 76, 79, 80, 16, 13, 88, 77, 84, 71, 70, 72, 66, 78, 67, 86, 69, 81, 89, 85};
    static uint32 previousInputFlags = 0;

    uint32 inputFlags = (first & 0x0fff) | ((second & 0x0fff) << 12);

    for(int index = 0; index < 24; index++)
    {
        uint32 mask = 1 << index;

        uint32 current = inputFlags & mask;
        uint32 last = previousInputFlags & mask;

        if(current != last)
        {
            S9xReportButton( OriginalCodes[index], current ? 1 : 0 );
        }
    }

    previousInputFlags = inputFlags;
}

void S9xSetupDefaultKeymap()
{	
    S9xUnmapAllControls();

    // Build key map
    s9xcommand_t cmd;

    // Player 1

    S9xMapButton( 65, cmd = S9xGetCommandT("Joypad1 Left"), false );    // A
    S9xMapButton( 68, cmd = S9xGetCommandT("Joypad1 Right"), false );   // D
    S9xMapButton( 87, cmd = S9xGetCommandT("Joypad1 Up"), false );      // W
    S9xMapButton( 83, cmd = S9xGetCommandT("Joypad1 Down"), false );    // S

    S9xMapButton( 79, cmd = S9xGetCommandT("Joypad1 X"), false );       // O
    S9xMapButton( 80, cmd = S9xGetCommandT("Joypad1 Y"), false );       // P
    S9xMapButton( 75, cmd = S9xGetCommandT("Joypad1 A"), false );       // K
    S9xMapButton( 76, cmd = S9xGetCommandT("Joypad1 B"), false );       // L

    S9xMapButton( 88, cmd = S9xGetCommandT("Joypad1 L"), false );       // X
    S9xMapButton( 77, cmd = S9xGetCommandT("Joypad1 R"), false );       // M

    S9xMapButton( 13, cmd = S9xGetCommandT("Joypad1 Start"), false );   // Enter
    S9xMapButton( 16, cmd = S9xGetCommandT("Joypad1 Select"), false );  // Shift

    // Player 2

    S9xMapButton( 70, cmd = S9xGetCommandT("Joypad2 Left"), false );    // F
    S9xMapButton( 72, cmd = S9xGetCommandT("Joypad2 Right"), false );   // H
    S9xMapButton( 84, cmd = S9xGetCommandT("Joypad2 Up"), false );      // T
    S9xMapButton( 71, cmd = S9xGetCommandT("Joypad2 Down"), false );    // G

    S9xMapButton( 67, cmd = S9xGetCommandT("Joypad2 X"), false );       // C
    S9xMapButton( 86, cmd = S9xGetCommandT("Joypad2 Y"), false );       // V
    S9xMapButton( 66, cmd = S9xGetCommandT("Joypad2 A"), false );       // B
    S9xMapButton( 78, cmd = S9xGetCommandT("Joypad2 B"), false );       // N

    S9xMapButton( 89, cmd = S9xGetCommandT("Joypad2 L"), false );       // Y
    S9xMapButton( 85, cmd = S9xGetCommandT("Joypad2 R"), false );       // U

    S9xMapButton( 81, cmd = S9xGetCommandT("Joypad2 Start"), false );   // Q
    S9xMapButton( 69, cmd = S9xGetCommandT("Joypad2 Select"), false );  // E

    S9xSetController(0, CTL_JOYPAD,     0, 0, 0, 0);
    S9xSetController(1, CTL_JOYPAD,     1, 0, 0, 0);
}



/*
 * Required interface methods specified in Porting.html
 */
bool8 S9xOpenSnapshotFile (const char *filepath, bool8 read_only, STREAM *file) {
   AS3_Trace(("S9xOpenSnapshotFile"));
    return false;
}

void S9xClosesnapshotFile (STREAM file) {
    AS3_Trace(("S9xClosesnapshotFile"));
}

void S9xExit (void) {
    AS3_Trace(("S9xExit"));
}

bool8 S9xInitUpdate (void) { // Screen is *about* to be rendered
    	// AS3_Trace(("S9xInitUpdate"));
    return true;
}

bool8 S9xDeinitUpdate (int width, int height) { // Screen has been rendered
    	// AS3_Trace(("S9xDeinitUpdate"));
    if (!Settings.StopEmulation){
        Convert16To24(IMAGE_WIDTH, IMAGE_HEIGHT);
    }
    return true;
}

void S9xMessage (int type, int number, const char *message) {
    //AS3_Trace((">>>>>>>>>>>>>>>>>>>> EMULATOR MESSAGE >>>>>>>>>>>>>>>>>>>>"));
    //AS3_ReadString(asmessage, message, strlen(message));
    //AS3_Trace(asmessage);
}

const char *S9xGetFilename (const char *extension, enum s9x_getdirtype dirtype) {
    AS3_Trace(("*S9xGetFilename"));

    static char filename [PATH_MAX + 1];
    char dir [_MAX_DIR + 1];
    char drive [_MAX_DRIVE + 1];
    char fname [_MAX_FNAME + 1];
    char ext [_MAX_EXT + 1];
    _splitpath (Memory.ROMFilename, drive, dir, fname, ext);
    snprintf(filename, sizeof(filename), "%s" SLASH_STR "%s%s",
            S9xGetDirectory(dirtype), fname, extension);

    // AS3_Trace((filename));

    return (filename);
}

const char *S9xGetFilenameInc (const char *extension, enum s9x_getdirtype dirtype) {
    AS3_Trace(("*S9xGetFilenameInc"));
    return "";
}

const char *S9xGetDirectory (enum s9x_getdirtype dirtype) {
    AS3_Trace(("*S9xGetDirectory"));
    return ".";
}

START_EXTERN_C
char* osd_GetPackDir() {
    AS3_Trace(("osd_GetPackDir"));
    return "";
}
END_EXTERN_C

const char *S9xChooseFilename (bool8 read_only) {
    AS3_Trace(("S9xChooseFilename"));
    return "";
}

const char *S9xChooseMovieFilename (bool8 read_only) {
    AS3_Trace(("S9xChooseMovieFilename"));
    return "";
}

const char *S9xBasename (const char *path) {
    AS3_Trace(("S9xBasename"));
    return "";
}

void S9xAutoSaveSRAM (void) {
    AS3_Trace(("S9xAutoSaveSRAM"));
}


bool8 S9xOpenSoundDevice (int mode, bool8 stereo, int buffer_size)
{
    fprintf( stderr, "OPEN SOUND DEVICE!!" );

    // Allocate sound buffer
    soundStream = (float *) malloc(sizeof(float) * MAX_SAMPLES << 1);  // Total bytes
    memset(soundStream, 0, sizeof(float) * MAX_SAMPLES << 1);

    so.stereo         = Settings.Stereo;
    so.encoded        = false;
    so.playback_rate  = Settings.SoundPlaybackRate;
    so.sixteen_bit    = Settings.SixteenBitSound;
    so.buffer_size    = sizeof(float) * MAX_SAMPLES << 1;

    S9xSetPlaybackRate(so.playback_rate);

    so.mute_sound = false;

    // Debug
    fprintf( stderr, "Rate: %d, Buffer size: %d, 16-bit: %s, Stereo: %s, Encoded: %s\n",
            so.playback_rate, so.buffer_size, so.sixteen_bit ? "yes" : "no",
            so.stereo ? "yes" : "no", so.encoded ? "yes" : "no");

    return true;
}

int beginPaintSound ()
{
    int minSamples = SAMPLE_THRESHOLD;  // 2048

    // Flush completely
    if ( bufferedSamples < minSamples )
    {
        // Get up to the threshold
        Flash_S9xMixSamples( soundStream, (minSamples-bufferedSamples) << 1, bufferedSamples << 1 );
        bufferedSamples = minSamples;
    }
    else if (bufferedSamples > MAX_SAMPLES)
    {
        bufferedSamples = MAX_SAMPLES;
    }

    float* soundBuffer = soundStream;
    float* soundBufferEnd = soundStream + (bufferedSamples << 1);
    while(soundBuffer != soundBufferEnd)
    {
        if(*soundBuffer > 1.0 
            || *soundBuffer < -1.0 
            || *soundBuffer == -0.37254899740219116
            || *soundBuffer == 0.0007104013347998261)
            *soundBuffer = 0.0;

        soundBuffer++;
    }

    return bufferedSamples;
}

void endPaintSound ()
{
    memset(soundStream, 0, bufferedSamples * sizeof(float) << 1);
    bufferedSamples = 0;
}

void S9xGenerateSound (void)
{
    AS3_Trace("S9xGenerateSound");
}

void S9xToggleSoundChannel (int c)
{
    AS3_Trace("S9xToggleSoundChannel");
}

int saveState(unsigned char* buffer)
{
    BUFFER_FILE stream;

    stream.buffer = (char*)buffer;
    stream.position = 0;
    stream.file = NULL;

    S9xPrepareSoundForSnapshotSave (FALSE);
    S9xFreezeToStream (&stream);
    S9xPrepareSoundForSnapshotSave (TRUE);
    S9xResetSaveTimer (TRUE);

    return stream.position;
}

void loadState(unsigned char* buffer)
{
    BUFFER_FILE snapshot;

    snapshot.buffer = (char*)buffer;//(char*)malloc(bytesAvailable);
    snapshot.position = 0;
    snapshot.file = NULL;

    S9xUnfreezeFromStream (&snapshot);
}

void S9xSetPalette (void) {
    // AS3_Trace("S9xSetPalette");
}

void S9xSyncSpeed (void) {

    IPPU.RenderThisFrame = true;
    IPPU.FrameSkip = 0;
    IPPU.SkippedFrames = 0;

}

void S9xLoadSDD1Data (void) {
    AS3_Trace("S9xLoadSDD1Data");
}


// These methods are part of the driver interface although aren't mentioned in porting.html
void _makepath (char *path, const char *, const char *dir,
        const char *fname, const char *ext)
{
    if (dir && *dir)
    {
        strcpy (path, dir);
        strcat (path, "/");
    }
    else
        *path = 0;
    strcat (path, fname);
    if (ext && *ext)
    {
        strcat (path, ".");
        strcat (path, ext);
    }
}

void _splitpath(const char *path, char *drive, char *dir, char *fname, char *ext)
{
    *drive = 0;

    char *slash = strrchr(path, SLASH_CHAR);
    char *dot = strrchr(path, '.');

    if (dot && slash && dot < slash) {
        dot = 0;
    }

    if (!slash) {
        *dir = 0;
        strcpy(fname, path);
        if (dot) {
            fname[dot - path] = 0;
            strcpy(ext, dot + 1);
        } else {
            *ext = 0;
        }
    } else {
        strcpy(dir, path);
        dir[slash - path] = 0;
        strcpy(fname, slash + 1);
        if (dot) {
            fname[(dot - slash) - 1] = 0;
            strcpy(ext, dot + 1);
        } else {
            *ext = 0;
        }
    }
}


/*
 * Other methods
 */
void OutOfMemory (void)
{
    AS3_Trace("out of memory");
}

// Color Depth Upcast: RGB565 -> RGB888
void Convert16To24 (int width, int height){

    // Profiler setup
    //	AS3_Val profilerParams = AS3_Array("StrType", "colorUpcast");
    //	AS3_CallS( "begin", FLASH_STATIC_PROFILER, profilerParams );

    for (register int y = 0; y < height; y++) {	// For each row
        register uint32 *d = (uint32 *) (FlashDisplayBuffer + y * (GFX.Pitch>>1));		// destination ptr
        register uint16 *s = (uint16 *) (GFX.Screen + y * (GFX.Pitch>>1));				// source ptr
        for (register int x = 0; x < width; x++) { // For each column
            uint16 pixel = *s++;
            *d++ = (((pixel >> 11) & 0x1F) << (16 + 3)) |
                (((pixel >> 5) & 0x3F) << (8 + 2)) |
                ((pixel & 0x1f) << (3));
        }
    }

    // Profiler teardown
    //	AS3_CallS( "end", FLASH_STATIC_PROFILER, profilerParams );
    //	AS3_CallS( "printReport", FLASH_STATIC_PROFILER, FLASH_EMPTY_PARAMS );
    //	AS3_Release( profilerParams );
}

// All printf statements are re-routed here

#include <stdarg.h>
void S9xTracef( const char *format, ... ){
    // AS3_Trace((format));
}


void S9xTraceInt( int val ){
    // AS3_Trace(AS3_Int((int)val));
}



void S9xPostRomInit(){
    AS3_Trace("S9xPostRomInit");

    if (!strncmp((const char *)Memory.NSRTHeader+24, "NSRT", 4))
    {
        //First plug in both, they'll change later as needed
        S9xSetController(0, CTL_JOYPAD,     0, 0, 0, 0);
        S9xSetController(1, CTL_JOYPAD,     1, 0, 0, 0);

        switch (Memory.NSRTHeader[29])
        {
            case 0: //Everything goes
                break;

            case 0x10: //Mouse in Port 0
                S9xSetController(0, CTL_MOUSE,      0, 0, 0, 0);
                break;

            case 0x01: //Mouse in Port 1
                S9xSetController(1, CTL_MOUSE,      1, 0, 0, 0);
                break;

            case 0x03: //Super Scope in Port 1
                S9xSetController(1, CTL_SUPERSCOPE, 0, 0, 0, 0);
                break;

            case 0x06: //Multitap in Port 1
                S9xSetController(1, CTL_MP5,        1, 2, 3, 4);
                break;

            case 0x66: //Multitap in Ports 0 and 1
                S9xSetController(0, CTL_MP5,        0, 1, 2, 3);
                S9xSetController(1, CTL_MP5,        4, 5, 6, 7);
                break;

            case 0x08: //Multitap in Port 1, Mouse in new Port 1
                S9xSetController(1, CTL_MOUSE,      1, 0, 0, 0);
                //There should be a toggle here for putting in Multitap instead
                break;

            case 0x04: //Pad or Super Scope in Port 1
                S9xSetController(1, CTL_SUPERSCOPE, 0, 0, 0, 0);
                //There should be a toggle here for putting in a pad instead
                break;

            case 0x05: //Justifier - Must ask user...
                S9xSetController(1, CTL_JUSTIFIER,  1, 0, 0, 0);
                //There should be a toggle here for how many justifiers
                break;

            case 0x20: //Pad or Mouse in Port 0
                S9xSetController(0, CTL_MOUSE,      0, 0, 0, 0);
                //There should be a toggle here for putting in a pad instead
                break;

            case 0x22: //Pad or Mouse in Port 0 & 1
                S9xSetController(0, CTL_MOUSE,      0, 0, 0, 0);
                S9xSetController(1, CTL_MOUSE,      1, 0, 0, 0);
                //There should be a toggles here for putting in pads instead
                break;

            case 0x24: //Pad or Mouse in Port 0, Pad or Super Scope in Port 1
                //There should be a toggles here for what to put in, I'm leaving it at gamepad for now
                break;

            case 0x27: //Pad or Mouse in Port 0, Pad or Mouse or Super Scope in Port 1
                //There should be a toggles here for what to put in, I'm leaving it at gamepad for now
                break;

                //Not Supported yet
            case 0x99: break; //Lasabirdie
            case 0x0A: break; //Barcode Battler
        }
    }
}

void S9xCloseSnapshotFile( STREAM file)
{
    AS3_Trace("S9xCloseSnapshotFile");
}

bool8 S9xContinueUpdate(int Width, int Height)
{
    AS3_Trace("S9xContinueUpdate");
    return true;
}

void S9xParseArg (char **argv, int &i, int argc)
{
    AS3_Trace("S9xParseArg");
}

void S9xExtraUsage ()
{
    AS3_Trace("S9xExtraUsage");
}

void S9xParsePortConfig(ConfigFile &conf, int pass)
{
    AS3_Trace("S9xParsePortConfig");
}

const char * S9xStringInput(const char *msg)
{
    AS3_Trace("S9xStringInput");
    return NULL;
}

void S9xHandlePortCommand(s9xcommand_t cmd, int16 data1, int16 data2)
{
    AS3_Trace("S9xHandlePortCommand");
}

bool S9xPollButton(uint32 id, bool *pressed)
{
    AS3_Trace("S9xPollButton");
    return false;
}

bool S9xPollAxis(uint32 id, int16 *value)
{
    AS3_Trace("S9xPollAxis");
    return false;
}


bool S9xPollPointer(uint32 id, int16 *x, int16 *y)
{
    AS3_Trace("S9xPollPointer");
    return true;
}
