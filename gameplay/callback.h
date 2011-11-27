
#ifndef PUBLIC_CALLBACKS_INCLUDED
#define PUBLIC_CALLBACKS_INCLUDED

#include "headers.h"
#include "../shared/ccor.h"
#include "../shared/engine.h"

namespace callback 
{

/**
 * enumeration containers
 */

typedef std::list<engine::IBSP*> BSPL;
typedef BSPL::iterator BSPI;

typedef std::list<engine::IClump*> ClumpL;
typedef ClumpL::iterator ClumpI;

typedef std::list<engine::IAtomic*> AtomicL;
typedef AtomicL::iterator AtomicI;

/**
 * simple enumeration callbacks
 */

engine::IAtomic* enumerateAtomics(engine::IAtomic* atomic, void* data);
engine::IClump* enumerateClumps(engine::IClump* clump, void* data);
engine::IBSP* enumerateBSPs(engine::IBSP* bsp, void* data);

/**
 * "is inside" state
 */

struct Insider
{
    void* target;
    bool  isInsider;
};

engine::IBSP* isBSPInsideAsset(engine::IBSP* bsp, void* data);
engine::IClump* isClumpInsideAsset(engine::IClump* clump, void* data);

/**
 * locators
 */

engine::IBSP* locateBSP(engine::IBSP* bsp, void* data);
engine::IClump* locateClump(engine::IClump* clump, void* data);
engine::IAtomic* locateAtomic(engine::IAtomic* atomic, void* data);
engine::ILight* locateLight(engine::ILight* light, void* data);

struct Locator
{
public:
    const char* targetName;
    void*       target;
public:    
    inline engine::IClump* locate(engine::IAsset* asset, const char* clumpName)
    {
        target = NULL;
        targetName = clumpName;
        asset->forAllClumps( callback::locateClump, this ); 
        return reinterpret_cast<engine::IClump*>( target );
    }
    inline engine::IClump* locate(engine::IBSP* bsp, const char* clumpName)
    {
        target = NULL;
        targetName = clumpName;
        bsp->forAllClumps( callback::locateClump, this ); 
        return reinterpret_cast<engine::IClump*>( target );
    }
    inline engine::IAtomic* locate(engine::IClump* clump, const char* atomicName)
    {
        target = NULL;
        targetName = atomicName;
        clump->forAllAtomics( callback::locateAtomic, this ); 
        return reinterpret_cast<engine::IAtomic*>( target );
    }
    inline engine::ILight* locateLight(engine::IBSP* bsp, const char* lightName)
    {
        ClumpL clumpL;
        bsp->forAllClumps( enumerateClumps, &clumpL );
        for( ClumpI clumpI = clumpL.begin(); clumpI != clumpL.end(); clumpI++ )
        {
            target = NULL;
            targetName = lightName;
            (*clumpI)->forAllLights( callback::locateLight, this );
            if( target != NULL )
            {
                return reinterpret_cast<engine::ILight*>( target );
            }
        }
        return NULL;
    }
};

/**
 * atomics
 */

engine::IAtomic* setAtomicFlags(engine::IAtomic* atomic, void* data);

}

#endif