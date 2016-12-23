CC=g++
AR=ar rcu
CFLAGS=-O3 -std=gnu++0x
LFLAGS=-O3
RM=del /f /q

NESBOX_LIB=nesbox.lib
NESBOX_LIB_SOURCES= \
	..\..\src\nesbox.cpp

GB_LIB=gb.lib
GB_DEFS= \
	-DFINAL_VERSION \
	-DNO_PNG \
	-DNO_LINK \
	-DC_CORE \
	-DNESBOX_JS

GB_SOURCES= \
	..\..\src\gb\core\apu\Blip_Buffer.cpp \
	..\..\src\gb\core\apu\Effects_Buffer.cpp \
	..\..\src\gb\core\apu\Gb_Apu.cpp \
	..\..\src\gb\core\apu\Gb_Apu_State.cpp \
	..\..\src\gb\core\apu\Gb_Oscs.cpp \
	..\..\src\gb\core\apu\Multi_Buffer.cpp \
	..\..\src\gb\core\gb\GB.cpp \
	..\..\src\gb\core\gb\gbCheats.cpp \
	..\..\src\gb\core\gb\gbDis.cpp \
	..\..\src\gb\core\gb\gbGfx.cpp \
	..\..\src\gb\core\gb\gbGlobals.cpp \
	..\..\src\gb\core\gb\gbMemory.cpp \
	..\..\src\gb\core\gb\gbPrinter.cpp \
	..\..\src\gb\core\gb\gbSGB.cpp \
	..\..\src\gb\core\gb\gbSound.cpp \
	..\..\src\gb\core\gba\agbprint.cpp \
	..\..\src\gb\core\gba\armdis.cpp \
	..\..\src\gb\core\gba\bios.cpp \
	..\..\src\gb\core\gba\gbaCheats.cpp \
	..\..\src\gb\core\gba\CheatSearch.cpp \
	..\..\src\gb\core\gba\EEprom.cpp \
	..\..\src\gb\core\gba\elf.cpp \
	..\..\src\gb\core\gba\gbaFlash.cpp \
	..\..\src\gb\core\gba\GBA.cpp \
	..\..\src\gb\core\gba\GBA-arm.cpp \
	..\..\src\gb\core\gba\gbafilter.cpp \
	..\..\src\gb\core\gba\GBAGfx.cpp \
	..\..\src\gb\core\gba\GBALink.cpp \
	..\..\src\gb\core\gba\GBASockClient.cpp \
	..\..\src\gb\core\gba\GBA-thumb.cpp \
	..\..\src\gb\core\gba\gbaGlobals.cpp \
	..\..\src\gb\core\gba\Mode0.cpp \
	..\..\src\gb\core\gba\Mode1.cpp \
	..\..\src\gb\core\gba\Mode2.cpp \
	..\..\src\gb\core\gba\Mode3.cpp \
	..\..\src\gb\core\gba\Mode4.cpp \
	..\..\src\gb\core\gba\Mode5.cpp \
	..\..\src\gb\core\gba\gbaSound.cpp \
	..\..\src\gb\core\gba\Sram.cpp \
	..\..\src\gb\core\Util.cpp \
	..\..\src\gb\gameboy.cpp


SEGA_LIB=sega.lib
SEGA_DEFS= \
	-DWITH_CZ80 \
	-DWITH_MUSA \

SEGA_SOURCES= \
	..\..\src\sega\core\cz80\cz80.cpp \
	..\..\src\sega\core\fm.cpp \
	..\..\src\sega\core\md.cpp \
	..\..\src\sega\core\mdfr.cpp \
	..\..\src\sega\core\mem.cpp \
	..\..\src\sega\core\musa\m68kcpu.cpp \
	..\..\src\sega\core\musa\m68kopac.cpp \
	..\..\src\sega\core\musa\m68kopdm.cpp \
	..\..\src\sega\core\musa\m68kopnz.cpp \
	..\..\src\sega\core\musa\m68kops.cpp \
	..\..\src\sega\core\myfm.cpp \
	..\..\src\sega\core\ras.cpp \
	..\..\src\sega\core\save.cpp \
	..\..\src\sega\core\sn76496.cpp \
	..\..\src\sega\core\vdp.cpp \
	..\..\src\sega\sega.cpp

SNES_LIB=snes.lib
SNES_DEFS= \
	-D__FLASH__ \
	-DHAVE_STDINT_H \
	-DHAVE_SYS_IOCTL_H \
	-DUSE_VIDMODE_EXTENSION \
	-DUSE_DGA_EXTENSION \
	-DHAVE_MKSTEMP \
	-DNEW_COLOUR_BLENDING \
	-DNOASM \
	-DSPC700_SHUTDOWN \
	-DMITSHM \
	-DNESBOX_JS

SNES_SOURCES=\
	..\..\src\snes\core\2xsai.cpp \
	..\..\src\snes\core\apu.cpp \
	..\..\src\snes\core\apudebug.cpp \
	..\..\src\snes\core\bsx.cpp \
	..\..\src\snes\core\c4.cpp \
	..\..\src\snes\core\c4emu.cpp \
	..\..\src\snes\core\cheats.cpp \
	..\..\src\snes\core\cheats2.cpp \
	..\..\src\snes\core\clip.cpp \
	..\..\src\snes\core\conffile.cpp \
	..\..\src\snes\core\controls.cpp \
	..\..\src\snes\core\cpu.cpp \
	..\..\src\snes\core\cpuexec.cpp \
	..\..\src\snes\core\cpuops.cpp \
	..\..\src\snes\core\crosshairs.cpp \
	..\..\src\snes\core\data.cpp \
	..\..\src\snes\core\debug.cpp \
	..\..\src\snes\core\dma.cpp \
	..\..\src\snes\core\dsp1.cpp \
	..\..\src\snes\core\fxdbg.cpp \
	..\..\src\snes\core\fxemu.cpp \
	..\..\src\snes\core\fxinst.cpp \
	..\..\src\snes\core\gfx.cpp \
	..\..\src\snes\core\globals.cpp \
	..\..\src\snes\core\loadzip.cpp \
	..\..\src\snes\core\logger.cpp \
	..\..\src\snes\core\memmap.cpp \
	..\..\src\snes\core\movie.cpp \
	..\..\src\snes\core\netplay.cpp \
	..\..\src\snes\core\obc1.cpp \
	..\..\src\snes\core\ppu.cpp \
	..\..\src\snes\core\reader.cpp \
	..\..\src\snes\core\sa1.cpp \
	..\..\src\snes\core\sa1cpu.cpp \
	..\..\src\snes\core\screenshot.cpp \
	..\..\src\snes\core\sdd1.cpp \
	..\..\src\snes\core\sdd1emu.cpp \
	..\..\src\snes\core\server.cpp \
	..\..\src\snes\core\seta.cpp \
	..\..\src\snes\core\seta010.cpp \
	..\..\src\snes\core\seta011.cpp \
	..\..\src\snes\core\seta018.cpp \
	..\..\src\snes\core\snaporig.cpp \
	..\..\src\snes\core\snapshot.cpp \
	..\..\src\snes\core\snes9x.cpp \
	..\..\src\snes\core\soundux.cpp \
	..\..\src\snes\core\spc700.cpp \
	..\..\src\snes\core\spc7110.cpp \
	..\..\src\snes\core\srtc.cpp \
	..\..\src\snes\core\tile.cpp \
	..\..\src\snes\snes.cpp

NES_LIB=nes.lib
NES_DEFS= \
	-DNETWORK \
	-DLSB_FIRST

NES_SOURCES=\
	..\..\src\nes\core\boards\01-222.cpp \
	..\..\src\nes\core\boards\103.cpp \
	..\..\src\nes\core\boards\106.cpp \
	..\..\src\nes\core\boards\108.cpp \
	..\..\src\nes\core\boards\112.cpp \
	..\..\src\nes\core\boards\117.cpp \
	..\..\src\nes\core\boards\120.cpp \
	..\..\src\nes\core\boards\121.cpp \
	..\..\src\nes\core\boards\15.cpp \
	..\..\src\nes\core\boards\164.cpp \
	..\..\src\nes\core\boards\175.cpp \
	..\..\src\nes\core\boards\176.cpp \
	..\..\src\nes\core\boards\177.cpp \
	..\..\src\nes\core\boards\178.cpp \
	..\..\src\nes\core\boards\179.cpp \
	..\..\src\nes\core\boards\183.cpp \
	..\..\src\nes\core\boards\185.cpp \
	..\..\src\nes\core\boards\186.cpp \
	..\..\src\nes\core\boards\187.cpp \
	..\..\src\nes\core\boards\189.cpp \
	..\..\src\nes\core\boards\199.cpp \
	..\..\src\nes\core\boards\208.cpp \
	..\..\src\nes\core\boards\222.cpp \
	..\..\src\nes\core\boards\23.cpp \
	..\..\src\nes\core\boards\235.cpp \
	..\..\src\nes\core\boards\3d-block.cpp \
	..\..\src\nes\core\boards\411120-c.cpp \
	..\..\src\nes\core\boards\43.cpp \
	..\..\src\nes\core\boards\57.cpp \
	..\..\src\nes\core\boards\603-5052.cpp \
	..\..\src\nes\core\boards\68.cpp \
	..\..\src\nes\core\boards\8157.cpp \
	..\..\src\nes\core\boards\8237.cpp \
	..\..\src\nes\core\boards\830118C.cpp \
	..\..\src\nes\core\boards\88.cpp \
	..\..\src\nes\core\boards\90.cpp \
	..\..\src\nes\core\boards\95.cpp \
	..\..\src\nes\core\boards\a9746.cpp \
	..\..\src\nes\core\boards\addrlatch.cpp \
	..\..\src\nes\core\boards\ax5705.cpp \
	..\..\src\nes\core\boards\bandai.cpp \
	..\..\src\nes\core\boards\bmc13in1jy110.cpp \
	..\..\src\nes\core\boards\bmc42in1r.cpp \
	..\..\src\nes\core\boards\bmc64in1nr.cpp \
	..\..\src\nes\core\boards\bmc70in1.cpp \
	..\..\src\nes\core\boards\bonza.cpp \
	..\..\src\nes\core\boards\bs-5.cpp \
	..\..\src\nes\core\boards\copyfami_mmc3.cpp \
	..\..\src\nes\core\boards\dance.cpp \
	..\..\src\nes\core\boards\datalatch.cpp \
	..\..\src\nes\core\boards\deirom.cpp \
	..\..\src\nes\core\boards\dream.cpp \
	..\..\src\nes\core\boards\edu2000.cpp \
	..\..\src\nes\core\boards\fk23c.cpp \
	..\..\src\nes\core\boards\ghostbusters63in1.cpp \
	..\..\src\nes\core\boards\gs-2004.cpp \
	..\..\src\nes\core\boards\gs-2013.cpp \
	..\..\src\nes\core\boards\h2288.cpp \
	..\..\src\nes\core\boards\karaoke.cpp \
	..\..\src\nes\core\boards\kof97.cpp \
	..\..\src\nes\core\boards\konami-qtai.cpp \
	..\..\src\nes\core\boards\ks7032.cpp \
	..\..\src\nes\core\boards\malee.cpp \
	..\..\src\nes\core\boards\mmc1.cpp \
	..\..\src\nes\core\boards\mmc3.cpp \
	..\..\src\nes\core\boards\mmc5.cpp \
	..\..\src\nes\core\boards\n-c22m.cpp \
	..\..\src\nes\core\boards\n106.cpp \
	..\..\src\nes\core\boards\n625092.cpp \
	..\..\src\nes\core\boards\novel.cpp \
	..\..\src\nes\core\boards\sachen.cpp \
	..\..\src\nes\core\boards\sc-127.cpp \
	..\..\src\nes\core\boards\sheroes.cpp \
	..\..\src\nes\core\boards\sl1632.cpp \
	..\..\src\nes\core\boards\smb2j.cpp \
	..\..\src\nes\core\boards\subor.cpp \
	..\..\src\nes\core\boards\super24.cpp \
	..\..\src\nes\core\boards\supervision.cpp \
	..\..\src\nes\core\boards\t-227-1.cpp \
	..\..\src\nes\core\boards\t-262.cpp \
	..\..\src\nes\core\boards\tengen.cpp \
	..\..\src\nes\core\boards\tf-1201.cpp \
	..\..\src\nes\core\boards\__dummy_mapper.cpp \
	..\..\src\nes\core\cart.cpp \
	..\..\src\nes\core\drawing.cpp \
	..\..\src\nes\core\fceu.cpp \
	..\..\src\nes\core\file.cpp \
	..\..\src\nes\core\ines.cpp \
	..\..\src\nes\core\input\arkanoid.cpp \
	..\..\src\nes\core\input\bworld.cpp \
	..\..\src\nes\core\input\cursor.cpp \
	..\..\src\nes\core\input\fkb.cpp \
	..\..\src\nes\core\input\ftrainer.cpp \
	..\..\src\nes\core\input\hypershot.cpp \
	..\..\src\nes\core\input\mahjong.cpp \
	..\..\src\nes\core\input\mouse.cpp \
	..\..\src\nes\core\input\oekakids.cpp \
	..\..\src\nes\core\input\powerpad.cpp \
	..\..\src\nes\core\input\quiz.cpp \
	..\..\src\nes\core\input\shadow.cpp \
	..\..\src\nes\core\input\suborkb.cpp \
	..\..\src\nes\core\input\toprider.cpp \
	..\..\src\nes\core\input.cpp \
	..\..\src\nes\core\mappers\151.cpp \
	..\..\src\nes\core\mappers\16.cpp \
	..\..\src\nes\core\mappers\17.cpp \
	..\..\src\nes\core\mappers\18.cpp \
	..\..\src\nes\core\mappers\193.cpp \
	..\..\src\nes\core\mappers\201.cpp \
	..\..\src\nes\core\mappers\202.cpp \
	..\..\src\nes\core\mappers\203.cpp \
	..\..\src\nes\core\mappers\204.cpp \
	..\..\src\nes\core\mappers\21.cpp \
	..\..\src\nes\core\mappers\212.cpp \
	..\..\src\nes\core\mappers\213.cpp \
	..\..\src\nes\core\mappers\214.cpp \
	..\..\src\nes\core\mappers\215.cpp \
	..\..\src\nes\core\mappers\217.cpp \
	..\..\src\nes\core\mappers\22.cpp \
	..\..\src\nes\core\mappers\225.cpp \
	..\..\src\nes\core\mappers\227.cpp \
	..\..\src\nes\core\mappers\228.cpp \
	..\..\src\nes\core\mappers\229.cpp \
	..\..\src\nes\core\mappers\230.cpp \
	..\..\src\nes\core\mappers\231.cpp \
	..\..\src\nes\core\mappers\232.cpp \
	..\..\src\nes\core\mappers\234.cpp \
	..\..\src\nes\core\mappers\241.cpp \
	..\..\src\nes\core\mappers\242.cpp \
	..\..\src\nes\core\mappers\244.cpp \
	..\..\src\nes\core\mappers\246.cpp \
	..\..\src\nes\core\mappers\24and26.cpp \
	..\..\src\nes\core\mappers\25.cpp \
	..\..\src\nes\core\mappers\255.cpp \
	..\..\src\nes\core\mappers\27.cpp \
	..\..\src\nes\core\mappers\32.cpp \
	..\..\src\nes\core\mappers\33.cpp \
	..\..\src\nes\core\mappers\40.cpp \
	..\..\src\nes\core\mappers\41.cpp \
	..\..\src\nes\core\mappers\42.cpp \
	..\..\src\nes\core\mappers\46.cpp \
	..\..\src\nes\core\mappers\50.cpp \
	..\..\src\nes\core\mappers\51.cpp \
	..\..\src\nes\core\mappers\59.cpp \
	..\..\src\nes\core\mappers\6.cpp \
	..\..\src\nes\core\mappers\60.cpp \
	..\..\src\nes\core\mappers\61.cpp \
	..\..\src\nes\core\mappers\62.cpp \
	..\..\src\nes\core\mappers\65.cpp \
	..\..\src\nes\core\mappers\67.cpp \
	..\..\src\nes\core\mappers\69.cpp \
	..\..\src\nes\core\mappers\71.cpp \
	..\..\src\nes\core\mappers\72.cpp \
	..\..\src\nes\core\mappers\73.cpp \
	..\..\src\nes\core\mappers\75.cpp \
	..\..\src\nes\core\mappers\76.cpp \
	..\..\src\nes\core\mappers\77.cpp \
	..\..\src\nes\core\mappers\79.cpp \
	..\..\src\nes\core\mappers\8.cpp \
	..\..\src\nes\core\mappers\80.cpp \
	..\..\src\nes\core\mappers\82.cpp \
	..\..\src\nes\core\mappers\83.cpp \
	..\..\src\nes\core\mappers\85.cpp \
	..\..\src\nes\core\mappers\86.cpp \
	..\..\src\nes\core\mappers\89.cpp \
	..\..\src\nes\core\mappers\91.cpp \
	..\..\src\nes\core\mappers\92.cpp \
	..\..\src\nes\core\mappers\97.cpp \
	..\..\src\nes\core\mappers\99.cpp \
	..\..\src\nes\core\mappers\emu2413.cpp \
	..\..\src\nes\core\mappers\mmc2and4.cpp \
	..\..\src\nes\core\mappers\simple.cpp \
	..\..\src\nes\core\palette.cpp \
	..\..\src\nes\core\nesppu.cpp \
	..\..\src\nes\core\sound.cpp \
	..\..\src\nes\core\state.cpp \
	..\..\src\nes\core\unif.cpp \
	..\..\src\nes\core\utils\ConvertUTF.cpp \
	..\..\src\nes\core\utils\crc32.cpp \
	..\..\src\nes\core\utils\endian.cpp \
	..\..\src\nes\core\utils\general.cpp \
	..\..\src\nes\core\utils\guid.cpp \
	..\..\src\nes\core\utils\md5.cpp \
	..\..\src\nes\core\utils\memory.cpp \
	..\..\src\nes\core\video.cpp \
	..\..\src\nes\core\vsuni.cpp \
	..\..\src\nes\core\x6502.cpp \
	..\..\src\nes\nes.cpp

all: $(NESBOX_LIB)

$(NESBOX_LIB): $(SEGA_SOURCES) $(SNES_SOURCES) $(NESBOX_LIB_SOURCES)
	$(RM) *.o
	$(CC) -c $(CFLAGS) $(SEGA_DEFS) $(SEGA_SOURCES)
	$(CC) -c $(CFLAGS) $(SNES_DEFS) $(SNES_SOURCES)
	$(CC) -c $(CFLAGS) $(NES_DEFS) $(NES_SOURCES)
	$(CC) -c $(CFLAGS) $(GB_DEFS) $(GB_SOURCES)
	$(CC) -c $(CFLAGS) $(NESBOX_LIB_SOURCES)
	$(AR) $@ *.o
	$(RM) *.o

clean:
	$(RM) \
		$(NESBOX_LIB) \
		*.o
