#ifndef NX_FOUNDATION_NXUSERALLOCATOR
#define NX_FOUNDATION_NXUSERALLOCATOR
/*----------------------------------------------------------------------------*\
|
|						Public Interface to NovodeX Technology
|
|							     www.novodex.com
|
\*----------------------------------------------------------------------------*/

#include "Nx.h"

class NxUserAllocator;


enum NxMemoryType
{
	NX_MEMORY_PERSISTENT,
	NX_MEMORY_TEMP,
};

#ifdef _DEBUG
	#define NX_NEW_TMP(x)		new(__FILE__, __LINE__, #x, NX_MEMORY_TEMP) x
	#define NX_NEW(x)			new(__FILE__, __LINE__, #x, NX_MEMORY_PERSISTENT) x
#else
	#define NX_NEW_TMP(x)		new(NX_MEMORY_TEMP) x
	#define NX_NEW(x)			new(NX_MEMORY_PERSISTENT) x
#endif


/**
 Abstract base class for an application defined memory allocator that can be used by the Nx library.

*/
class NxUserAllocator
	{
	public:
	/**
	Allocates size bytes of memory.

	Same as simple malloc below, but with extra debug info fields.
	*/
	virtual void* mallocDEBUG(size_t size, const char* fileName, int line) = 0;
	virtual void* mallocDEBUG(size_t size, const char* fileName, int line, const char* className, NxMemoryType type)
		{
		return mallocDEBUG(size, fileName, line);	// Just so we don't break user code
		}

	/**
	Allocates size bytes of memory.

	Compatible with the standard C malloc(), with the exception that
	it should never return NULL.  If you run out of memory, then
	you should terminate the app or something.
	*/
	virtual void* malloc(size_t size) = 0;
	virtual void* malloc(size_t size, NxMemoryType type)
		{
		return malloc(size);		// Just so we don't break user code
		}

	/**
	Resizes the memory block previously allocated with malloc() or
	realloc() to be size() bytes, and returns the possibly moved memory.

	Compatible with the standard C realloc(), with the exception that
	it should never return NULL.  If you run out of memory, then
	you should terminate the app or something.
	*/
	virtual void* realloc(void* memory, size_t size) = 0;

	/**
	Frees the memory previously allocated by malloc() or realloc().

	Compatible with the standard C free().
	*/
	virtual void free(void* memory) = 0;

	/**
	Verify heap.
	*/
	virtual void checkDEBUG(void) {};

	};
#endif
