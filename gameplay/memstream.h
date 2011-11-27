
#ifndef MEMORY_STREAM_INCLUDED
#define MEMORY_STREAM_INCLUDED

#include "headers.h"
#include "../shared/engine.h"
#include "../common/istring.h"

/**
 * debug renderer
 */

class DebugRenderer : public NxUserDebugRenderer
{
public:
    virtual void renderData(const NxDebugRenderable &data) const;
};

/**
 * output stream
 */

class OutputStream : public NxUserOutputStream
{
public:
    virtual void reportError(NxErrorCode code, const char *message, const char *file, int line)
    {
        OutputDebugString( strformat( "NXERROR %s : %d : %s\n", file, line, message ).c_str() );
    }
    virtual NxAssertResponse reportAssertViolation(const char *message, const char *file, int line)
    {
        OutputDebugString( strformat( "NXASSERT %s : %d : %s\n", file, line, message ).c_str() );
        return NX_AR_BREAKPOINT;
    }
    virtual void print(const char* message)
    {
        OutputDebugString( strformat( "NXMESSAGE %s\n", message ).c_str() );
    }
};

/**
 * memory allocator for NX
 */

class MemoryAllocator : public NxUserAllocator
{
public:
    MemoryAllocator() {}
	virtual ~MemoryAllocator() {}
    void* malloc(NxU32 size) { return ::malloc( size ); }
    void* malloc(NxU32 size, NxMemoryType type) { return ::malloc( size ); }
    void* mallocDEBUG(NxU32 size, const char* file, int line)  { return ::malloc( size ); }
    void* mallocDEBUG(NxU32 size, const char* file, int line, const char* className, NxMemoryType type)  { return ::malloc( size ); }
    void* realloc(void* memory, NxU32 size)  { return ::realloc( memory, size ); }
    void free(void* memory) { ::free( memory ); }
};

/**
 * memory stream for NX
 */

class MemoryWriteBuffer : public NxStream
{
public:
    NxU32 currentSize;
	NxU32 maxSize;
	NxU8* data;
public:
    MemoryWriteBuffer();
	virtual	~MemoryWriteBuffer();
    virtual	NxU8 readByte() const { return 0;} 
	virtual	NxU16 readWord() const { return 0; }
	virtual	NxU32 readDword() const	{ return 0;	}
	virtual	float readFloat() const	{ return 0.0f; }
	virtual	double readDouble() const { return 0.0;	}
	virtual	void readBuffer(void* buffer, NxU32 size) const	{}
	virtual	NxStream& storeByte(NxU8 b);
	virtual	NxStream& storeWord(NxU16 w);
	virtual	NxStream& storeDword(NxU32 d);
	virtual	NxStream& storeFloat(NxReal f);
	virtual	NxStream& storeDouble(NxF64 f);
	virtual	NxStream& storeBuffer(const void* buffer, NxU32 size);
};

class MemoryReadBuffer : public NxStream
{
public:
	mutable		const NxU8*		buffer;
public:
    MemoryReadBuffer(const NxU8* data);
	virtual ~MemoryReadBuffer();
	virtual NxU8 readByte() const;
	virtual NxU16 readWord() const;
	virtual NxU32 readDword() const;
	virtual float readFloat() const;
	virtual double readDouble() const;
	virtual void readBuffer(void* buffer, NxU32 size) const;
	virtual NxStream& storeByte(NxU8 b) { return *this;	}
    virtual	NxStream& storeWord(NxU16 w) { return *this; }
	virtual	NxStream& storeDword(NxU32 d) { return *this; }
    virtual	NxStream& storeFloat(NxReal f) { return *this; }
	virtual	NxStream& storeDouble(NxF64 f) { return *this; }
	virtual	NxStream& storeBuffer(const void* buffer, NxU32 size) { return *this;	}
};

#endif