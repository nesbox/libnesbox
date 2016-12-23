#include "Sega.h"

#define IS_MAIN_CPP

#include "core/md.h"
#include "core/rc-vars.h"
#include <memory>
#include <assert.h>
#include <algorithm>
#include <stdlib.h>

using namespace sega;

static const int EMULATOR_WIDTH = 320;
static const int EMULATOR_HEIGHT = 224; 
static const int MAX_SAMPLES = 4096;
static const int PAD_UNTOUCHED = 0xf303f;
static const int MIN_STATE_BUFFER_SIZE = 0x22478;

static const EmulatorKey::Type KEYS_ORDER[] =
{
	EmulatorKey::UP,
	EmulatorKey::DOWN,
	EmulatorKey::LEFT,
	EmulatorKey::RIGHT,

	EmulatorKey::B,
	EmulatorKey::C,
	EmulatorKey::A,
	EmulatorKey::START,

	EmulatorKey::Z,
	EmulatorKey::Y,
	EmulatorKey::X,
	EmulatorKey::MODE

};

static const int KEY_OFFSETS[] =
{
	1 << 0,		// UP,
	1 << 1,		// DOWN,
	1 << 2,		// LEFT,
	1 << 3,		// RIGHT,
	1 << 4,		// B,
	1 << 5,		// C,
	1 << 12,	// A,
	1 << 13,	// START,
	1 << 16,	// Z,
	1 << 17,	// Y,
	1 << 18,	// X,
	1 << 19		// MODE
};				   

Sega::Sega(const RomInfo& info)
: m_romInfo(info)
, m_bufferedSamples(0)
{
	m_romInfo.width = EMULATOR_WIDTH;
	m_romInfo.height = EMULATOR_HEIGHT;
}

bool Sega::LoadRom(const RomBuffer& romBuffer)
{
	try
	{
		dgen_soundrate = m_romInfo.samplerate;
		dgen_pal = m_romInfo.isPal() ? 1 : 0;
		dgen_region = m_romInfo.region.empty() ? '\0' : (m_romInfo.region.at(0) & ~(0x20));

		m_megad.reset(new md(dgen_pal, dgen_region));

		if (!m_megad->okay())
		{
			return false;
		}

		m_rom = romBuffer;

		{
			uint8_t *rom = &m_rom[0];
			size_t size = m_rom.size();

			if (memcmp(&rom[0x100], "SEGA", 4)) 
			{
				uint8_t *dst = rom;
				uint8_t *src = &rom[0x200];
				size_t chunks = ((size - 0x200) / 0x4000);

				if (((size - 0x200) & (0x4000 - 1)) != 0)
					return false;

				size -= 0x200;

				while (chunks) 
				{
					size_t i;
					uint8_t tmp[0x2000];

					memcpy(tmp, src, 0x2000);
					src += 0x2000;
					for (i = 0; (i != 0x2000); ++i) 
					{
						*(dst++) = *(src++);
						*(dst++) = tmp[i];
					}
					--chunks;
				}

				if (memcmp(&rom[0x100], "SEGA", 4)) 
				{
					return false;
				}
			}
		}

		m_megad->load(&m_rom.at(0), m_rom.size());

		m_megad->fix_rom_checksum();

		m_mdscr.bpp = 32;
		m_mdscr.w = (EMULATOR_WIDTH + 16);
		m_mdscr.h = (NTSC_VBLANK + 16);
		m_mdscr.pitch = (m_mdscr.w * 4);
		m_mdscr.data = (uint8_t *)malloc(m_mdscr.pitch * 256);
		m_screenBuffer.reset(new uint8_t[EMULATOR_WIDTH * EMULATOR_HEIGHT * 4]);

		memset(m_mdscr.data, 0, m_mdscr.pitch * 256);

		m_sndi.len = dgen_soundrate / (dgen_pal ? PAL_HZ : NTSC_HZ);
		m_sndi.lr = (int16_t *)calloc(2, (m_sndi.len * sizeof(m_sndi.lr[0])));
		m_soundBuffer.reset(new short int[2 * 1024 * 4]);
		return true;
	}
	catch (...)
	{
	}
	return false;

}

static unsigned int proccessGamepad(const EmulatorKeyBitset& keys)
{
	unsigned int flags = PAD_UNTOUCHED;
	
	int index = 0;
	for (auto key : KEYS_ORDER)
	{
		if (keys.IsKeyDown(key))
			flags &= ~KEY_OFFSETS[index];

		index++;
	}

	return flags;
}

ScreenBuffer Sega::Tick(const EmulatorKeyBitset& keys)
{
	static const EmulatorKeyBitset temp;
	return Tick(keys, temp);
}


ScreenBuffer Sega::Tick(const EmulatorKeyBitset& player1, const EmulatorKeyBitset& player2)
{
	m_megad->pad[0] = proccessGamepad(player1);
	m_megad->pad[1] = proccessGamepad(player2);

	m_megad->one_frame(&m_mdscr, NULL, &m_sndi);

	uint8_t* src = m_mdscr.data + ((EMULATOR_WIDTH + 16) * 8 + 4) * 4;
	uint8_t* dst = m_screenBuffer.get();

	for (int line = 0; line < EMULATOR_HEIGHT; line++)
	{
		memcpy(dst, src, EMULATOR_WIDTH * 4);
		dst += EMULATOR_WIDTH * 4;
		src += (EMULATOR_WIDTH + 16) * 4;
	}

	int numSamples = m_sndi.len;
	if (m_bufferedSamples + numSamples <= MAX_SAMPLES)
	{
		MixSamples(m_soundBuffer.get(), numSamples << 1, m_bufferedSamples << 1);
		m_bufferedSamples += numSamples;
	}

	return m_screenBuffer.get();
}

SoundBuffer Sega::GetSoundBuffer()
{
	SoundBuffer temp(m_soundBuffer.get(), m_soundBuffer.get() + (m_bufferedSamples << 1));
	m_bufferedSamples = 0;
	return temp;
}

void Sega::MixSamples(short int *buffer, int sample_count, int offset)
{
	for (int index = 0; index < sample_count; index++)
	{
		((short int *)buffer)[index + offset] = m_sndi.lr[index];  // Bound the signed short to [0-1]
	}
}

const RomInfo& Sega::GetRomInfo() const
{
	return m_romInfo;
}

StateBuffer Sega::SaveState() const
{
	StateBuffer state(MIN_STATE_BUFFER_SIZE);

	m_megad->export_gst(&state[0]);

	return state;
}

void Sega::LoadState(const StateBuffer& state)
{
	m_megad->import_gst((unsigned char*)&state[0]);
}