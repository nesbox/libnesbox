#include "gameboy.h"

#include "core/gba/GBA.h"
#include "core/gba/RTC.h"
#include "core/gba/Sound.h"
#include "core/Util.h"
#include "core/common/SoundDriver.h"

#include "core/gb/gb.h"
#include "core/gb/gbGlobals.h"
#include "core/gb/gbCheats.h"
#include "core/gb/gbSound.h"

#include <memory>
#include <assert.h>
#include <string.h>
#include <algorithm>
#include <iostream>
#include <iterator>

//#include <sstream>
//
//void trace(const std::string& string, int value)
//{
//	std::stringstream stream;
//
//	stream << string;
//	stream << value;
//	stream << std::endl;
//
//	OutputDebugStringA(stream.str().c_str());
//}

using namespace gb;

static const int GBA_EMULATOR_WIDTH = 241;
static const int GBA_EMULATOR_HEIGHT = 160;

static const int GBX_EMULATOR_WIDTH = 161;
static const int GBX_EMULATOR_HEIGHT = 144;

static const int MAX_SAMPLES = 4096;

static const int MIN_STATE_BUFFER_SIZE = 1024 * 1024 * 4;

static int inputFlags = 0;
static std::vector<u16> soundWave;

void saveState(unsigned char* buffer) {}
void loadState(unsigned char* buffer) {}

bool rtcWrite(unsigned int, unsigned short){ return false; }
unsigned short rtcRead(unsigned int){ return 0; }
void rtcEnable(bool){}
void rtcReset(){}
void rtcSaveGame(gzFile gzFile){}
void rtcReadGame(gzFile gzFile){}

void log(const char *, ...)
{}

bool systemPauseOnFrame()
{
	return false;
}

void systemGbPrint(u8 *, int, int, int, int, int)
{}

void systemScreenCapture(int)
{}

static int drawScreenCalled = 0;

void systemDrawScreen()
{
	drawScreenCalled++;
}
// updates the joystick data
bool systemReadJoypads()
{
	return true;
}

// return information about the given joystick, -1 for default joystick
u32 systemReadJoypad(int)
{
	if ((inputFlags & 48) == 48)
		inputFlags &= ~16;
	if ((inputFlags & 192) == 192)
		inputFlags &= ~128;

	return inputFlags;
}

void systemMessage(int, const char *, ...)
{

}

void systemSetTitle(const char *) {}

class CustomSound : public SoundDriver
{

public:
	CustomSound(){}
	virtual ~CustomSound(){}

	bool init(long sampleRate){ return true; }
	void pause(){}
	void reset(){}
	void resume(){}
	void write(u16 * finalWave, int length)
	{
		int samples = length / sizeof(u16);
		soundWave.reserve(soundWave.size() + samples);
		std::copy(finalWave, finalWave + samples, std::back_inserter(soundWave));
	}
};

SoundDriver * systemSoundInit()
{
	return new CustomSound();
}

void systemOnWriteDataToSoundBuffer(const u16 * finalWave, int length){}
void systemOnSoundShutdown(){}
void systemScreenMessage(const char *){}
void systemUpdateMotionSensor(){}
int systemGetSensorX(){ return 0; }
int systemGetSensorY(){ return 0; }
bool systemCanChangeSoundQuality(){ return true; }
void systemShowSpeed(int){}
void system10Frames(int)
{

}

void systemFrame(){}
void systemGbBorderOn(){}
void Sm60FPS_Init(){}
bool Sm60FPS_CanSkipFrame(){ return false; }
void Sm60FPS_Sleep(){}
void DbgMsg(const char *msg, ...){}
void winlog(const char *, ...){}
void(*dbgOutput)(const char *s, u32 addr);
void(*dbgSignal)(int sig, int number);

//u32 systemGetClock()
//{
//	return GetTickCount64();
//}

u16 systemColorMap16[0x10000];
u32 systemColorMap32[0x10000];
u16 systemGbPalette[24];
int systemRedShift = 0;
int systemGreenShift = 0;
int systemBlueShift = 0;
int systemColorDepth = 0;
int systemDebug = 0;
int systemVerbose = 0;
int systemFrameSkip = 0;
int systemSaveUpdateCounter = 0;
int systemSpeed = 0;

int emulating = 0;
bool debugger = false;

gzFile ZEXPORT gzdopen OF((int fd, const char *mode)){ return 0; }
int ZEXPORT gzread OF((gzFile file, voidp buf, unsigned len)){ return 0; }
int ZEXPORT gzwrite OF((gzFile file, voidpc buf, unsigned len)){ return 0; }
int ZEXPORT gzclose OF((gzFile file)){ return 0; }
gzFile ZEXPORT gzopen OF((const char *path, const char *mode)){ return 0; }
z_off_t ZEXPORT gzseek OF((gzFile file, z_off_t offset, int whence)){ return 0; }


u8 *utilLoad(const char *file,
	bool(*accept)(const char *),
	u8 *data,
	int &size)
{
	return 0;
}

static const EmulatorKey::Type KEYS_ORDER[] =
{
	EmulatorKey::A,
	EmulatorKey::B,
	EmulatorKey::SELECT,
	EmulatorKey::START,
	EmulatorKey::RIGHT,
	EmulatorKey::LEFT,
	EmulatorKey::UP,
	EmulatorKey::DOWN,
	EmulatorKey::R,
	EmulatorKey::L
};

Gba::Gba(const RomInfo& info)
: m_romInfo(info)
, m_bufferedSamples(0)
{
	m_romInfo.width = GBA_EMULATOR_WIDTH;
	m_romInfo.height = GBA_EMULATOR_HEIGHT;
}

extern long soundSampleRate;

bool Gba::LoadRom(const RomBuffer& romBuffer)
{
	try
	{
		soundSampleRate = m_romInfo.samplerate;

		m_soundBuffer.reset(new short int[2 * 1024 * 4]);

		m_screenBuffer.reset(new uint8_t[GBA_EMULATOR_WIDTH * GBA_EMULATOR_HEIGHT * 4]);

		systemRedShift = 19;
		systemGreenShift = 11;
		systemBlueShift = 3;
		systemColorDepth = 32;

		utilUpdateSystemColorMaps(true);

		m_rom = romBuffer;
		CPULoadRom(&m_rom.at(0), m_rom.size());

		flashSetSize(0x20000);
		doMirroring(false);

		soundInit();
		CPUInit("", false);
		CPUReset();

		return true;
	}
	catch (...)
	{
	}
	return false;

}

ScreenBuffer Gba::Tick(const EmulatorKeyBitset& keys)
{
	static const EmulatorKeyBitset temp;
	return Tick(keys, temp);
}

ScreenBuffer Gba::Tick(const EmulatorKeyBitset& keys, const EmulatorKeyBitset& player2)
{
	inputFlags = 0;
	int index = 0;
	for (auto key : KEYS_ORDER)
	{
		if (keys.IsKeyDown(key))
			inputFlags |= 1 << index;

		index++;
	}

	while (drawScreenCalled == 0)
		CPULoop(250000);

	drawScreenCalled = 0;

	memcpy(m_screenBuffer.get(), pix + GBA_EMULATOR_WIDTH * 4, GBA_EMULATOR_WIDTH * GBA_EMULATOR_HEIGHT * 4);

	int numSamples = soundWave.size();
	if (m_bufferedSamples + numSamples <= MAX_SAMPLES)
	{
		short int* buffer = m_soundBuffer.get();
		for (int index = 0; index < numSamples; index++)
		{
			buffer[index + m_bufferedSamples] = (s16)soundWave[index];
		}

		m_bufferedSamples += numSamples;
	}

	return m_screenBuffer.get();
}

SoundBuffer Gba::GetSoundBuffer()
{
	SoundBuffer temp(m_soundBuffer.get(), m_soundBuffer.get() + m_bufferedSamples);
	m_bufferedSamples = 0;
	soundWave.clear();
	return temp;
}

const RomInfo& Gba::GetRomInfo() const
{
	return m_romInfo;
}

StateBuffer Gba::SaveState() const
{
	StateBuffer state(MIN_STATE_BUFFER_SIZE);

	size_t size = CPUWriteMemState((char*)&state[0], state.size());

	if (size < state.size())
		state.resize(size);

	return state;
}

void Gba::LoadState(const StateBuffer& state)
{
	CPUReadMemState((char*)&state[0], state.size());
}

Gbx::Gbx(const RomInfo& info)
: m_romInfo(info)
, m_bufferedSamples(0)
{
	m_romInfo.width = GBX_EMULATOR_WIDTH;
	m_romInfo.height = GBX_EMULATOR_HEIGHT;
}

bool Gbx::LoadRom(const RomBuffer& romBuffer)
{
	try
	{
		soundSampleRate = m_romInfo.samplerate;

		m_soundBuffer.reset(new short int[2 * 1024 * 4]);

		m_screenBuffer.reset(new uint8_t[GBX_EMULATOR_WIDTH * GBX_EMULATOR_HEIGHT * 4]);

		for (int i = 0; i < 24;) {
			systemGbPalette[i++] = (0x1f) | (0x1f << 5) | (0x1f << 10);
			systemGbPalette[i++] = (0x15) | (0x15 << 5) | (0x15 << 10);
			systemGbPalette[i++] = (0x0c) | (0x0c << 5) | (0x0c << 10);
			systemGbPalette[i++] = 0;
		}

		systemRedShift = 19;
		systemGreenShift = 11;
		systemBlueShift = 3;
		systemColorDepth = 32;

		utilUpdateSystemColorMaps(true);

		m_rom = romBuffer;

		soundInit();

		bool failed = !gbLoadRom(&m_rom.at(0), m_rom.size());
		if (!failed)
		{
			gbGetHardwareType();

			// used for the handling of the gb Boot Rom
			if (gbHardware & 5)
				gbCPUInit("", false);

			gbReset();

			return true;
		}

		return false;
	}
	catch (...)
	{
	}
	return false;

}

ScreenBuffer Gbx::Tick(const EmulatorKeyBitset& keys)
{
	static const EmulatorKeyBitset temp;
	return Tick(keys, temp);
}

ScreenBuffer Gbx::Tick(const EmulatorKeyBitset& keys, const EmulatorKeyBitset& player2)
{
	inputFlags = 0;
	int index = 0;
	for (auto key : KEYS_ORDER)
	{
		if (keys.IsKeyDown(key))
			inputFlags |= 1 << index;

		index++;
	}

	while (drawScreenCalled == 0)
		gbEmulate(70000/4);

	drawScreenCalled = 0;

	memcpy(m_screenBuffer.get(), pix + GBX_EMULATOR_WIDTH * 4, GBX_EMULATOR_WIDTH * GBX_EMULATOR_HEIGHT * 4);

	int numSamples = soundWave.size();

	if (m_bufferedSamples + numSamples <= MAX_SAMPLES)
	{
		short int* buffer = m_soundBuffer.get();
		for (int index = 0; index < numSamples; index++)
		{
			buffer[index + m_bufferedSamples] = (s16)soundWave[index];
		}

		m_bufferedSamples += numSamples;
	}

	return m_screenBuffer.get();
}

SoundBuffer Gbx::GetSoundBuffer()
{
	SoundBuffer temp(m_soundBuffer.get(), m_soundBuffer.get() + m_bufferedSamples);
	m_bufferedSamples = 0;
	soundWave.clear();
	return temp;
}

const RomInfo& Gbx::GetRomInfo() const
{
	return m_romInfo;
}

StateBuffer Gbx::SaveState() const
{
	StateBuffer state(MIN_STATE_BUFFER_SIZE);

	size_t size = gbWriteMemSaveState((char*)&state[0], state.size());

	if (size < state.size())
		state.resize(size);

	return state;
}

void Gbx::LoadState(const StateBuffer& state)
{
	gbReadMemSaveState((char*)&state[0], state.size());
}
