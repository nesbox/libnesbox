#ifndef __SEGA_H__
#define __SEGA_H__

#include "core/md.h"
#include "../../include/nesbox.h"

#include <vector>
#include <map>

namespace sega
{
	class Sega : public Emulator
	{
	public:
		virtual bool LoadRom(const RomBuffer& romBuffer);
		virtual ScreenBuffer Tick(const EmulatorKeyBitset& keys);
		virtual ScreenBuffer Tick(const EmulatorKeyBitset& player1, const EmulatorKeyBitset& player2);
		virtual SoundBuffer GetSoundBuffer();
		virtual const RomInfo& GetRomInfo() const;
		virtual StateBuffer SaveState() const;
		virtual void LoadState(const StateBuffer& state);

		Sega(const RomInfo& info);
	private:
		void MixSamples(short int *buffer, int sample_count, int offset);

	private:
		std::unique_ptr<md> m_megad;
		bmap m_mdscr;
		sndinfo m_sndi;
		std::unique_ptr<short int[]> m_soundBuffer;
		int m_bufferedSamples;
		std::unique_ptr<uint8_t[]> m_screenBuffer;

		RomBuffer m_rom;
		RomInfo m_romInfo;
	};
}

#endif // __SEGA_H__