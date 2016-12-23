  int xsize, ysize;
  int x, scan, w, xstart;
  static int sizes[4] = {32, 64, 64, 128};
  unsigned which;
  unsigned char *where, *scrolls, *tiles, *tile_line;
  int xoff, yoff, xoff_mask;
  int xscroll, yscroll;

#if PLANE == 0
  // Plane 0 is only where the window isn't
  // This should make Herzog Zwei split screen work perfectly, and clean
  // up those little glitches on Sonic 3's level select.
  if(reg[18] & 0x80) // Window goes down, plane 0 goes up! :)
    {
      if((line>>3) >= (reg[18]&0x1f)) return;
    } else { // Window goes up, plane 0 goes down
      if((line>>3) < (reg[18]&0x1f)) return;
    }
#endif

  xsize = sizes[ reg[16]       & 3] << 1;
  ysize = sizes[(reg[16] >> 4) & 3];

#if PLANE == 0
  yscroll = get_word(vsram);
  scrolls = vram + ((reg[13]<<10) & 0xfc00);
  tiles   = vram +  (reg[ 2]<<10);
#else // PLANE == 1
  yscroll = get_word(vsram+2);
  scrolls = vram + ((reg[13]<<10) & 0xfc00) + 2;
  tiles   = vram +  (reg[ 4]<<13);
#endif

  // Interlace?
  if(reg[12] & 2)
    yscroll >>= 1;

  // Offset for the line
  yscroll += line;

  // Wide or narrow?
  if(reg[12] & 1)
    {
      w = 40; xstart = -8;
    } else {
      w = 32; xstart = 24;
    }

  // How do we horizontal offset?
  switch(reg[11] & 3)
    {
    case 2: // per tile
      scrolls += (line & ~7) << 2;
      break;
    case 3: // per line
      scrolls += line << 2;
      break;
    default:
      break;
    }

  xscroll = get_word(scrolls);

  xoff_mask = xsize - 1;
  yoff = (yscroll>>3) & (ysize - 1);
  xoff = ((-(xscroll>>3) - 1)<<1) & xoff_mask;
  tile_line = tiles + xsize * yoff;
  scan = yscroll & 7;

  where = dest + (xstart + (xscroll & 7)) * (int)Bpp;

  for(x = -1; x < w; ++x)
    {
#if PLANE == 0
      if(reg[17] & 0x80) // Don't draw where the window will be
	{
	  if(x >= ((reg[17]&0x1f) << 1)) goto skip;
	} else {
	  // + 1 so scroll layers in Sonic look right
	  if((x + 1) < ((reg[17]&0x1f) << 1)) goto skip;
	}
#endif
      which = get_word(tile_line + xoff);

#if (FRONT == 0) && (PLANE == 1)
      draw_tile_solid(which, scan, where);
#elif FRONT == 1
      if(which >> 15) draw_tile(which, scan, where);
#else
      if(!(which >> 15)) draw_tile(which, scan, where);
#endif

#if PLANE == 0
skip:
#endif
      where += Bpp_times8;
      xoff = (xoff + 2) & xoff_mask;
    }
