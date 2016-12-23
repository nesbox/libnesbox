// DGen/SDL v1.29+
// Megadrive 1 Frame module
// Many, many thanks to John Stiles for the new structure of this module! :)
// And kudos to Gens (Gens/GS) and Genplus (GX) authors -- zamaz

#include <stdio.h>
#include <stdlib.h>
//#include <unistd.h>
#include <string.h>
#ifdef HAVE_MEMCPY_H
#include "memcpy.h"
#endif
#include "md.h"
#include "debug.h"

// Set and unset contexts (Musashi, StarScream, MZ80)

#ifdef WITH_MUSA
class md* md::md_musa(0);

void md::md_set_musa(bool set)
{
	if (set) {
		++md_musa_ref;
		if (md_musa == this)
			return;
		m68k_set_context(ctx_musa);
		md_musa_prev = md_musa;
		md_musa = this;
	}
	else {
		if (md_musa != this)
			abort();
		if (--md_musa_ref != 0)
			return;
		m68k_get_context(ctx_musa);
		md_musa = md_musa_prev;
		md_musa_prev = 0;
	}
}
#endif // WITH_MUSA

#ifdef WITH_STAR
class md* md::md_star(0);

void md::md_set_star(bool set)
{
	if (set) {
		++md_star_ref;
		if (md_star == this)
			return;
		s68000SetContext(&cpu);
		md_star_prev = md_star;
		md_star = this;
	}
	else {
		if (md_star != this)
			abort();
		if (--md_star_ref != 0)
			return;
		s68000GetContext(&cpu);
		md_star = md_star_prev;
		md_star_prev = 0;
	}
}
#endif // WITH_STAR

#ifdef WITH_MZ80
class md* md::md_mz80(0);

void md::md_set_mz80(bool set)
{
	if (set) {
		++md_mz80_ref;
		if (md_mz80 == this)
			return;
		mz80SetContext(&z80);
		md_mz80_prev = md_mz80;
		md_mz80 = this;
	}
	else {
		if (md_mz80 != this)
			abort();
		if (--md_mz80_ref != 0)
			return;
		mz80GetContext(&z80);
		md_mz80 = md_mz80_prev;
		md_mz80_prev = 0;
	}
}
#endif // WITH_MZ80

// Set/unset contexts
void md::md_set(bool set)
{
#ifdef WITH_MUSA
	if (cpu_emu == CPU_EMU_MUSA)
		md_set_musa(set);
	else
#endif
#ifdef WITH_STAR
	if (cpu_emu == CPU_EMU_STAR)
		md_set_star(set);
	else
#endif
		(void)0;
#ifdef WITH_MZ80
	if (z80_core == Z80_CORE_MZ80)
		md_set_mz80(set);
#endif
}

// Return current M68K odometer
int md::m68k_odo()
{
	if (m68k_st_running) {
#ifdef WITH_MUSA
		if (cpu_emu == CPU_EMU_MUSA)
			return (odo.m68k + m68k_cycles_run());
#endif
#ifdef WITH_STAR
		if (cpu_emu == CPU_EMU_STAR)
			return (odo.m68k + s68000readOdometer());
#endif
	}
	return odo.m68k;
}

// Run M68K to odo.m68k_max
void md::m68k_run()
{
	int cycles = (odo.m68k_max - odo.m68k);

	if (cycles <= 0)
		return;
	m68k_st_running = 1;
#ifdef WITH_MUSA
	if (cpu_emu == CPU_EMU_MUSA) {
#ifndef WITH_DEBUGGER
		odo.m68k += m68k_execute(cycles);
#else
		if (debug_trap == false)
			odo.m68k += m68k_execute(cycles);
		// check for breakpoint hit
		if (m68k_bp_hit || m68k_wp_hit) {
			debug_enter();
			// reset
			m68k_bp_hit = 0;
			if (m68k_wp_hit)
				debug_update_fired_wps();
			m68k_wp_hit = 0;
		}
#endif
	}
	else
#endif
#ifdef WITH_STAR
	if (cpu_emu == CPU_EMU_STAR) {
		s68000tripOdometer();
		s68000exec(cycles);
		odo.m68k += s68000readOdometer();
	}
	else
#endif
		odo.m68k += cycles;
	m68k_st_running = 0;
}

// Issue BUSREQ
void md::m68k_busreq_request()
{
	if (z80_st_busreq)
		return;
	z80_st_busreq = 1;
	if (z80_st_reset)
		return;
	z80_sync(0);
}

// Cancel BUSREQ
void md::m68k_busreq_cancel()
{
	if (!z80_st_busreq)
		return;
	z80_st_busreq = 0;
	z80_sync(1);
}

// Trigger M68K IRQ
void md::m68k_irq(int i)
{
#ifdef WITH_MUSA
	if (cpu_emu == CPU_EMU_MUSA)
		m68k_set_irq(i);
	else
#endif
#ifdef WITH_STAR
	if (cpu_emu == CPU_EMU_STAR)
		s68000interrupt(i, -1);
	else
#endif
		(void)0;
}

// Return current Z80 odometer
int md::z80_odo()
{
	if (z80_st_running) {
#ifdef WITH_CZ80
		if (z80_core == Z80_CORE_CZ80)
			return (odo.z80 + Cz80_Get_CycleRemaining(&cz80));
#endif
#ifdef WITH_MZ80
		if (z80_core == Z80_CORE_MZ80)
			return (odo.z80 + mz80GetElapsedTicks(0));
#endif
	}
	return odo.z80;
}

// Run Z80 to odo.z80_max
void md::z80_run()
{
	int cycles = (odo.z80_max - odo.z80);

	if (cycles <= 0)
		return;
	if (z80_st_busreq | z80_st_reset)
		odo.z80 += cycles;
	else {
		z80_st_running = 1;
#ifdef WITH_CZ80
		if (z80_core == Z80_CORE_CZ80)
			odo.z80 += Cz80_Exec(&cz80, cycles);
		else
#endif
#ifdef WITH_MZ80
		if (z80_core == Z80_CORE_MZ80) {
			mz80exec(cycles);
			odo.z80 += mz80GetElapsedTicks(1);
		}
		else
#endif
			odo.z80 += cycles;
		z80_st_running = 0;
	}
}

// Synchronize Z80 with M68K, don't execute code if fake is nonzero
void md::z80_sync(int fake)
{
	int cycles = (m68k_odo() >> 1);

	if (cycles > odo.z80_max)
		cycles = odo.z80_max;
	cycles -= odo.z80;
	if (cycles <= 0)
		return;
	if (fake)
		odo.z80 += cycles;
	else {
		z80_st_running = 1;
#ifdef WITH_CZ80
		if (z80_core == Z80_CORE_CZ80)
			odo.z80 += Cz80_Exec(&cz80, cycles);
		else
#endif
#ifdef WITH_MZ80
		if (z80_core == Z80_CORE_MZ80) {
			mz80exec(cycles);
			odo.z80 += mz80GetElapsedTicks(1);
		}
		else
#endif
			odo.z80 += cycles;
		z80_st_running = 0;
	}
}

// Trigger Z80 IRQ
void md::z80_irq()
{
#ifdef WITH_CZ80
	if (z80_core == Z80_CORE_CZ80)
		Cz80_Set_IRQ(&cz80, 0);
	else
#endif
#ifdef WITH_MZ80
	if (z80_core == Z80_CORE_MZ80)
		mz80int(0);
	else
#endif
		(void)0;
}

// Clear Z80 IRQ
void md::z80_irq_clear()
{
#ifdef WITH_CZ80
	if (z80_core == Z80_CORE_CZ80)
		Cz80_Clear_IRQ(&cz80);
	else
#endif
#ifdef WITH_MZ80
	if (z80_core == Z80_CORE_MZ80)
		mz80ClearPendingInterrupt();
	else
#endif
		(void)0;
}

// Return the number of microseconds spent in current frame
unsigned int md::frame_usecs()
{
	if (z80_st_running)
		return ((z80_odo() * 1000) / (clk0 / 1000));
	return ((m68k_odo() * 1000) / (clk1 / 1000));
}

// Return first line of vblank
unsigned int md::vblank()
{
	return (((pal) && (vdp.reg[1] & 0x08)) ? PAL_VBLANK : NTSC_VBLANK);
}

// 6-button pad status update. Should be called once per displayed line.
void md::pad_update()
{
	// The following code was originally in DGen until at least DGen v1.21
	// (Win32 version) but wasn't in DGen/SDL v1.23, preventing 6-button
	// emulation from working at all until now (v1.31 included).
	// This broke some games (no input at all).

	// Reset 6-button pad toggle after 26? lines
	if (aoo3_six_timeout > 25)
		aoo3_six = 0;
	else
		++aoo3_six_timeout;
	if (aoo5_six_timeout > 25)
		aoo5_six = 0;
	else
		++aoo5_six_timeout;
}

// Generate one frame
int md::one_frame(struct bmap *bm, unsigned char retpal[256],
		  struct sndinfo *sndi)
{
	int hints;
	int m68k_max, z80_max;
	int zirq = 0;
	unsigned int vblank = md::vblank();

#ifdef WITH_DEBUGGER
	if (debug_trap)
		return 0;
#endif
	md_set(1);
	// Reset odometers
	memset(&odo, 0, sizeof(odo));
	// Reset FM tickers
	fm_ticker[1] = 0;
	fm_ticker[3] = 0;
	// Raster zero causes special things to happen :)
	coo4 = 0x02; // Init status register (FIXME)
	if (vdp.reg[12] & 0x2)
		coo5 ^= 0x10; // Toggle odd/even for interlace
	if (vdp.reg[1] & 0x40)
		coo5 &= ~0x88; // Clear vblank and vint
	else
		coo5 &= ~0x08; // Clear vblank
	if (!(vdp.reg[1] & 0x20))
		coo5 |= 0x80; // If vint disabled, vint happened
	coo5 |= !!pal;
	// Is permanently set
	hints = vdp.reg[10]; // Set hint counter
	// Video display! :D
	for (ras = 0; ((unsigned int)ras < vblank); ++ras) {
		pad_update(); // Update 6-button pads
		fm_timer_callback(); // Update sound timers
		if ((vdp.reg[0] & 0x10) && (--hints < 0)) {
			// Trigger hint
			m68k_irq(4);
			hints = vdp.reg[10];
			may_want_to_get_pic(bm, retpal, 1);
		}
		else
			may_want_to_get_pic(bm, retpal, 0);
		// Enable h-blank
		coo5 |= 0x04;
		// H-blank comes before, about 36/209 of the whole scanline
		m68k_max = (odo.m68k_max + M68K_CYCLES_PER_LINE);
		odo.m68k_max += M68K_CYCLES_HBLANK;
		z80_max = (odo.z80_max + Z80_CYCLES_PER_LINE);
		odo.z80_max += Z80_CYCLES_HBLANK;
		m68k_run();
		z80_run();
		// Disable h-blank
		coo5 &= ~0x04;
		// Do hdisplay now
		odo.m68k_max = m68k_max;
		odo.z80_max = z80_max;
		m68k_run();
		z80_run();
	}
	// Now we're in vblank, more special things happen :)
	// The following was roughly adapted from Genplus GX
	// Enable v-blank
	coo5 |= 0x08;
	if ((vdp.reg[0] & 0x10) && (--hints < 0)) {
		// Trigger hint
		m68k_irq(4);
		hints = vdp.reg[10];
	}
	// Save m68k_max and z80_max
	m68k_max = (odo.m68k_max + M68K_CYCLES_PER_LINE);
	z80_max = (odo.z80_max + Z80_CYCLES_PER_LINE);
	// Delay between vint and vint flag
	odo.m68k_max += M68K_CYCLES_HBLANK;
	odo.z80_max += Z80_CYCLES_HBLANK;
	// Enable h-blank
	coo5 |= 0x04;
	m68k_run();
	z80_run();
	// Disable h-blank
	coo5 &= ~0x04;
	// Toggle vint flag
	coo5 |= 0x80;
	// Delay between v-blank and vint
	odo.m68k_max += (M68K_CYCLES_VDELAY - M68K_CYCLES_HBLANK);
	odo.z80_max += (Z80_CYCLES_VDELAY - Z80_CYCLES_HBLANK);
	m68k_run();
	z80_run();
	// Restore m68k_max and z80_max
	odo.m68k_max = m68k_max;
	odo.z80_max = z80_max;
	// Blank everything and trigger vint
	if (vdp.reg[1] & 0x20)
		m68k_irq(6);
	if (!z80_st_reset) {
		z80_irq();
		zirq = 1;
	}
	fm_timer_callback();
	// Run remaining cycles
	m68k_run();
	z80_run();
	++ras;
	// Run the course of vblank
	pad_update();
	fm_timer_callback();
	// Usual h-blank stuff
	coo5 |= 0x04;
	m68k_max = (odo.m68k_max + M68K_CYCLES_PER_LINE);
	odo.m68k_max += M68K_CYCLES_HBLANK;
	z80_max = (odo.z80_max + Z80_CYCLES_PER_LINE);
	odo.z80_max += Z80_CYCLES_HBLANK;
	m68k_run();
	z80_run();
	coo5 &= ~0x04;
	odo.m68k_max = m68k_max;
	odo.z80_max = z80_max;
	m68k_run();
	z80_run();
	// Clear Z80 interrupt
	if (zirq)
		z80_irq_clear();
	++ras;
	// Remaining lines
	while ((unsigned int)ras < lines) {
		pad_update();
		fm_timer_callback();
		// Enable h-blank
		coo5 |= 0x04;
		m68k_max = (odo.m68k_max + M68K_CYCLES_PER_LINE);
		odo.m68k_max += M68K_CYCLES_HBLANK;
		z80_max = (odo.z80_max + Z80_CYCLES_PER_LINE);
		odo.z80_max += Z80_CYCLES_HBLANK;
		m68k_run();
		z80_run();
		// Disable h-blank
		coo5 &= ~0x04;
		odo.m68k_max = m68k_max;
		odo.z80_max = z80_max;
		m68k_run();
		z80_run();
		++ras;
	}
	// Fill the sound buffers
	if (sndi)
		may_want_to_get_sound(sndi);
	fm_timer_callback();
	md_set(0);
	return 0;
}

// Return V counter (Gens/GS style)
uint8_t md::calculate_coo8()
{
	unsigned int id;
	unsigned int hc, vc;
	uint8_t bl, bh;

	id = m68k_odo();
	/*
	  FIXME
	  Using "(ras - 1)" instead of "ras" here seems to solve horizon
	  issues in Road Rash and Mickey Mania (Moose Chase level).
	*/
	if (ras)
		id -= ((ras - 1) * M68K_CYCLES_PER_LINE);
	id &= 0x1ff;
	if (vdp.reg[4] & 0x81) {
		hc = hc_table[id][1];
		bl = 0xa4;
	}
	else {
		hc = hc_table[id][0];
		bl = 0x84;
	}
	bh = (hc <= 0xe0);
	bl = (hc >= bl);
	bl &= bh;
	vc = ras;
	vc += (bl != 0);
	if (pal) {
		if (vc >= 0x103)
			vc -= 56;
	}
	else {
		if (vc >= 0xeb)
			vc -= 6;
	}
	return vc;
}

// Return H counter (Gens/GS style)
uint8_t md::calculate_coo9()
{
	unsigned int id;

	id = m68k_odo();
	if (ras)
		id -= ((ras - 1) * M68K_CYCLES_PER_LINE);
	id &= 0x1ff;
	if (vdp.reg[4] & 0x81)
		return hc_table[id][1];
	return hc_table[id][0];
}

// *************************************
//       May want to get pic or sound
// *************************************

inline int md::may_want_to_get_pic(struct bmap *bm,unsigned char retpal[256],int/*mark*/)
{
  if (bm==NULL) return 0;

  if (ras>=0 && (unsigned int)ras<vblank())
    vdp.draw_scanline(bm, ras);
  //if(retpal && ras == 100) get_md_palette(retpal, vdp.cram);
  return 0;
}

int md::may_want_to_get_sound(struct sndinfo *sndi)
{
  extern intptr_t dgen_volume;
  unsigned int i, len = sndi->len;
  int in_dac, cur_dac = 0;
  unsigned int acc_dac = len;
  int *dac = dac_data - 1;

  // Get the PSG
  SN76496Update_16_2(0, sndi->lr, len);

  // We bring in the dac, but stretch it out to fit the real length.
  for (i = 0; (i != len); ++i)
    {
      acc_dac += lines;
      if(acc_dac >= len)
	{
	  acc_dac -= len;
	  in_dac = *(++dac);
	  if(in_dac != 1) cur_dac = in_dac;
	}
      sndi->lr[(i << 1)] += cur_dac;
      sndi->lr[((i << 1) ^ 1)] += cur_dac;
    }

  // Add in the stereo FM buffer
  YM2612UpdateOne(0, sndi->lr, len, dgen_volume);

  // Clear the dac for next frame
  dac_clear();
  return 0;
}

