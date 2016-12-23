// DGen v1.10+
// Megadrive C++ module saving and loading

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "md.h"
#include "system.h"

void md::m68k_state_dump()
{
	/*
	  32 and 16-bit values must be stored LSB first even though M68K is
	  big-endian for compatibility with other emulators.
	*/
	switch (cpu_emu) {
		unsigned int i, j;

#ifdef WITH_MUSA
	case CPU_EMU_MUSA:
		md_set_musa(1);
		for (i = M68K_REG_D0, j = 0; (i <= M68K_REG_D7); ++i, ++j)
			m68k_state.d[j] =
				h2le32(m68k_get_reg(NULL, (m68k_register_t)i));
		for (i = M68K_REG_A0, j = 0; (i <= M68K_REG_A7); ++i, ++j)
			m68k_state.a[j] =
				h2le32(m68k_get_reg(NULL, (m68k_register_t)i));
		m68k_state.pc = h2le32(m68k_get_reg(NULL, M68K_REG_PC));
		m68k_state.sr = h2le16(m68k_get_reg(NULL, M68K_REG_SR));
		md_set_musa(0);
		break;
#endif
#ifdef WITH_STAR
	case CPU_EMU_STAR:
		(void)j;
		for (i = 0; (i < 8); ++i) {
			m68k_state.d[i] = h2le32(cpu.dreg[i]);
			m68k_state.a[i] = h2le32(cpu.areg[i]);
		}
		m68k_state.pc = h2le32(cpu.pc);
		m68k_state.sr = h2le16(cpu.sr);
		break;
#endif
	default:
		(void)i;
		(void)j;
		break;
	}
}

void md::m68k_state_restore()
{
	/* 32 and 16-bit values are stored LSB first. */
	switch (cpu_emu) {
		unsigned int i, j;

#ifdef WITH_MUSA
	case CPU_EMU_MUSA:
		md_set_musa(1);
		for (i = M68K_REG_D0, j = 0; (i <= M68K_REG_D7); ++i, ++j)
			m68k_set_reg((m68k_register_t)i,
				     le2h32(m68k_state.d[j]));
		for (i = M68K_REG_A0, j = 0; (i <= M68K_REG_A7); ++i, ++j)
			m68k_set_reg((m68k_register_t)i,
				     le2h32(m68k_state.a[j]));
		m68k_set_reg(M68K_REG_PC, le2h32(m68k_state.pc));
		m68k_set_reg(M68K_REG_SR, le2h16(m68k_state.sr));
		md_set_musa(0);
		break;
#endif
#ifdef WITH_STAR
	case CPU_EMU_STAR:
		(void)j;
		for (i = 0; (i < 8); ++i) {
			cpu.dreg[i] = le2h32(m68k_state.d[i]);
			cpu.areg[i] = le2h32(m68k_state.a[i]);
		}
		cpu.pc = le2h32(m68k_state.pc);
		cpu.sr = le2h16(m68k_state.sr);
		break;
#endif
	default:
		(void)i;
		(void)j;
		break;
	}
}

void md::z80_state_dump()
{
	/* 16-bit values must be stored LSB first. */
	switch (z80_core) {
#ifdef WITH_CZ80
	case Z80_CORE_CZ80:
		z80_state.alt[0].fa = h2le16(Cz80_Get_AF(&cz80));
		z80_state.alt[0].cb = h2le16(Cz80_Get_BC(&cz80));
		z80_state.alt[0].ed = h2le16(Cz80_Get_DE(&cz80));
		z80_state.alt[0].lh = h2le16(Cz80_Get_HL(&cz80));
		z80_state.alt[1].fa = h2le16(Cz80_Get_AF2(&cz80));
		z80_state.alt[1].cb = h2le16(Cz80_Get_BC2(&cz80));
		z80_state.alt[1].ed = h2le16(Cz80_Get_DE2(&cz80));
		z80_state.alt[1].lh = h2le16(Cz80_Get_HL2(&cz80));
		z80_state.ix = h2le16(Cz80_Get_IX(&cz80));
		z80_state.iy = h2le16(Cz80_Get_IY(&cz80));
		z80_state.sp = h2le16(Cz80_Get_SP(&cz80));
		z80_state.pc = h2le16(Cz80_Get_PC(&cz80));
		z80_state.r = Cz80_Get_R(&cz80);
		z80_state.i = Cz80_Get_I(&cz80);
		z80_state.iff = Cz80_Get_IFF(&cz80);
		z80_state.im = Cz80_Get_IM(&cz80);
		break;
#endif
#ifdef WITH_MZ80
	case Z80_CORE_MZ80:
		z80_state.alt[0].fa = h2le16(z80.z80AF);
		z80_state.alt[0].cb = h2le16(z80.z80BC);
		z80_state.alt[0].ed = h2le16(z80.z80DE);
		z80_state.alt[0].lh = h2le16(z80.z80HL);
		z80_state.alt[1].fa = h2le16(z80.z80afprime);
		z80_state.alt[1].cb = h2le16(z80.z80bcprime);
		z80_state.alt[1].ed = h2le16(z80.z80deprime);
		z80_state.alt[1].lh = h2le16(z80.z80hlprime);
		z80_state.ix = h2le16(z80.z80IX);
		z80_state.iy = h2le16(z80.z80IY);
		z80_state.sp = h2le16(z80.z80sp);
		z80_state.pc = h2le16(z80.z80pc);
		z80_state.r = z80.z80r;
		z80_state.i = z80.z80i;
		z80_state.iff = z80.z80iff;
		z80_state.im = z80.z80interruptMode;
		break;
#endif
	default:
		break;
	}
}

void md::z80_state_restore()
{
	/* 16-bit values are stored LSB first. */
	switch (z80_core) {
#ifdef WITH_CZ80
	case Z80_CORE_CZ80:
		Cz80_Set_AF(&cz80, le2h16(z80_state.alt[0].fa));
		Cz80_Set_BC(&cz80, le2h16(z80_state.alt[0].cb));
		Cz80_Set_DE(&cz80, le2h16(z80_state.alt[0].ed));
		Cz80_Set_HL(&cz80, le2h16(z80_state.alt[0].lh));
		Cz80_Set_AF2(&cz80, le2h16(z80_state.alt[1].fa));
		Cz80_Set_BC2(&cz80, le2h16(z80_state.alt[1].cb));
		Cz80_Set_DE2(&cz80, le2h16(z80_state.alt[1].ed));
		Cz80_Set_HL2(&cz80, le2h16(z80_state.alt[1].lh));
		Cz80_Set_IX(&cz80, le2h16(z80_state.ix));
		Cz80_Set_IY(&cz80, le2h16(z80_state.iy));
		Cz80_Set_SP(&cz80, le2h16(z80_state.sp));
		Cz80_Set_PC(&cz80, le2h16(z80_state.pc));
		Cz80_Set_R(&cz80, z80_state.r);
		Cz80_Set_I(&cz80, z80_state.i);
		Cz80_Set_IFF(&cz80, z80_state.iff);
		Cz80_Set_IM(&cz80, z80_state.im);
		break;
#endif
#ifdef WITH_MZ80
	case Z80_CORE_MZ80:
		z80.z80AF = le2h16(z80_state.alt[0].fa);
		z80.z80BC = le2h16(z80_state.alt[0].cb);
		z80.z80DE = le2h16(z80_state.alt[0].ed);
		z80.z80HL = le2h16(z80_state.alt[0].lh);
		z80.z80afprime = le2h16(z80_state.alt[1].fa);
		z80.z80bcprime = le2h16(z80_state.alt[1].cb);
		z80.z80deprime = le2h16(z80_state.alt[1].ed);
		z80.z80hlprime = le2h16(z80_state.alt[1].lh);
		z80.z80IX = le2h16(z80_state.ix);
		z80.z80IY = le2h16(z80_state.iy);
		z80.z80sp = le2h16(z80_state.sp);
		z80.z80pc = le2h16(z80_state.pc);
		z80.z80r = z80_state.r;
		z80.z80i = z80_state.i;
		z80.z80iff = z80_state.iff;
		z80.z80interruptMode = z80_state.im;
		break;
#endif
	default:
		break;
	}
}

/*
gs0 genecyst save file INfo

GST\0 to start with

80-9f = d0-d7 almost certain
a0-bf = a0-a7 almost certain
c8    = pc    fairly certain
d0    = sr    fairly certain


112 Start of cram len 0x80
192 Start of vsram len 0x50
1e2-474 UNKNOWN sound info?
Start of z80 ram at 474 (they store 2000)
Start of RAM at 2478 almost certain (BYTE SWAPPED)
Start of VRAM at 12478
end of VRAM
*/

/*
  2011-11-05 - Adapted from Gens' genecyst_save_file_format.txt:

  Range        Size   Description
  -----------  -----  -----------
  00000-00002  3      "GST"
  00003-00004  2      "\x40\xe0" (Gens and new DGen format)
  00006-00007  2      "\xe0\x40" (old DGen format)
  00040-00043  4      Last VDP control data written
  00044-00044  1      Second write flag
  00045-00045  1      DMA fill flag
  00048-0004B  4      VDP write address
  00050-00050  1      Version
  00051-00051  1      Emulator ID
  00052-00052  1      System ID
  00060-0006F  16     PSG registers
  00080-000D9  90     M68K registers
  000FA-00111  24     VDP registers
  00112-00191  128    Color RAM
  00192-001E1  80     Vertical scroll RAM
  001E4-003E3  512    YM2612 registers
  00404-00437  52     Z80 registers
  00438-0043F  8      Z80 state
  00474-02473  8192   Z80 RAM
  02478-12477  65536  68K RAM
  12478-22477  65536  Video RAM

  M68K registers (stored little-endian for compatibility)
  --------------
  00080-0009F : D0-D7
  000A0-000BF : A0-A7
  000C8 : PC
  000D0 : SR
  000D2 : USP
  000D6 : SSP

  Z80 registers (little-endian)
  -------------
  00404 : AF
  00408 : BC
  0040C : DE
  00410 : HL
  00414 : IX
  00418 : IY
  0041C : PC
  00420 : SP
  00424 : AF'
  00428 : BC'
  0042C : DE'
  00430 : HL'

  00434 : I
  00435 : R (DGen)
  00436 : x x x x x IFF1 x IFF2 (Gens v5)
  00437 : IM (DGen)

  Z80 State
  ---------
  00438 : Z80 RESET
  00439 : Z80 BUSREQ
  0043A : Unknown
  0043B : Unknown
  0043C : Z80 BANK (DWORD)

  Gens and Kega ADD
  -----------------
  00040 : last VDP Control data written (DWORD)
  00044 : second write flag (1 for second write)
  00045 : DMA Fill flag (1 mean next data write will cause a DMA fill)
  00048 : VDP write address (DWORD)

  00050 : Version       (Genecyst=0; Kega=5; Gens=5; DGen=5)
  00051 : Emulator ID   (Genecyst=0; Kega=0; Gens=1; DGen=9)
  00052 : System ID     (Genesis=0; SegaCD=1; 32X=2; SegaCD32X=3)

  00060-0007F : PSG registers (DWORDs, little endian).
*/

static void *swap16cpy(void *dest, const void *src, size_t n)
{
	size_t i;

	for (i = 0; (i != (n & ~1)); ++i)
		((uint8_t *)dest)[(i ^ 1)] = ((uint8_t *)src)[i];
	((uint8_t *)dest)[i] = ((uint8_t *)src)[i];
	return dest;
}

void md::import_gst(uint8_t* buffer)//, int size)
{
	uint8_t (*buf)[0x22478] =
		(uint8_t (*)[sizeof(*buf)])malloc(sizeof(*buf));
	uint8_t *p;
	uint8_t *q;
	size_t i;
	uint32_t tmp;

	memcpy(*buf, buffer, sizeof(*buf));

	//if ((buf == NULL) ||
	//    (fread((*buf), sizeof(*buf), 1, hand) != 1) ||
	//    /* GST header */
	//    ((memcmp((*buf), "GST\x0\x0\0\xe0\x40", 8) != 0) &&
	//     (memcmp((*buf), "GST\x40\xe0", 5) != 0))) {
	//	//fprintf(stderr,
	//	//	"%s: error: invalid save file header.\n",
	//	//	__func__);
	//	free(buf);
	//	return -1;
	//}

	p = &(*buf)[0x50];
	if (p[2] != 0) {
		//fprintf(stderr,
		//	"%s: error: this is not a Genesis/Mega Drive"
		//	" save file.\n",
		//	__func__);
		free(buf);
		return;// -1;
	}
	if (p[1] != 9) {
		//fprintf(stderr,
		//	"%s: warning: save file was probably not generated by"
		//	" DGen/SDL.\n",
		//	__func__);
	}
	else if (p[0] != 5){}
		//fprintf(stderr,
		//	"%s: warning: unknown save file version.\n",
		//	__func__);
	/* FIXME: VDP stuff */
	/* PSG registers (8x16-bit, 16 bytes) */
	SN76496_restore(0, &(*buf)[0x60]);
	/* M68K registers (19x32-bit, 1x16-bit, 90 bytes (padding: 12)) */
	p = &(*buf)[0x80];
	q = &(*buf)[0xa0];
	for (i = 0; (i != 8); ++i, p += 4, q += 4) {
		memcpy(&m68k_state.d[i], p, 4);
		memcpy(&m68k_state.a[i], q, 4);
	}
	memcpy(&m68k_state.pc, &(*buf)[0xc8], 4);
	memcpy(&m68k_state.sr, &(*buf)[0xd0], 2);
	/*
	  FIXME?
	  memcpy(&m68k_state.usp, &(*buf)[0xd2], 4);
	  memcpy(&m68k_state.ssp, &(*buf)[0xd6], 4);
	*/
	m68k_state_restore();
	/* VDP registers (24x8-bit VDP registers, not sizeof(vdp.reg)) */
	memcpy(vdp.reg, &(*buf)[0xfa], 0x18);
	memset(&vdp.reg[0x18], 0, (sizeof(vdp.reg) - 0x18));
	/* CRAM (64x16-bit registers, 128 bytes), swapped */
	swap16cpy(vdp.cram, &(*buf)[0x112], 0x80);
	/* VSRAM (40x16-bit words, 80 bytes), swapped */
	swap16cpy(vdp.vsram, &(*buf)[0x192], 0x50);
	/* YM2612 registers */
	p = &(*buf)[0x1e4];
	YM2612_restore(0, p);
	fm_reg[0][0x24] = p[0x24];
	fm_reg[0][0x25] = p[0x25];
	fm_reg[0][0x26] = p[0x26];
	fm_reg[0][0x27] = p[0x27];
	memset(fm_ticker, 0, sizeof(fm_ticker));
	/* Z80 registers (12x16-bit and 4x8-bit, 52 bytes (padding: 24)) */
	p = &(*buf)[0x404];
	for (i = 0; (i != 2); ++i, p = &(*buf)[0x424]) {
		memcpy(&z80_state.alt[i].fa, &p[0x0], 2);
		memcpy(&z80_state.alt[i].cb, &p[0x4], 2);
		memcpy(&z80_state.alt[i].ed, &p[0x8], 2);
		memcpy(&z80_state.alt[i].lh, &p[0xc], 2);
	}
	p = &(*buf)[0x414];
	memcpy(&z80_state.ix, &p[0x0], 2);
	memcpy(&z80_state.iy, &p[0x4], 2);
	memcpy(&z80_state.pc, &p[0x8], 2);
	memcpy(&z80_state.sp, &p[0xc], 2);
	p = &(*buf)[0x434];
	z80_state.i = p[0];
	z80_state.r = p[1];
	z80_state.iff = ((p[2] << 1) | p[2]); /* IFF2 = IFF1 */
	z80_state.im = ((p[3] == 0) ? 1 : p[3]);
	z80_state_restore();
	/* Z80 state (8 bytes) */
	p = &(*buf)[0x438];
	z80_st_reset = !p[0]; /* BUS RESET state */
	if (z80_st_reset) {
		z80_reset();
		fm_reset();
	}
	z80_st_busreq = (p[1] & 1); /* BUSREQ state */
	memcpy(&tmp, &(*buf)[0x43c], 4);
	z80_bank68k = le2h32(tmp);
	/* Z80 RAM (8192 bytes) */
	memcpy(z80ram, &(*buf)[0x474], 0x2000);
	/* RAM (65536 bytes), swapped */
	swap16cpy(ram, &(*buf)[0x2478], 0x10000);
	/* VRAM (65536 bytes) */
	memcpy(vdp.vram, &(*buf)[0x12478], 0x10000);
	/* Mark everything as changed */
	memset(vdp.dirt, 0xff, 0x35);
	/* Initialize DAC */
	dac_init();
	free(buf);
//	return 0;
}

void md::export_gst(uint8_t* buffer)
{
	uint8_t (*buf)[0x22478] =
		(uint8_t (*)[sizeof(*buf)])calloc(1, sizeof(*buf));
	uint8_t *p;
	uint8_t *q;
	size_t i;
	uint32_t tmp;

	//if (buf == NULL)
	//	return -1;
	/* GST header */
	memcpy((*buf), "GST\x40\xe0", 5);
	/* FIXME: VDP stuff */
	/* Version */
	(*buf)[0x50] = 5;
	/* Emulator ID */
	(*buf)[0x51] = 9;
	/* System ID */
	(*buf)[0x52] = 0;
	/* PSG registers (8x16-bit, 16 bytes) */
	SN76496_dump(0, &(*buf)[0x60]);
	/* M68K registers (19x32-bit, 1x16-bit, 90 bytes (padding: 12)) */
	m68k_state_dump();
	p = &(*buf)[0x80];
	q = &(*buf)[0xa0];
	for (i = 0; (i != 8); ++i, p += 4, q += 4) {
		memcpy(p, &m68k_state.d[i], 4);
		memcpy(q, &m68k_state.a[i], 4);
	}
	memcpy(&(*buf)[0xc8], &m68k_state.pc, 4);
	memcpy(&(*buf)[0xd0], &m68k_state.sr, 2);
	/*
	  FIXME?
	  memcpy(&(*buf)[0xd2], &m68k_state.usp, 4);
	  memcpy(&(*buf)[0xd6], &m68k_state.ssp, 4);
	*/
	/* VDP registers (24x8-bit VDP registers, not sizeof(vdp.reg)) */
	memcpy(&(*buf)[0xfa], vdp.reg, 0x18);
	/* CRAM (64x16-bit registers, 128 bytes), swapped */
	swap16cpy(&(*buf)[0x112], vdp.cram, 0x80);
	/* VSRAM (40x16-bit words, 80 bytes), swapped */
	swap16cpy(&(*buf)[0x192], vdp.vsram, 0x50);
	/* YM2612 registers */
	p = &(*buf)[0x1e4];
	YM2612_dump(0, p);
	p[0x24] = fm_reg[0][0x24];
	p[0x25] = fm_reg[0][0x25];
	p[0x26] = fm_reg[0][0x26];
	p[0x27] = fm_reg[0][0x27];
	/* Z80 registers (12x16-bit and 4x8-bit, 52 bytes (padding: 24)) */
	z80_state_dump();
	p = &(*buf)[0x404];
	for (i = 0; (i != 2); ++i, p = &(*buf)[0x424]) {
		memcpy(&p[0x0], &z80_state.alt[i].fa, 2);
		memcpy(&p[0x4], &z80_state.alt[i].cb, 2);
		memcpy(&p[0x8], &z80_state.alt[i].ed, 2);
		memcpy(&p[0xc], &z80_state.alt[i].lh, 2);
	}
	p = &(*buf)[0x414];
	memcpy(&p[0x0], &z80_state.ix, 2);
	memcpy(&p[0x4], &z80_state.iy, 2);
	memcpy(&p[0x8], &z80_state.pc, 2);
	memcpy(&p[0xc], &z80_state.sp, 2);
	p = &(*buf)[0x434];
	p[0] = z80_state.i;
	p[1] = z80_state.r;
	p[2] = ((z80_state.iff >> 1) | z80_state.iff);
	p[3] = z80_state.im;
	/* Z80 state (8 bytes) */
	p = &(*buf)[0x438];
	p[0] = !z80_st_reset;
	p[1] = z80_st_busreq;
	tmp = h2le32(z80_bank68k);
	memcpy(&(*buf)[0x43c], &tmp, 4);
	/* Z80 RAM (8192 bytes) */
	memcpy(&(*buf)[0x474], z80ram, 0x2000);
	/* RAM (65536 bytes), swapped */
	swap16cpy(&(*buf)[0x2478], ram, 0x10000);
	/* VRAM (65536 bytes) */
	memcpy(&(*buf)[0x12478], vdp.vram, 0x10000);
	/* Output */
	//i = fwrite((*buf), sizeof(*buf), 1, hand);

	memcpy(buffer, *buf, sizeof(*buf));

	free(buf);
	//if (i != 1)
	//	return -1;
	//return 0;
}
