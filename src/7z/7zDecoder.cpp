#include "core/Precomp.h"

#include "../../include/nesbox.h"

#include <stdio.h>
#include <string.h>

#include "core/7z.h"
#include "core/7zAlloc.h"
#include "core/7zBuf.h"
#include "core/7zCrc.h"
#include "core/7zFile.h"
#include "core/7zVersion.h"

class BufferLookInStream : public ILookInStream
{
public:

	BufferLookInStream(const SevenZBuffer& buffer)
		: m_buffer(buffer)
		, m_position(0)
	{
		Look = BufferLook;
		Skip = BufferSkip;
		Read = BufferRead;
		Seek = BufferSeek;
	}

private:

	const SevenZBuffer& m_buffer;
	Int64 m_position;

	static SRes BufferLook(void *p, const void **buf, size_t *size)
	{
		auto self = static_cast<BufferLookInStream*>(p);

		size_t rem = self->m_buffer.size() - self->m_position;

		if (rem < *size)
			*size = rem;

		*buf = &self->m_buffer[0] + self->m_position;

		return S_OK;
	}

	static SRes BufferSkip(void *p, size_t offset)
	{
		auto self = static_cast<BufferLookInStream*>(p);
		self->m_position += offset;
		return S_OK;
	}

	static SRes BufferRead(void *p, void *buf, size_t *size)
	{
		auto self = static_cast<BufferLookInStream*>(p);

		size_t rem = self->m_buffer.size() - self->m_position;

		if (rem > *size)
			rem = *size;

		memcpy(buf, &self->m_buffer[0] + self->m_position, rem);
		self->m_position += rem;
		*size = rem;

		return S_OK;
	}

	static SRes BufferSeek(void *p, Int64 *pos, ESzSeek origin)
	{
		auto self = static_cast<BufferLookInStream*>(p);

		switch (origin)
		{
		case ESzSeek::SZ_SEEK_SET:
			self->m_position = *pos;
			break;
		case ESzSeek::SZ_SEEK_CUR:
			self->m_position += *pos;
			break;
		case ESzSeek::SZ_SEEK_END:
			self->m_position = self->m_buffer.size() + *pos;
			break;
		}

		*pos = self->m_position;

		return S_OK;
	}
};

class I7zVisitor
{
public:
	virtual bool extract(const std::wstring& name) const = 0;
	virtual void extracted(const SevenZBuffer& buffer) const = 0;
};

static void Extract7z(const SevenZBuffer& buffer, const I7zVisitor& visitor)
{
	BufferLookInStream stream(buffer);

	CSzArEx db;

	ISzAlloc allocImp;
	ISzAlloc allocTempImp;

	SRes res;

	allocImp.Alloc = SzAlloc;
	allocImp.Free = SzFree;

	allocTempImp.Alloc = SzAllocTemp;
	allocTempImp.Free = SzFreeTemp;

	CrcGenerateTable();

	SzArEx_Init(&db);

	if (SzArEx_Open(&db, &stream, &allocImp, &allocTempImp) == SZ_OK)
	{
		UInt32 blockIndex = 0xFFFFFFFF; /* it can have any value before first call (if outBuffer = 0) */
		Byte *outBuffer = 0; /* it must be 0 before first call for each new archive. */
		size_t outBufferSize = 0;  /* it can have any value before first call (if outBuffer = 0) */
		UInt32 i = 0;
		size_t tempSize = 0;
		UInt16 *temp = NULL;

		for (i = 0; i < db.NumFiles; i++)
		{
			size_t offset = 0;
			size_t outSizeProcessed = 0;
			size_t len;

			unsigned isDir = SzArEx_IsDir(&db, i);

			if (isDir)
				continue;

			len = SzArEx_GetFileNameUtf16(&db, i, NULL);

			if (len > tempSize)
			{
				SzFree(NULL, temp);
				tempSize = len;
				temp = (UInt16 *)SzAlloc(NULL, tempSize * sizeof(temp[0]));
			}

			SzArEx_GetFileNameUtf16(&db, i, temp);

			if (!isDir)
			{
				std::wstring name(temp, temp + len - 1);
				if (visitor.extract(name))
				{
					res = SzArEx_Extract(&db, &stream, i,
						&blockIndex, &outBuffer, &outBufferSize,
						&offset, &outSizeProcessed,
						&allocImp, &allocTempImp);
					if (res != SZ_OK)
						break;

					visitor.extracted(SevenZBuffer(outBuffer, outBuffer + outSizeProcessed));
				}
			}

		}

		IAlloc_Free(&allocImp, outBuffer);

		SzArEx_Free(&db, &allocImp);
		SzFree(NULL, temp);
	}
}

SevenZDecoder::SevenZDecoder(const SevenZBuffer& buffer)
	: m_buffer(buffer)
{

}

SevenZList SevenZDecoder::list() const
{
	class Visitor : public I7zVisitor
	{
	public:
		virtual bool extract(const std::wstring& name) const
		{
			list.push_back(name);
			return false;
		}

		virtual void extracted(const SevenZBuffer& buffer) const {}
	
		mutable SevenZList list;
	}visitor;

	Extract7z(m_buffer, visitor);

	return visitor.list;
}

void SevenZDecoder::extract(const std::wstring& name, SevenZBuffer& out)
{
	class Visitor : public I7zVisitor
	{
	public:
		Visitor(const std::wstring& name, SevenZBuffer& out) 
			: m_name(name)
			, m_out(out) 
			, m_found(false)
		{}

		virtual bool extract(const std::wstring& name) const
		{
			if (!m_found)
			{
				if (name == m_name)
				{
					m_found = true;
					return true;
				}
			}

			return false;
		}

		virtual void extracted(const SevenZBuffer& buffer) const 
		{
			m_out.resize(buffer.size());
			std::copy(buffer.begin(), buffer.end(), m_out.begin());
		}

	private:
		
		SevenZBuffer& m_out;
		std::wstring m_name;
		mutable bool m_found;
	}visitor(name, out);

	Extract7z(m_buffer, visitor);
}