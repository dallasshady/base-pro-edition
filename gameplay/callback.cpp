
#include "headers.h"
#include "callback.h"

engine::IAtomic* callback::enumerateAtomics(engine::IAtomic* atomic, void* data)
{
    reinterpret_cast<callback::AtomicL*>( data )->push_back( atomic );
    return atomic;
}

engine::IClump* callback::enumerateClumps(engine::IClump* clump, void* data)
{
    reinterpret_cast<callback::ClumpL*>( data )->push_back( clump );
    return clump;
}

engine::IBSP* callback::enumerateBSPs(engine::IBSP* bsp, void* data)
{
    reinterpret_cast<callback::BSPL*>( data )->push_back( bsp );
    return bsp;
}

engine::IBSP* callback::isBSPInsideAsset(engine::IBSP* bsp, void* data)
{
    callback::Insider* insider = reinterpret_cast<callback::Insider*>( data );
    if( reinterpret_cast<engine::IBSP*>( insider->target ) == bsp )
    {
        insider->isInsider = true;
        return NULL;
    }
    return bsp;
}

engine::IClump* callback::isClumpInsideAsset(engine::IClump* clump, void* data)
{
    callback::Insider* insider = reinterpret_cast<callback::Insider*>( data );
    if( reinterpret_cast<engine::IClump*>( insider->target ) == clump )
    {
        insider->isInsider = true;
        return NULL;
    }
    return clump;
}

engine::IBSP* callback::locateBSP(engine::IBSP* bsp, void* data)
{
    callback::Locator* locator = reinterpret_cast<callback::Locator*>( data );    
    if( strcmp( bsp->getName(), locator->targetName ) == 0 )
    {
        locator->target = bsp;
        return NULL;
    }
    return bsp;
}

engine::IClump* callback::locateClump(engine::IClump* clump, void* data)
{
    callback::Locator* locator = reinterpret_cast<callback::Locator*>( data );    
    if( strcmp( clump->getName(), locator->targetName ) == 0 )
    {
        locator->target = clump;
        return NULL;
    }
    return clump;
}

engine::IAtomic* callback::locateAtomic(engine::IAtomic* atomic, void* data)
{
    callback::Locator* locator = reinterpret_cast<callback::Locator*>( data );
    if( strcmp( atomic->getFrame()->getName(), locator->targetName ) == 0 )
    {
        locator->target = atomic;
        return NULL;
    }
    return atomic;
}

engine::ILight* callback::locateLight(engine::ILight* light, void* data)
{
    callback::Locator* locator = reinterpret_cast<callback::Locator*>( data );
    if( strcmp( light->getFrame()->getName(), locator->targetName ) == 0 )
    {
        locator->target = light;
        return NULL;
    }
    return light;
}

engine::IAtomic* callback::setAtomicFlags(engine::IAtomic* atomic, void* data)
{
    atomic->setFlags( *((unsigned int*)(data)) );
    return atomic;
}