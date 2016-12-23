#include "Snes.h"

#include "core/snes9x.h"
#include "core/memmap.h"
#include "core/debug.h"
#include "core/cpuexec.h"
#include "core/ppu.h"
#include "core/snapshot.h"
#include "core/apu.h"
#include "core/display.h"
#include "core/gfx.h"
#include "core/soundux.h"
#include "core/spc700.h"
#include "core/spc7110.h"
#include "core/controls.h"
#include "core/conffile.h"
#include "core/logger.h"

#include <stdarg.h>
#include <assert.h>

using namespace snes;

static uint32 *flashDisplayBuffer = nullptr;
static short int* soundStream = nullptr;

static int bufferedSamples = 0;  // equal to bytes available

static const int SAMPLE_THRESHOLD = 2048;
static const int MAX_SAMPLES = 4096;
static const int KEY_CODE_OFFSET = 100;

static const EmulatorKey::Type KEYS_ORDER[] =
{
	EmulatorKey::LEFT,
	EmulatorKey::RIGHT,
	EmulatorKey::UP,
	EmulatorKey::DOWN,

	EmulatorKey::X,
	EmulatorKey::Y,
	EmulatorKey::A,
	EmulatorKey::B,

	EmulatorKey::L,
	EmulatorKey::R,
	EmulatorKey::START,
	EmulatorKey::SELECT
};

static const char* KEY_NAMES[] = 
{
	"Joypad1 Left",
	"Joypad1 Right",
	"Joypad1 Up",
	"Joypad1 Down",

	"Joypad1 X",
	"Joypad1 Y",
	"Joypad1 A",
	"Joypad1 B",

	"Joypad1 L",
	"Joypad1 R",
	"Joypad1 Start",
	"Joypad1 Select",

	"Joypad2 Left",
	"Joypad2 Right",
	"Joypad2 Up",
	"Joypad2 Down",

	"Joypad2 X",
	"Joypad2 Y",
	"Joypad2 A",
	"Joypad2 B",

	"Joypad2 L",
	"Joypad2 R",
	"Joypad2 Start",
	"Joypad2 Select"
};

void S9xPostRomInit();
void OutOfMemory(void);
static void Convert16To24(int width, int height);
void ResetKeymap();

#ifdef NESBOX_JS
void _makepath(char *path, const char *drive, const char *dir,
	const char *fname, const char *ext)
{

}

void _splitpath(const char *path, char *drive, char *dir, char *fname,
	char *ext)
{

}
#endif

bool8 S9xOpenSnapshotFile(const char *filepath, bool8 read_only, STREAM *file) 
{
	return false;
}

void S9xClosesnapshotFile(STREAM file) 
{
}

void S9xExit(void) 
{
}

bool8 S9xInitUpdate(void) 
{
	return true;
}

bool8 S9xDeinitUpdate(int width, int height) 
{
	if (!Settings.StopEmulation)
	{
		Convert16To24(IMAGE_WIDTH, IMAGE_HEIGHT);
	}
	return true;
}

void S9xMessage(int type, int number, const char *message) {}

const char *S9xGetFilename(const char *extension, enum s9x_getdirtype dirtype) 
{
	static char filename[MAX_PATH + 1];
	//char dir[_MAX_DIR + 1];
	//char drive[_MAX_DRIVE + 1];
	//char fname[_MAX_FNAME + 1];
	//char ext[_MAX_EXT + 1];
	//_splitpath(Memory.ROMFilename, drive, dir, fname, ext);
	//_snprintf(filename, sizeof(filename), "%s" SLASH_STR "%s%s",
	//	S9xGetDirectory(dirtype), fname, extension);

	return (filename);
}

const char *S9xGetFilenameInc(const char *extension, enum s9x_getdirtype dirtype) 
{
	return "";
}

const char *S9xGetDirectory(enum s9x_getdirtype dirtype) 
{
	return ".";
}

START_EXTERN_C
char* osd_GetPackDir() 
{
	return "";
}
END_EXTERN_C

const char *S9xChooseFilename(bool8 read_only) 
{
	return "";
}

const char *S9xChooseMovieFilename(bool8 read_only) 
{
	return "";
}

const char *S9xBasename(const char *path) 
{
	return "";
}

void S9xAutoSaveSRAM(void) 
{
}


bool8 S9xOpenSoundDevice(int mode, bool8 stereo, int buffer_size)
{
	fprintf(stderr, "OPEN SOUND DEVICE!!");

	// Allocate sound buffer
	if (soundStream)
	{
		free(soundStream);
		soundStream = nullptr;
	}

	soundStream = (short int *)malloc(sizeof(short int)* MAX_SAMPLES << 1);  // Total bytes
	memset(soundStream, 0, sizeof(short int)* MAX_SAMPLES << 1);

	so.stereo = Settings.Stereo;
	so.encoded = false;
	so.playback_rate = Settings.SoundPlaybackRate;
	so.sixteen_bit = Settings.SixteenBitSound;
	so.buffer_size = sizeof(short int)* MAX_SAMPLES << 1;

	S9xSetPlaybackRate(so.playback_rate);

	so.mute_sound = false;

	// Debug
	fprintf(stderr, "Rate: %d, Buffer size: %d, 16-bit: %s, Stereo: %s, Encoded: %s\n",
		so.playback_rate, so.buffer_size, so.sixteen_bit ? "yes" : "no",
		so.stereo ? "yes" : "no", so.encoded ? "yes" : "no");

	return true;
}

void S9xGenerateSound(void)
{
}

void S9xToggleSoundChannel(int c)
{
}

void S9xSetPalette(void) 
{
}

void S9xSyncSpeed(void) {

	IPPU.RenderThisFrame = true;
	IPPU.FrameSkip = 0;
	IPPU.SkippedFrames = 0;

}

void S9xLoadSDD1Data(void) 
{
}

void OutOfMemory(void)
{
	throw std::bad_alloc();
}

static void Convert16To24(int width, int height)
{
	for (register int y = 0; y < height; y++) {	// For each row
		register uint32 *d = (uint32 *)(flashDisplayBuffer + y * (GFX.Pitch >> 1));		// destination ptr
		register uint16 *s = (uint16 *)(GFX.Screen + y * (GFX.Pitch >> 1));				// source ptr
		for (register int x = 0; x < width; x++) { // For each column
			uint16 pixel = *s++;
			*d++ = (((pixel >> 11) & 0x1F) << (16 + 3)) |
				(((pixel >> 5) & 0x3F) << (8 + 2)) |
				((pixel & 0x1f) << (3));
		}
	}
}

void S9xTracef(const char *format, ...)
{
}

void S9xTraceInt(int val)
{
}

void S9xPostRomInit()
{
	if (!strncmp((const char *)Memory.NSRTHeader + 24, "NSRT", 4))
	{
		//First plug in both, they'll change later as needed
		S9xSetController(0, CTL_JOYPAD, 0, 0, 0, 0);
		S9xSetController(1, CTL_JOYPAD, 1, 0, 0, 0);

		switch (Memory.NSRTHeader[29])
		{
		case 0: //Everything goes
			break;

		case 0x10: //Mouse in Port 0
			S9xSetController(0, CTL_MOUSE, 0, 0, 0, 0);
			break;

		case 0x01: //Mouse in Port 1
			S9xSetController(1, CTL_MOUSE, 1, 0, 0, 0);
			break;

		case 0x03: //Super Scope in Port 1
			S9xSetController(1, CTL_SUPERSCOPE, 0, 0, 0, 0);
			break;

		case 0x06: //Multitap in Port 1
			S9xSetController(1, CTL_MP5, 1, 2, 3, 4);
			break;

		case 0x66: //Multitap in Ports 0 and 1
			S9xSetController(0, CTL_MP5, 0, 1, 2, 3);
			S9xSetController(1, CTL_MP5, 4, 5, 6, 7);
			break;

		case 0x08: //Multitap in Port 1, Mouse in new Port 1
			S9xSetController(1, CTL_MOUSE, 1, 0, 0, 0);
			//There should be a toggle here for putting in Multitap instead
			break;

		case 0x04: //Pad or Super Scope in Port 1
			S9xSetController(1, CTL_SUPERSCOPE, 0, 0, 0, 0);
			//There should be a toggle here for putting in a pad instead
			break;

		case 0x05: //Justifier - Must ask user...
			S9xSetController(1, CTL_JUSTIFIER, 1, 0, 0, 0);
			//There should be a toggle here for how many justifiers
			break;

		case 0x20: //Pad or Mouse in Port 0
			S9xSetController(0, CTL_MOUSE, 0, 0, 0, 0);
			//There should be a toggle here for putting in a pad instead
			break;

		case 0x22: //Pad or Mouse in Port 0 & 1
			S9xSetController(0, CTL_MOUSE, 0, 0, 0, 0);
			S9xSetController(1, CTL_MOUSE, 1, 0, 0, 0);
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

void S9xCloseSnapshotFile(STREAM file)
{
}

bool8 S9xContinueUpdate(int Width, int Height)
{
	return true;
}

void S9xParseArg(char **argv, int &i, int argc)
{
}

void S9xExtraUsage()
{
}

void S9xParsePortConfig(ConfigFile &conf, int pass)
{
}

const char * S9xStringInput(const char *msg)
{
	return NULL;
}

void S9xHandlePortCommand(s9xcommand_t cmd, int16 data1, int16 data2)
{
}

bool S9xPollButton(uint32 id, bool *pressed)
{
	return false;
}

bool S9xPollAxis(uint32 id, int16 *value)
{
	return false;
}


bool S9xPollPointer(uint32 id, int16 *x, int16 *y)
{
	return true;
}

Snes::Snes(const RomInfo& info)
: m_romInfo(info)
{
	m_romInfo.width = SNES_WIDTH;
	m_romInfo.height = SNES_HEIGHT;
}

bool Snes::LoadRom(const RomBuffer& romBuffer)
{
	try
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
		Settings.SoundPlaybackRate = m_romInfo.samplerate;
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
			OutOfMemory();

		Memory.PostRomInitFunc = S9xPostRomInit;

		// Further sound initialization
		S9xInitSound(7, Settings.Stereo, Settings.SoundBufferSize); // The 7 is our "mode" and isn't really explained anywhere. 7 ensures that OpenSoundDevice is called.

		//S9xInitInputDevices(); // Not necessary! In the unix port it's used to setup an optional joystick

		// Initialize GFX members
		GFX.Pitch = IMAGE_WIDTH * 2;

		GFX.Screen = (uint16 *)malloc(GFX.Pitch * IMAGE_HEIGHT);
		GFX.SubScreen = (uint16 *)malloc(GFX.Pitch * IMAGE_HEIGHT);
		GFX.ZBuffer = (uint8 *)malloc((GFX.Pitch >> 1) * IMAGE_HEIGHT);
		GFX.SubZBuffer = (uint8 *)malloc((GFX.Pitch >> 1) * IMAGE_HEIGHT);

		if (!GFX.Screen || !GFX.SubScreen)
			OutOfMemory();

		// Initialize 32-bit version of GFX.Screen
		if (flashDisplayBuffer)
		{
			free(flashDisplayBuffer);
			flashDisplayBuffer = nullptr;
		}

		flashDisplayBuffer = (uint32 *)malloc((GFX.Pitch << 1) * IMAGE_HEIGHT);

		ZeroMemory(flashDisplayBuffer, (GFX.Pitch << 1) * IMAGE_HEIGHT);
		ZeroMemory(GFX.Screen, GFX.Pitch * IMAGE_HEIGHT);
		ZeroMemory(GFX.SubScreen, GFX.Pitch * IMAGE_HEIGHT);
		ZeroMemory(GFX.ZBuffer, (GFX.Pitch >> 1) * IMAGE_HEIGHT);
		ZeroMemory(GFX.SubZBuffer, (GFX.Pitch >> 1) * IMAGE_HEIGHT);

		if (!S9xGraphicsInit())
			OutOfMemory();

		uint32 saved_flags = CPU.Flags;

		m_rom = romBuffer;
		uint32 loaded = Memory.LoadROM(&m_rom.at(0), m_rom.size());

		Settings.StopEmulation = false;

		if (loaded == 0)
		{
			return false;
		}

		CPU.Flags = saved_flags;

		ResetKeymap();
		return true;
	}
	catch (...)
	{
	}
	return false;
}

static int processGamepad(int offset, const EmulatorKeyBitset& keys, EmulatorKeyBitset& prev)
{
	for (auto key : KEYS_ORDER)
	{
		if (keys.IsKeyDown(key) && !prev.IsKeyDown(key))
			S9xReportButton(offset, true);

		else if (!keys.IsKeyDown(key) && prev.IsKeyDown(key))
			S9xReportButton(offset, false);

		offset++;
	}

	prev = keys;

	return offset;
}

ScreenBuffer Snes::Tick(const EmulatorKeyBitset& player1, const EmulatorKeyBitset& player2)
{
	int offset = processGamepad(KEY_CODE_OFFSET, player1, m_prevKeys1);
	processGamepad(offset, player2, m_prevKeys2);

	S9xMainLoop();

	int numSamples = m_romInfo.samplerate / 60;

	if (bufferedSamples + numSamples <= MAX_SAMPLES)
	{
		Flash_S9xMixSamples(soundStream, numSamples << 1, bufferedSamples << 1);
		bufferedSamples += numSamples;
	}

	return (uint8*)flashDisplayBuffer;
}

ScreenBuffer Snes::Tick(const EmulatorKeyBitset& keys)
{
	static const EmulatorKeyBitset temp;
	return Tick(keys, temp);
}


SoundBuffer Snes::GetSoundBuffer()
{
	SoundBuffer temp(soundStream, soundStream + (bufferedSamples << 1));
	bufferedSamples = 0;
	return temp;
}

const RomInfo& Snes::GetRomInfo() const
{
	return m_romInfo;
}

void Snes::ResetKeymap()
{
	S9xUnmapAllControls();

	int offset = KEY_CODE_OFFSET;

	for (auto name : KEY_NAMES)
		S9xMapButton(offset++, S9xGetCommandT(name), false);

	S9xSetController(0, CTL_JOYPAD, 0, 0, 0, 0);
	S9xSetController(1, CTL_JOYPAD, 1, 0, 0, 0);
}

StateBuffer Snes::SaveState() const
{
	static const int STATE_BUFFER_SIZE = 1024 * 1024 * 4;

	StateBuffer state(STATE_BUFFER_SIZE);

	BUFFER_FILE stream;

	stream.buffer = (char*)&state[0];
	stream.position = 0;
	stream.file = NULL;

	S9xPrepareSoundForSnapshotSave(FALSE);
	S9xFreezeToStream(&stream);
	S9xPrepareSoundForSnapshotSave(TRUE);
	S9xResetSaveTimer(TRUE);

	assert(stream.position < STATE_BUFFER_SIZE);

	state.resize(stream.position);

	return state;
}

void Snes::LoadState(const StateBuffer& state)
{
	BUFFER_FILE snapshot;

	snapshot.buffer = (char*)&state[0];
	snapshot.position = 0;
	snapshot.file = NULL;

	S9xUnfreezeFromStream(&snapshot);
}
