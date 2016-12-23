#include "../include/nesbox.h"
#include "sega/Sega.h"
#include "snes/Snes.h"
#include "nes/Nes.h"
#include "gb/GameBoy.h"
#include <algorithm>
#include <cctype>

static const wchar_t* UNDEFINED_REGION = L"";
static const wchar_t* USA_REGION = L"U";
static const wchar_t* EUROPE_REGION = L"E";
static const wchar_t* JAPAN_REGION = L"J";
static const wchar_t* ALL_REGIONS[] = { USA_REGION, EUROPE_REGION, JAPAN_REGION };
static const int PAL_FRAMERATE = 50;
static const int NTSC_FRAMERATE = 60;
static const int DEFAULT_SAMPLERATE = 48000;

static const wchar_t* SEGA_EXT 		= L".gen";
static const wchar_t* SEGA_SMD_EXT 	= L".smd";
static const wchar_t* SEGA_BIN_EXT 	= L".bin";
static const wchar_t* SEGA_MD_EXT 	= L".md";
static const wchar_t* SNES_SMC_EXT 	= L".smc";
static const wchar_t* SNES_SFC_EXT 	= L".sfc";
static const wchar_t* NES_EXT 		= L".nes";
static const wchar_t* GBA_EXT 		= L".gba";
static const wchar_t* GBC_EXT 		= L".gbc";
static const wchar_t* GB_EXT 		= L".gb";

void EmulatorKeyBitset::SetKeyUp(EmulatorKey::Type key)
{
	m_bitset.set(key, false);
}

void EmulatorKeyBitset::SetKeyDown(EmulatorKey::Type key)
{
	m_bitset.set(key, true);
}

bool EmulatorKeyBitset::IsKeyDown(EmulatorKey::Type key) const
{
	return m_bitset.test(key);
}

void EmulatorKeyBitset::SetKeys(unsigned long data)
{
	m_bitset = data;
}

unsigned long EmulatorKeyBitset::GetKeys() const
{
	return m_bitset.to_ulong();
}

RomInfo::RomInfo()
: videoMode(NTSC)
, region(UNDEFINED_REGION)
, width(0)
, height(0)
, framerate(NTSC_FRAMERATE)
, samplerate(DEFAULT_SAMPLERATE)
, emulatorType(EmulatorType::UNKNOWN)
{

}

bool RomInfo::isPal() const
{
	return region == EUROPE_REGION;
}

static RomInfo ParseRomName(const std::wstring& name)
{
	RomInfo info;

	for (auto region : ALL_REGIONS)
	{
		std::wstring regionInBrackets(L"(");
		regionInBrackets.append(region);
		regionInBrackets.append(L")");

		if (name.find(regionInBrackets) != std::wstring::npos)
		{
			info.region = region;
			break;
		}
	}

	auto extPos = name.rfind('.');
	if (extPos != std::wstring::npos)
	{
		std::wstring ext = name.substr(extPos);
		std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

		if (ext == SEGA_EXT)
			info.emulatorType = EmulatorType::SEGA;

		else if (ext == SEGA_BIN_EXT)
			info.emulatorType = EmulatorType::SEGA;

		else if (ext == SEGA_SMD_EXT)
			info.emulatorType = EmulatorType::SEGA;

		else if (ext == SEGA_MD_EXT)
			info.emulatorType = EmulatorType::SEGA;

		else if (ext == SNES_SMC_EXT)
			info.emulatorType = EmulatorType::SNES;

		else if (ext == SNES_SFC_EXT)
			info.emulatorType = EmulatorType::SNES;

		else if (ext == NES_EXT)
			info.emulatorType = EmulatorType::NES;

		else if (ext == GBA_EXT)
			info.emulatorType = EmulatorType::GBA;

		else if (ext == GBC_EXT)
			info.emulatorType = EmulatorType::GBX;

		else if (ext == GB_EXT)
			info.emulatorType = EmulatorType::GBX;
	}
	

	bool pal = info.isPal();
	info.framerate = pal ? PAL_FRAMERATE : NTSC_FRAMERATE;

	return info;
}

EmulatorPtr Emulator::CreateByRomFile(const std::string& name, int samplerate)
{
	return CreateByRomFile(std::wstring(name.begin(), name.end()), samplerate);
}

EmulatorPtr Emulator::CreateByRomFile(const std::wstring& name, int samplerate)
{
	RomInfo info = ParseRomName(name);

	if (samplerate) info.samplerate = samplerate;

	switch (info.emulatorType)
	{
	case EmulatorType::SEGA:
		return EmulatorPtr(new sega::Sega(info));
	case EmulatorType::SNES:
		return EmulatorPtr(new snes::Snes(info));
	case EmulatorType::NES:
		return EmulatorPtr(new nes::Nes(info));
	case EmulatorType::GBA:
		return EmulatorPtr(new gb::Gba(info));
	case EmulatorType::GBX:
		return EmulatorPtr(new gb::Gbx(info));
	default:
		return EmulatorPtr();
	}

	return EmulatorPtr();
}
