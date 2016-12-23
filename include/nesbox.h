#ifndef __NESBOX_H__
#define __NESBOX_H__

#include <vector>
#include <memory>
#include <bitset>

class Emulator;
typedef std::unique_ptr<Emulator> EmulatorPtr;
typedef unsigned char* ScreenBuffer; 
typedef std::vector<unsigned char> RomBuffer;
typedef std::vector<short int> SoundBuffer;
typedef std::vector<unsigned char> StateBuffer;

struct EmulatorType
{
	enum Type
	{
		SEGA,
		SNES,
		NES,
		GBA,
		GBX,
		UNKNOWN
	};
};

struct EmulatorKey
{
	enum Type
	{
		UP,
		DOWN,
		LEFT,
		RIGHT,
		START,
		SELECT,
		MODE,
		A,
		B,
		C,
		L,
		R,
		X,
		Y,
		Z,
		ALL
	};
};

class EmulatorKeyBitset
{
public:
	EmulatorKeyBitset() {}
	void SetKeyUp(EmulatorKey::Type key);
	void SetKeyDown(EmulatorKey::Type key);
	bool IsKeyDown(EmulatorKey::Type key) const;
	void SetKeys(unsigned long data);
	unsigned long GetKeys() const;

private:
	std::bitset<EmulatorKey::ALL> m_bitset;
};

struct RomInfo
{
	enum VideoModeType
	{
		Unknown,
		PAL,
		NTSC
	};

	RomInfo();
	bool isPal() const;

	std::wstring region;
	VideoModeType videoMode;
	int width;
	int height;
	int framerate;
	int samplerate;
	::EmulatorType::Type emulatorType;
};

class Emulator
{
public:
	virtual bool LoadRom(const RomBuffer& romBuffer) = 0;
	virtual ScreenBuffer Tick(const EmulatorKeyBitset& keys) = 0;
	virtual ScreenBuffer Tick(const EmulatorKeyBitset& player1, const EmulatorKeyBitset& player2) = 0;
	virtual SoundBuffer GetSoundBuffer() = 0;
	virtual const RomInfo& GetRomInfo() const = 0;

	virtual StateBuffer SaveState() const = 0;
	virtual void LoadState(const StateBuffer& state) = 0;

	virtual ~Emulator(){}

	static EmulatorPtr CreateByRomFile(const std::string& name, int samplerate);
	static EmulatorPtr CreateByRomFile(const std::wstring& name, int samplerate);
};

typedef std::vector<unsigned char> SevenZBuffer;
typedef std::vector<std::wstring> SevenZList;

class SevenZDecoder
{
public:

	SevenZDecoder(const SevenZBuffer& buffer);

	SevenZList list() const;
	void extract(const std::wstring& name, SevenZBuffer& out);

private:
	SevenZBuffer m_buffer;
};

#endif // __NESBOX_H__