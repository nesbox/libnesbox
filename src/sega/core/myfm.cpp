// DGen v1.29

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "md.h"

// REMEMBER NOT TO USE ANY STATIC variables, because they
// will exist thoughout ALL megadrives!
int md::myfm_write(int a, int v, int md)
{
	int sid = 0;
	int pass = 1;

	(void)md;
	a &= 3;
	sid = ((a & 0x02) >> 1);
	if ((a & 0x01) == 0) {
		fm_sel[sid] = v;
		goto end;
	}
	if (fm_sel[sid] == 0x2a) {
		dac_submit(v);
		pass = 0;
	}
	if (fm_sel[sid] == 0x2b) {
		dac_enable(v);
		pass = 0;
	}
	if (fm_sel[sid] == 0x27) {
		unsigned int now = frame_usecs();

		if ((v & 0x01) && ((fm_reg[0][0x27] & 0x01) == 0)) {
			// load timer A
			fm_ticker[0] = 0;
			fm_ticker[1] = now;
		}
		if ((v & 0x02) && ((fm_reg[0][0x27] & 0x02) == 0)) {
			// load timer B
			fm_ticker[2] = 0;
			fm_ticker[3] = now;
		}
		// (v & 0x04) enable/disable timer A
		// (v & 0x08) enable/disable timer B
		if (v & 0x10) {
			// reset overflow A
			fm_tover &= ~0x01;
			v &= ~0x10;
			fm_reg[0][0x27] &= ~0x10;
		}
		if (v & 0x20) {
			// reset overflow B
			fm_tover &= ~0x02;
			v &= ~0x20;
			fm_reg[0][0x27] &= ~0x20;
		}
	}
	// stash all values
	fm_reg[sid][(fm_sel[sid])] = v;
end:
	if (pass)
		YM2612Write(0, a, v);
	return 0;
}

int md::myfm_read(int a)
{
	fm_timer_callback();
	return (fm_tover | (YM2612Read(0, (a & 3)) & ~0x03));
}

int md::mysn_write(int d)
{
	SN76496Write(0, d);
	return 0;
}

int md::fm_timer_callback()
{
	// periods in microseconds for timers A and B
	int amax = (18 * (1024 -
			  (((fm_reg[0][0x24] << 2) |
			    (fm_reg[0][0x25] & 0x03)) & 0x3ff)));
	int bmax = (288 * (256 - (fm_reg[0][0x26] & 0xff)));
	unsigned int now = frame_usecs();

	if ((fm_reg[0][0x27] & 0x01) && ((now - fm_ticker[1]) > 0)) {
		fm_ticker[0] += (now - fm_ticker[1]);
		fm_ticker[1] = now;
		if (fm_ticker[0] >= amax) {
			if (fm_reg[0][0x27] & 0x04)
				fm_tover |= 0x01;
			fm_ticker[0] -= amax;
		}
	}
	if ((fm_reg[0][0x27] & 0x02) && ((now - fm_ticker[3]) > 0)) {
		fm_ticker[2] += (now - fm_ticker[3]);
		fm_ticker[3] = now;
		if (fm_ticker[2] >= bmax) {
			if (fm_reg[0][0x27] & 0x08)
				fm_tover |= 0x02;
			fm_ticker[2] -= bmax;
		}
	}
	return 0;
}

void md::fm_reset()
{
	memset(fm_sel, 0, sizeof(fm_sel));
	fm_tover = 0x00;
	memset(fm_ticker, 0, sizeof(fm_ticker));
	memset(fm_reg, 0, sizeof(fm_reg));
	YM2612ResetChip(0);
}
