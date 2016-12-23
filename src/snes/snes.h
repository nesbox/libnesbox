#ifndef __SNES_H__
#define __SNES_H__

#include "../../include/nesbox.h"

namespace snes
{
    class Snes : public Emulator
    {
    public:
		Snes(const RomInfo& info);

		virtual bool LoadRom(const RomBuffer& romBuffer);
		virtual ScreenBuffer Tick(const EmulatorKeyBitset& keys);
		virtual ScreenBuffer Tick(const EmulatorKeyBitset& player1, const EmulatorKeyBitset& player2);
		virtual SoundBuffer GetSoundBuffer();
		virtual const RomInfo& GetRomInfo() const;
		virtual StateBuffer SaveState() const;
		virtual void LoadState(const StateBuffer& state);

	private:
		void ResetKeymap();

	private:
		RomInfo m_romInfo;
		RomBuffer m_rom;
		EmulatorKeyBitset m_prevKeys1;
		EmulatorKeyBitset m_prevKeys2;
    };
}

#endif // __SNES_H__