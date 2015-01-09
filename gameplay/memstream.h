#ifndef MEMORY_STREAM_INCLUDED
#define MEMORY_STREAM_INCLUDED

#include "headers.h"
#include "../shared/engine.h"
#include "../common/istring.h"
#include "foundation/PxErrorCallback.h"
/**
 * debug renderer
 */

//class DebugRenderer : public NxUserDebugRenderer
//{
//public:
//    virtual void renderData(const NxDebugRenderable &data) const;
//};

/**
 * output stream
 */
//PHYSX3
// USING PxDefaultErrorCallback INSTEAD
//class OutputStream : public PxErrorCallback
//{
//public:
//	//PHYSX3
//	/*
//    virtual NxAssertResponse reportAssertViolation(const char *message, const char *file, int line)
//    {
//        OutputDebugString( strformat( "NXASSERT %s : %d : %s\n", file, line, message ).c_str() );
//        return NX_AR_BREAKPOINT;
//    }
//    virtual void print(const char* message)
//    {
//        OutputDebugString( strformat( "NXMESSAGE %s\n", message ).c_str() );
//    }*/
//};

/**
 * memory allocator for NX
 */
//PHYSX3
// Using PxDefaultAllocator instead
//class MemoryAllocator : public PxAllocatorCallback
//{
//public:
//    MemoryAllocator() {}
//	virtual ~MemoryAllocator() {}
//    void* malloc(PxU32 size) { return ::malloc( size ); }
//    void* malloc(PxU32 size, NxMemoryType type) { return ::malloc( size ); }
//    void* mallocDEBUG(PxU32 size, const char* file, int line)  { return ::malloc( size ); }
//    void* mallocDEBUG(PxU32 size, const char* file, int line, const char* className, NxMemoryType type)  { return ::malloc( size ); }
//    void* realloc(void* memory, PxU32 size)  { return ::realloc( memory, size ); }
//    void free(void* memory) { ::free( memory ); }
//
//
//
//	/**
//	\brief Allocates size bytes of memory, which must be 16-byte aligned.
//
//	This method should never return NULL.  If you run out of memory, then
//	you should terminate the app or take some other appropriate action.
//
//	<b>Threading:</b> This function should be thread safe as it can be called in the context of the user thread 
//	and physics processing thread(s).
//
//	\param size			Number of bytes to allocate.
//	\param typeName		Name of the datatype that is being allocated
//	\param filename		The source file which allocated the memory
//	\param line			The source line which allocated the memory
//	\return				The allocated block of memory.
//	*/
//	void* allocate(size_t size, const char* typeName, const char* filename, int line){};
//
//	/**
//	\brief Frees memory previously allocated by allocate().
//
//	<b>Threading:</b> This function should be thread safe as it can be called in the context of the user thread 
//	and physics processing thread(s).
//
//	\param ptr Memory to free.
//	*/
//	void deallocate(void* ptr) {}
//};

/**
 * memory stream for NX
 */
//PHYSX3
/*
class MemoryWriteBuffer : public NxStream
{
public:
    PxU32 currentSize;
	PxU32 maxSize;
	PxU8* data;
public:
    MemoryWriteBuffer();
	virtual	~MemoryWriteBuffer();
    virtual	PxU8 readByte() const { return 0;} 
	virtual	PxU16 readWord() const { return 0; }
	virtual	PxU32 readDword() const	{ return 0;	}
	virtual	float readFloat() const	{ return 0.0f; }
	virtual	double readDouble() const { return 0.0;	}
	virtual	void readBuffer(void* buffer, PxU32 size) const	{}
	virtual	NxStream& storeByte(PxU8 b);
	virtual	NxStream& storeWord(PxU16 w);
	virtual	NxStream& storeDword(PxU32 d);
	virtual	NxStream& storeFloat(PxReal f);
	virtual	NxStream& storeDouble(PxF64 f);
	virtual	NxStream& storeBuffer(const void* buffer, PxU32 size);
};

class MemoryReadBuffer : public NxStream
{
public:
	mutable		const PxU8*		buffer;
public:
    MemoryReadBuffer(const PxU8* data);
	virtual ~MemoryReadBuffer();
	virtual PxU8 readByte() const;
	virtual PxU16 readWord() const;
	virtual PxU32 readDword() const;
	virtual float readFloat() const;
	virtual double readDouble() const;
	virtual void readBuffer(void* buffer, PxU32 size) const;
	virtual NxStream& storeByte(PxU8 b) { return *this;	}
    virtual	NxStream& storeWord(PxU16 w) { return *this; }
	virtual	NxStream& storeDword(PxU32 d) { return *this; }
    virtual	NxStream& storeFloat(PxReal f) { return *this; }
	virtual	NxStream& storeDouble(PxF64 f) { return *this; }
	virtual	NxStream& storeBuffer(const void* buffer, PxU32 size) { return *this;	}
};
*/
#endif