#ifndef __FLASH_H__
#define __FLASH_H__

int loadRom(unsigned char* romBuffer, int bytesAvailable);
uint8* tick(unsigned int inputFlagsFirst, unsigned int inputFlagsSecond);
int beginPaintSound ();
void endPaintSound ();
int saveState(unsigned char* buffer);
void loadState(unsigned char* buffer);

void S9xPostRomInit();
void S9xProcessEvents(unsigned int inputFlagsFirst, unsigned int inputFlagsSecond);

void Convert16To24 (int, int);
void OutOfMemory (void);

#endif