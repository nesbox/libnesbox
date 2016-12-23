// DGen

#ifndef __MD_H__
#define __MD_H__

#define VER VERSION

#include <stdint.h>

#ifdef WITH_STAR
#ifndef __STARCPU_H__
#include "star/starcpu.h"
#endif
#endif

#ifdef WITH_MUSA
#ifndef M68K__HEADER
extern "C"
{
#include "musa/m68k.h"
}
#endif
#endif

#ifdef WITH_MZ80
#ifndef	_MZ80_H_
#include "mz80/mz80.h"
#endif
#endif

#ifdef WITH_CZ80
#include "cz80/cz80.h"
#endif

//#define BUILD_YM2612
extern "C" {
#include "fm.h"
}

#include "sn76496.h"
#include "system.h"

// Debugging macros and support functions. They look like this because C++98
// lacks support for variadic macros. Not my fault.
#ifndef NDEBUG
#include <stdarg.h>
#include <stdio.h>

static inline const char *debug_basename_(const char *name)
{
	const char *tmp;

	for (tmp = name; (*tmp != '\0'); ++tmp)
		if (*tmp == '/')
			name = (tmp + 1);
	return name;
}

static inline void debug_printf__()
{
	fputc('\n', stderr);
}

static inline void (*debug_printf_(const char *s, ...))()
{
	va_list vl;

	va_start(vl, s);
	vfprintf(stderr, s, vl);
	va_end(vl);
	return debug_printf__;
}

static inline void (*(*debug_(const char *file, unsigned int line,
			      const char *func))(const char *, ...))()
{
	fprintf(stderr, "%s:%u: %s: ", debug_basename_(file), line, func);
	return debug_printf_;
}

#define DEBUG(s) (((debug_(__FILE__, __LINE__, __func__))s)(), (void)0)
#else
#define DEBUG(s) (void)0
#endif

extern "C" int test_ctv(unsigned char *dest, int len);
extern "C" int blur_bitmap_16(unsigned char *dest, int len);
extern "C" int blur_bitmap_15(unsigned char *dest, int len);

struct bmap { unsigned char *data; int w,h; int pitch; int bpp; };

// New struct, happily encapsulates all the sound info
struct sndinfo {
	int16_t *lr;
	unsigned int len; /* number of stereo samples */
};

//int get_md_palette(unsigned char pal[256],unsigned char *cram);

class md;
class md_vdp
{
public:
  // Next three lines are the state of the VDP
  // They have to be public so we can save and load states
  // and draw the screen.
  uint8_t mem[(0x10100 + 0x35)]; //0x20+0x10+0x4+1 for dirt
  uint8_t *vram, *cram, *vsram;
  uint8_t reg[0x20];
  int rw_mode,rw_addr,rw_dma;
private:
  int poke_vram (int addr,unsigned char d);
  int poke_cram (int addr,unsigned char d);
  int poke_vsram(int addr,unsigned char d);
  int dma_len();
  int dma_addr();
  unsigned char dma_mem_read(int addr);
  int putword(unsigned short d);
  int putbyte(unsigned char d);
  // Used by draw_scanline to render the different display components
  void draw_tile1(int which, int line, unsigned char *where);
  void draw_tile1_solid(int which, int line, unsigned char *where);
  void draw_tile2(int which, int line, unsigned char *where);
  void draw_tile2_solid(int which, int line, unsigned char *where);
  void draw_tile3(int which, int line, unsigned char *where);
  void draw_tile3_solid(int which, int line, unsigned char *where);
  void draw_tile4(int which, int line, unsigned char *where);
  void draw_tile4_solid(int which, int line, unsigned char *where);
  void draw_window(int line, int front);
  void draw_sprites(int line, int front);
  void draw_plane_back0(int line);
  void draw_plane_back1(int line);
  void draw_plane_front0(int line);
  void draw_plane_front1(int line);
  // Working variables for the above
  unsigned char sprite_order[0x101], *sprite_base;
  int sprite_count;
  unsigned int Bpp;
  unsigned int Bpp_times8;
  unsigned char *dest;
  md& belongs;
  bool command_pending; // set when first half of command arrives
public:
  md_vdp(md&);
  ~md_vdp();
// These are called by MEM.CPP
  int command(uint16_t cmd);
  unsigned short readword();
  unsigned char readbyte();
  int writeword(unsigned short d);
  int writebyte(unsigned char d);

  unsigned char *dirt; // Bitfield: what has changed VRAM/CRAM/VSRAM/Reg
  void reset();

  uint32_t highpal[64];
  // Draw a scanline
  void draw_scanline(struct bmap *bits, int line);
  void set_command_pending(bool f);
  bool get_command_pending();
  void write_reg(uint8_t addr, uint8_t data);
};

/* Generic structures for dumping and restoring M68K and Z80 states. */

typedef struct {
	/* Stored MSB first (big endian) */
	uint32_t d[8]; /* D0-D7 */
	uint32_t a[8]; /* A0-A7 */
	uint32_t pc;
	uint16_t sr;
} m68k_state_t;

typedef struct {
	/* Stored LSB first (little endian) */
	struct {
		uint16_t fa;
		uint16_t cb;
		uint16_t ed;
		uint16_t lh;
	} alt[2]; /* [1] = alternate registers */
	uint16_t ix;
	uint16_t iy;
	uint16_t sp;
	uint16_t pc;
	uint8_t r;
	uint8_t i;
	uint8_t iff; /* x x x x x x IFF2 IFF1 */
	uint8_t im; /* interrupt mode */
} z80_state_t;

#define MCLK_CYCLES_PER_LINE 3416 /* 3420 */
#define M68K_CYCLES_PER_LINE (MCLK_CYCLES_PER_LINE / 7)
#define M68K_CYCLES_HBLANK ((M68K_CYCLES_PER_LINE * 36) / 209)
#define M68K_CYCLES_VDELAY ((M68K_CYCLES_PER_LINE * 36) / 156)
#define Z80_CYCLES_PER_LINE (MCLK_CYCLES_PER_LINE / 15)
#define Z80_CYCLES_HBLANK ((Z80_CYCLES_PER_LINE * 36) / 209)
#define Z80_CYCLES_VDELAY ((Z80_CYCLES_PER_LINE * 36) / 156)
#define NTSC_LINES 262
#define NTSC_VBLANK 224
#define NTSC_HZ 60
#define NTSC_MCLK (MCLK_CYCLES_PER_LINE * NTSC_LINES * NTSC_HZ)
#define PAL_LINES 312
#define PAL_VBLANK 240
#define PAL_HZ 50
#define PAL_MCLK (MCLK_CYCLES_PER_LINE * PAL_LINES * PAL_HZ)

#define MD_UP_MASK     (1)        //  0x00001
#define MD_DOWN_MASK   (1 << 1)   //  0x00002
#define MD_LEFT_MASK   (1 << 2)   //  0x00004
#define MD_RIGHT_MASK  (1 << 3)   //  0x00008
#define MD_B_MASK      (1 << 4)   //  0x00010
#define MD_C_MASK      (1 << 5)   //  0x00020
#define MD_A_MASK      (1 << 12)  //  0x01000
#define MD_START_MASK  (1 << 13)  //  0x02000
#define MD_Z_MASK      (1 << 16)  //  0x10000
#define MD_Y_MASK      (1 << 17)  //  0x20000
#define MD_X_MASK      (1 << 18)  //  0x40000
#define MD_MODE_MASK   (1 << 19)  //  0x80000
#define MD_PAD_UNTOUCHED 0xf303f

class md {
public:
	// Get default NTSC/PAL, Hz, VBLANK, lines number and memory byte
	// for region, which is identified by a single character (J, X, U, E).
	static void region_info(uint8_t region, int* pal, int* hz,
				int* vblank, int* lines, uint8_t* mem)
	{
		// Make region code uppercase and replace space with 0.
		region &= ~0x20;
		switch (region) {
		case 'X':
			// Japan (PAL). This region code isn't found in ROMs
			// but I wanted to label it somehow.
			if (mem)
				*mem = (0x00 | 0x40); // local + PAL
			mem = 0;
		case 'E':
			// Europe (PAL).
			if (pal)
				*pal = 1;
			if (hz)
				*hz = PAL_HZ;
			if (vblank)
				*vblank = PAL_VBLANK;
			if (lines)
				*lines = PAL_LINES;
			if (mem)
				*mem = (0x80 | 0x40); // overseas + PAL
			break;
		case 'J':
		default:
			// Japan (NTSC).
			if (mem)
				*mem = 0x00; // local
			mem = 0;
		case 'U':
			// America (NTSC).
			if (pal)
				*pal = 0;
			if (hz)
				*hz = NTSC_HZ;
			if (vblank)
				*vblank = NTSC_VBLANK;
			if (lines)
				*lines = NTSC_LINES;
			if (mem)
				*mem = 0x80; // overseas
			break;
		}
	}

#ifdef WITH_MUSA
	static class md* md_musa;
	unsigned int md_musa_ref;
	class md* md_musa_prev;

	void md_set_musa(bool set);
#endif
#ifdef WITH_STAR
	static class md* md_star;
	unsigned int md_star_ref;
	class md* md_star_prev;

	void md_set_star(bool set);
#endif
#ifdef WITH_MZ80
	static class md* md_mz80;
	unsigned int md_mz80_ref;
	class md* md_mz80_prev;

	void md_set_mz80(bool set);
#endif
	void md_set(bool set);

	unsigned int mclk; // Master clock
	unsigned int clk0; // MCLK/15 for Z80, SN76489
	unsigned int clk1; // MCLK/7 for M68K, YM2612
	unsigned int lines;
	unsigned int vhz;
	unsigned int pal: 1;

	unsigned int vblank(); // Return first vblank line

private:
	static bool lock; // Prevent other MD objects

	unsigned int ok: 1;
	unsigned int ok_ym2612: 1; // YM2612
	unsigned int ok_sn76496: 1; // SN76496

  unsigned int romlen;
  unsigned char *mem,*rom,*ram,*z80ram;
  // Saveram stuff:
  unsigned char *saveram; // The actual saveram buffer
  unsigned save_start, save_len; // Start address and length
  int save_prot, save_active; // Flags set from $A130F1
public:
  md_vdp vdp;
  m68k_state_t m68k_state;
  z80_state_t z80_state;
  void m68k_state_dump();
  void m68k_state_restore();
  void z80_state_dump();
  void z80_state_restore();
private:
#ifdef WITH_MZ80
  struct mz80context z80;
#endif
#ifdef WITH_CZ80
  cz80_struc cz80;
#endif
#ifdef WITH_STAR
  struct S68000CONTEXT cpu;
  STARSCREAM_PROGRAMREGION *fetch;
  STARSCREAM_DATAREGION    *readbyte,*readword,*writebyte,*writeword;
  int memory_map();
#endif
#ifdef WITH_MUSA
	void *ctx_musa;
#endif

	uint32_t z80_bank68k;
	unsigned int z80_st_busreq: 1; // in BUSREQ state
	unsigned int z80_st_reset: 1; // in RESET state
	unsigned int z80_st_running: 1; // Z80 is running
	unsigned int m68k_st_running: 1; // M68K is running
	struct {
		int m68k;
		int m68k_max;
		int z80;
		int z80_max;
	} odo;

  int ras;

  // Note order is (0) Vblank end -------- Vblank Start -- (HIGH)
  // So int6 happens in the middle of the count

  int aoo3_toggle,aoo5_toggle,aoo3_six,aoo5_six;
  int aoo3_six_timeout, aoo5_six_timeout;
  unsigned char  calculate_coo8();
  unsigned char  calculate_coo9();
  int may_want_to_get_pic(struct bmap *bm,unsigned char retpal[256],int mark);
  int may_want_to_get_sound(struct sndinfo *sndi);

	// Horizontal counter table
	uint8_t hc_table[512][2];

	int m68k_odo(); // M68K odometer
	void m68k_run(); // Run M68K to odo.m68k_max
	void m68k_busreq_request(); // Issue BUSREQ
	void m68k_busreq_cancel(); // Cancel BUSREQ
	void m68k_irq(int i); // Trigger M68K IRQ

	int z80_odo(); // Z80 odometer
	void z80_run(); // Run Z80 to odo.z80_max
	void z80_sync(int fake); // Synchronize Z80 with M68K
	void z80_irq(); // Trigger Z80 IRQ
	void z80_irq_clear(); // Clear Z80 IRQ

	 // Number of microseconds spent in current frame
	unsigned int frame_usecs();

  int fm_timer_callback();
  int myfm_read(int a);
  int mysn_write(int v);
  void fm_reset();
  uint8_t fm_sel[2];
  uint8_t fm_tover;
  int fm_ticker[4];
  signed short fm_reg[2][0x100]; // All of them (-1 = not def'd yet)

  int dac_data[0x138], dac_enabled, dac_last;
  // Since dac values are normally shifted << 6, 1 is used to mark unchanged
  // values.
  void dac_init() { dac_last = 1; dac_enabled = 1; dac_clear(); }
  void dac_clear() {
    dac_data[0] = dac_last;
    for(int i = 1; i < 0x138; ++i)
      dac_data[i] = 1;
  }
  void dac_submit(int d) {
    dac_last = (d - 0x80) << 6; if(dac_enabled) dac_data[ras] = dac_last;
  }
  void dac_enable(int d) {
    dac_enabled = d & 0x80;
    dac_data[ras] = (dac_enabled? dac_last : 1);
  }

  uint8_t m68k_ROM_read(uint32_t a);
  uint8_t m68k_IO_read(uint32_t a);
  uint8_t m68k_VDP_read(uint32_t a);
  void m68k_ROM_write(uint32_t, uint8_t);
  void m68k_IO_write(uint32_t, uint8_t);


public:
  int myfm_write(int a,int v,int md);
  // public struct, full with data from the cartridge header
  struct _carthead_ {
    char system_name[0x10];           // "SEGA GENESIS    ", "SEGA MEGA DRIVE  "
    char copyright[0x10];             // "(C)SEGA 1988.JUL"
    char domestic_name[0x30];         // Domestic game name
    char overseas_name[0x30];         // Overseas game name
    char product_no[0xe];             // "GM XXXXXXXX-XX" or "GM MK-XXXX -00"
    unsigned short checksum;          // ROM checksum
    char control_data[0x10];          // I/O use data (usually only joystick)
    unsigned rom_start, rom_end;      // ROM start & end addresses
    unsigned ram_start, ram_end;      // RAM start & end addresses
    unsigned short save_magic;        // 0x5241("RA") if saveram, else 0x2020
    unsigned short save_flags;        // Misc saveram info
    unsigned save_start, save_end;    // Saveram start & end
    unsigned short modem_magic;       // 0x4d4f("MO") if uses modem, else 0x2020
    char modem_firm[4];               // Modem firm name (should be same as (c))
    char modem_ver[4];                // yy.z, yy=ID number, z=version
    char memo[0x28];                  // Extra data
    char countries[0x10];             // Country code
  } cart_head;
  char region; // Emulator region.
  uint8_t region_guess();
  int one_frame(struct bmap *bm,unsigned char retpal[256],struct sndinfo *sndi);
  void pad_update();
  int pad[2];
  uint8_t pad_com[2];
// c000004 bit 1 write fifo empty, bit 0 write fifo full (???)
// c000005 vint happened, (sprover, coll, oddinint)
// invblank, inhblank, dma busy, pal
  unsigned char coo4,coo5;
  int okay() {return ok;}
  bool plugged;
  md(bool pal, char region);
  ~md();
  void init_pal();
  bool init_sound();
  int plug_in(unsigned char *cart,int len);
  int unplug();
  int load(uint8_t *temp, size_t size);

  int reset();

	uint8_t misc_readbyte(uint32_t a);
	void misc_writebyte(uint32_t a, uint8_t d);
	uint16_t misc_readword(uint32_t a);
	void misc_writeword(uint32_t a, uint16_t d);

	void z80_init();
	void z80_reset();
	uint8_t z80_read(uint16_t a);
	void z80_write(uint16_t a, uint8_t d);
	uint8_t z80_port_read(uint16_t a);
	void z80_port_write(uint16_t a, uint8_t d);

  enum z80_core {
    Z80_CORE_NONE,
#ifdef WITH_MZ80
    Z80_CORE_MZ80,
#endif
#ifdef WITH_CZ80
    Z80_CORE_CZ80,
#endif
    Z80_CORE_TOTAL
  } z80_core;
  void cycle_z80();

  enum cpu_emu {
    CPU_EMU_NONE,
#ifdef WITH_STAR
    CPU_EMU_STAR,
#endif
#ifdef WITH_MUSA
    CPU_EMU_MUSA,
#endif
    CPU_EMU_TOTAL
  } cpu_emu; // OK to read it but call cycle_cpu() to change it
  void cycle_cpu();

#ifdef WITH_MZ80
  mz80context&   z80_context() {return z80;}
#endif
  void import_gst(uint8_t* buffer);
  void export_gst(uint8_t* buffer);

  //char romname[256];

  int z80dump();

  // Fix ROM checksum
  void fix_rom_checksum();

  // List of patches currently applied.
  struct patch_elem {
    struct patch_elem *next;
    uint32_t addr;
    uint16_t data;
  } *patch_elem;
  // Added by Joe Groff:
  // Patch the ROM code, using Game Genie/Hex codes
  int patch(const char *list, unsigned int *errors,
	    unsigned int *applied, unsigned int *reverted);
  // Get/put the battery save RAM
  int has_save_ram();
  int get_save_ram(FILE *from);
  int put_save_ram(FILE *into);

#ifdef WITH_JOYSTICK
  // Added by Phillip K. Hornung <redx@pknet.com>
  void init_joysticks();
  void deinit_joysticks();
#endif

  // Debugger stuff
#ifdef WITH_DEBUGGER
private:
// M68K FLAGS
// user byte
#define	M68K_SR_CARRY		(1<<0)
#define M68K_SR_OVERFLOW	(1<<1)
#define M68K_SR_ZERO		(1<<2)
#define M68K_SR_NEGATIVE	(1<<3)
#define M68K_SR_EXTEND		(1<<4)
// system byte
#define M68K_SR_IP_MASK1	(1<<8)
#define M68K_SR_IP_MASK2	(1<<9)
#define M68K_SR_IP_MASK3	(1<<10)
#define M68K_SR_MI_STATE	(1<<12)
#define M68K_SR_SUP_STATE	(1<<13)
#define M68K_SR_TRACE_EN1	(1<<14)
#define M68K_SR_TRACE_EN2	(1<<15)
// Z80 FLAGS
#define Z80_SR_CARRY		(1<<0)
#define Z80_SR_ADD_SUB		(1<<1)
#define Z80_SR_PARITY_OVERFLOW	(1<<2)
#define Z80_SR_HALF_CARRY	(1<<4)
#define Z80_SR_ZERO		(1<<6)
#define Z80_SR_SIGN		(1<<7)
public:
  struct dgen_debugger_cmd {
	  char		*cmd;
	  int		n_args;
	  int		(md::*handler)(int, char **);
  };
  const static struct md::dgen_debugger_cmd debug_cmd_list[];
  bool debug_trap;
  // commands
  int debug_despatch_cmd(int n_toks, char **args);
  int debug_cmd_cont(int n_args, char **args);
  int debug_cmd_reg(int n_args, char **args);
  int debug_cmd_help(int n_args, char **args);
  int debug_cmd_break(int n_args, char **args);
  int debug_cmd_quit(int n_args, char **args);
  int debug_cmd_step(int n_args, char **args);
  int debug_cmd_minus_break(int n_args, char **args);
  int debug_cmd_cpu(int n_args, char **args);
  int debug_cmd_dis(int n_args, char **args);
  int debug_cmd_mem(int n_args, char **args);
  int debug_cmd_watch(int n_args, char **args);
  int debug_cmd_minus_watch(int n_args, char **args);
  // misc
  void debug_enter(void);
  void debug_leave(void);
  void debug_update_wp_cache(struct dgen_wp *w);
  void debug_update_fired_wps(void);
  void debug_set_wp_m68k(uint32_t start_addr, uint32_t end_addr);
  void debug_print_disassemble(uint32_t from, int len);
  void debug_show_m68k_regs(void);
  void debug_show_z80_regs(void);
  void debug_dump_mem(uint32_t addr, uint32_t len);
#endif
};

inline int md::has_save_ram()
{
  return save_len;
}

#endif // __MD_H__
