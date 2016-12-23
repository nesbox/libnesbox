#ifndef _FCEU_FILE_H_
#define _FCEU_FILE_H_

#include <string>
#include <iostream>
#include "types.h"
#include "utils/memorystream.h"

extern bool bindSavestate;

struct FCEUFILE {
	//the stream you can use to access the data
	std::iostream *stream;

	//the size of the file
	int size;

	FCEUFILE()
		: stream(0)
		//, archiveCount(-1)
	{}
	
	~FCEUFILE()
	{
		if(stream) delete stream;
	}

	enum {
		READ, WRITE, READWRITE
	} mode;

	//guarantees that the file contains a memorystream, and returns it for your convenience
	memorystream* EnsureMemorystream() {
		memorystream* ret = (memorystream*)stream;
		if(ret) return ret;
		
		//nope, we need to create it: copy the contents 
		ret = new memorystream(size);
		stream->read(ret->buf(), size);
		delete stream;
		stream = ret;
		return ret;
	}

	void SetStream(std::iostream *newstream) {
		if(stream) delete stream;
		stream = newstream;
		//get the size of the stream
		stream->seekg(0, std::ios::end);
		size = stream->tellg();
		stream->seekg(0, std::ios::beg);
	}
};

struct FileBaseInfo {
	std::string filebase, filebasedirectory, ext;
	FileBaseInfo() {}
	FileBaseInfo(std::string fbd, std::string fb, std::string ex)
	{
		filebasedirectory = fbd;
		filebase = fb;
		ext = ex;
	}
	
};

FCEUFILE *FCEU_fopen(std::iostream* stream, const char *ipsfn, char *mode, char *ext, int index=-1, const char** extensions = 0);
uint64 FCEU_fread(void *ptr, size_t size, size_t nmemb, FCEUFILE*);
uint64 FCEU_fwrite(void *ptr, size_t size, size_t nmemb, FCEUFILE*);
int FCEU_fseek(FCEUFILE*, long offset, int whence);
uint64 FCEU_ftell(FCEUFILE*);
int FCEU_read32le(uint32 *Bufo, FCEUFILE*);
int FCEU_read16le(uint16 *Bufo, FCEUFILE*);
int FCEU_fgetc(FCEUFILE*);
uint64 FCEU_fgetsize(FCEUFILE*);

void GetFileBase(const char *f);
std::string FCEU_GetPath(int type);
std::string FCEU_MakePath(int type, const char* filebase);
std::string FCEU_MakeFName(int type, int id1, const char *cd1);

#define FCEUMKF_STATE        1
#define FCEUMKF_SAV          3
#define FCEUMKF_PALETTE      6
#define FCEUMKF_STATEGLOB    13
#define FCEUMKF_AUTOSTATE	 15
#define FCEUMKF_MEMW         16
#define FCEUMKF_BBOT         17
#define FCEUMKF_ROMS         18
#define FCEUMKF_INPUT        19
#endif
