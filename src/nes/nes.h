#ifndef __NES_H__
#define __NES_H__

#include "../../include/nesbox.h"

namespace nes
{
    class Nes : public Emulator
    {
    public:
		Nes(const RomInfo& info);

		virtual bool LoadRom(const RomBuffer& romBuffer);
		virtual ScreenBuffer Tick(const EmulatorKeyBitset& keys);
		virtual ScreenBuffer Tick(const EmulatorKeyBitset& player1, const EmulatorKeyBitset& player2);
		virtual SoundBuffer GetSoundBuffer();
		virtual const RomInfo& GetRomInfo() const;
		virtual StateBuffer SaveState() const;
		virtual void LoadState(const StateBuffer& state);

	private:
		void mixSamples(short int *buffer, int sample_count, int offset);

	private:

		std::unique_ptr<unsigned int[]> m_screenBuffer;
		int* m_soundBuffer;
		std::unique_ptr<short int[]> m_soundStream;
		unsigned int m_inputFlags;
		int m_bufferedSamples;

		RomInfo m_romInfo;
		RomBuffer m_rom;
    };
}

#endif // __NES_H__