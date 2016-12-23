#ifndef __GB_H__
#define __GB_H__

#include "../../include/nesbox.h"

#include <vector>
#include <map>

namespace gb
{
	class Gba : public Emulator
	{
	public:
		virtual bool LoadRom(const RomBuffer& romBuffer);
		virtual ScreenBuffer Tick(const EmulatorKeyBitset& keys);
		virtual ScreenBuffer Tick(const EmulatorKeyBitset& player1, const EmulatorKeyBitset& player2);
		virtual SoundBuffer GetSoundBuffer();
		virtual const RomInfo& GetRomInfo() const;
		virtual StateBuffer SaveState() const;
		virtual void LoadState(const StateBuffer& state);

		Gba(const RomInfo& info);

	private:
		std::unique_ptr<short int[]> m_soundBuffer;
		int m_bufferedSamples;
		std::unique_ptr<uint8_t[]> m_screenBuffer;

		RomBuffer m_rom;
		RomInfo m_romInfo;
	};

	class Gbx : public Emulator
	{
	public:
		virtual bool LoadRom(const RomBuffer& romBuffer);
		virtual ScreenBuffer Tick(const EmulatorKeyBitset& keys);
		virtual ScreenBuffer Tick(const EmulatorKeyBitset& player1, const EmulatorKeyBitset& player2);
		virtual SoundBuffer GetSoundBuffer();
		virtual const RomInfo& GetRomInfo() const;
		virtual StateBuffer SaveState() const;
		virtual void LoadState(const StateBuffer& state);

		Gbx(const RomInfo& info);

	private:
		std::unique_ptr<short int[]> m_soundBuffer;
		int m_bufferedSamples;
		std::unique_ptr<uint8_t[]> m_screenBuffer;

		RomBuffer m_rom;
		RomInfo m_romInfo;
	};
}


#endif // __GB_H__