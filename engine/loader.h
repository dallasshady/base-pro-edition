/**
 * This source code is a part of D3 game project.
 * (c) Digital Dimension Development, 2004-2005
 *
 * @description loader implementation
 *
 * @author bad3p
 */

#ifndef LOADER_IMPLEMENTATION_INCLUDED
#define LOADER_IMPLEMENTATION_INCLUDED

#include "headers.h"
#include "../shared/ccor.h"
#include "../shared/engine.h"

class Loader : public engine::ILoader
{
private:
    DWORD           _threadId;
    HANDLE          _threadHandle;
    std::string     _resourcePath;
    IResource*      _resource;
    int             _resourceSize;
    engine::IAsset* _asset;
private:
    static DWORD WINAPI loadBinaryAsset(LPVOID lpParameter);
    static DWORD WINAPI loadXAsset(LPVOID lpParameter);
public:
    // class implementation
    Loader(engine::AssetType assetType, const char* resourcePath);
    virtual ~Loader();
    // ILoader
    virtual void __stdcall release(void);
    virtual float __stdcall getProgress(void);
    virtual engine::IAsset* __stdcall getAsset(void);
    virtual void __stdcall suspend(void);
    virtual void __stdcall resume(void);
};

#endif