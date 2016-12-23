#include "Nes.h"

#include "core/driver.h"
#include "core/fceu.h"
#include "core/state.h"

#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <assert.h>

using namespace nes;

int NesSampleRate = 0;

static const int SCREEN_WIDTH  = 256;
static const int SCREEN_HEIGHT = 240;
static const uint32 SCREEN_BUFFER_SIZE = SCREEN_WIDTH * SCREEN_HEIGHT;
static const int SAMPLE_THRESHOLD = 2048;
static const int MAX_SAMPLES = 4096;

static uint32 colorPalette[256];

static const EmulatorKey::Type KEYS_ORDER[] = 
{ 
	EmulatorKey::A,
	EmulatorKey::B,
	EmulatorKey::SELECT,
	EmulatorKey::START,
	EmulatorKey::UP,
	EmulatorKey::DOWN,
	EmulatorKey::LEFT,
	EmulatorKey::RIGHT
};

void Nes::mixSamples(short int *buffer, int sample_count, int offset)
{
	int bufferIndex = 0;
	float sample = 0.0f;
	for (int index = 0; index < sample_count; index++)
	{
		sample = m_soundBuffer[index] * 2;
		buffer[bufferIndex++ + offset] = sample;
		buffer[bufferIndex++ + offset] = sample;
	}
}

FILE *FCEUD_UTF8fopen(const char *fn, const char *mode)
{
	return NULL;
}

std::iostream* FCEUD_UTF8_fstream(const char *n, const char *m)
{
	return NULL;
}

void FCEUD_SetPalette(uint8 index, uint8 r, uint8 g, uint8 b)
{
	colorPalette[index] = (0xff << 24) | (r << 16) | (g << 8) | b;
}

Nes::Nes(const RomInfo& info)
: m_romInfo(info)
, m_soundBuffer(nullptr)
, m_inputFlags(0)
, m_bufferedSamples(0)
{
	m_romInfo.width = SCREEN_WIDTH;
	m_romInfo.height = SCREEN_HEIGHT;

	m_screenBuffer.reset(new uint32[SCREEN_BUFFER_SIZE]);
}

bool Nes::LoadRom(const RomBuffer& romBuffer)
{
	try
	{
		NesSampleRate = m_romInfo.samplerate;

		FCEUI_Initialize();
		FCEUI_SetVidSystem(m_romInfo.isPal() ? 1 : 0);
		FCEUI_Sound(m_romInfo.samplerate);

		std::stringstream stream;

		m_rom = romBuffer;
		stream.write((const char*)&m_rom.at(0), m_rom.size());

		if (FCEUI_LoadGameVirtual(&stream, 1) == 0)
		{
			return false;
		}

		FCEUI_SetInput(0, SI_GAMEPAD, &m_inputFlags, 0);
		FCEUI_SetInput(1, SI_GAMEPAD, &m_inputFlags, 0);

		m_soundStream.reset(new short int[MAX_SAMPLES << 1]);
		memset(m_soundStream.get(), 0, sizeof(short int)* MAX_SAMPLES << 1);
		return true;
	}
	catch (...)
	{
	}
	return false;
}

ScreenBuffer Nes::Tick(const EmulatorKeyBitset& keys)
{
	static const EmulatorKeyBitset temp;
	return Tick(keys, temp);
}

static unsigned int proccessGamepad(const EmulatorKeyBitset& keys)
{
	unsigned int flags = 0;
	int index = 0;

	for (auto key : KEYS_ORDER)
	{
		if (keys.IsKeyDown(KEYS_ORDER[index]))
			flags |= (1 << index);

		index++;
	}

	return flags;
}

ScreenBuffer Nes::Tick(const EmulatorKeyBitset& player1, const EmulatorKeyBitset& player2)
{
	uint8 *gfx = 0;
	int32 ssize = 0;

	unsigned int inputFlagsFirst = proccessGamepad(player1);
	unsigned int inputFlagsSecond = proccessGamepad(player2);

	m_inputFlags = (inputFlagsFirst & 0xff) | ((inputFlagsSecond & 0xff) << 8);

	FCEUI_Emulate(&gfx, 0);
	FCEUI_EmulateSound(&m_soundBuffer, &ssize);

	uint32* buffer = m_screenBuffer.get();
	uint8* gfxBuffer = gfx;

	int index = 0;
	while (index++ < SCREEN_BUFFER_SIZE)
	{
		*(buffer++) = *(colorPalette + *(gfxBuffer++));
	}

	if (m_bufferedSamples + ssize <= MAX_SAMPLES)
	{
		mixSamples(m_soundStream.get(), ssize, m_bufferedSamples << 1);
		m_bufferedSamples += ssize;
	}

	return (uint8*)m_screenBuffer.get();
}

SoundBuffer Nes::GetSoundBuffer()
{
	SoundBuffer temp(m_soundStream.get(), m_soundStream.get() + (m_bufferedSamples << 1));
	m_bufferedSamples = 0;
	return temp;
}

const RomInfo& Nes::GetRomInfo() const
{
	return m_romInfo;
}

StateBuffer Nes::SaveState() const
{
	std::stringstream stream;

	FCEUSS_SaveMS(&stream);

	const std::string& data = stream.str();

	return StateBuffer(data.begin(), data.end());
}

void Nes::LoadState(const StateBuffer& state)
{
	std::stringstream stream(std::string(state.begin(), state.end()));

	FCEUSS_LoadFP(&stream);
}
