
#ifndef PRELOADED_ACTIVITY_INCLUDED
#define PRELOADED_ACTIVITY_INCLUDED

#include "headers.h"
#include "../shared/ccor.h"
#include "../shared/engine.h"
#include "../shared/gui.h"

#include "activity.h"

using namespace ccor;

/**
 * preloaded resource
 */

typedef void (*AssetPreprocessor)(engine::IAsset* asset);

struct PreloadedAsset
{
public:
    const char*       forcedName;
    engine::AssetType assetType;
    const char*       resourceName;
    AssetPreprocessor preprocessor;
    engine::IAsset*   asset;
public:
    PreloadedAsset(const char* fn, engine::AssetType at, const char* rn, AssetPreprocessor p) :
        forcedName( fn ), assetType( at ), resourceName( rn ), preprocessor( p ), asset( NULL )
    {}
};

/**
 * preloaded activity
 */

class Preloaded : public Activity
{
private:
    gui::IGuiWindow*            _loadingWindow;
    gui::IGuiStaticText*        _loadingMessage;
    std::vector<PreloadedAsset> _preloadedAssets;
private:
    static void progressCallback(const wchar_t* description, float progress, void* userData);
protected:
    virtual ~Preloaded();
public:
    Preloaded();
public:
    virtual void updateActivity(float dt);
    virtual bool endOfActivity(void);
    virtual void onBecomeActive(void);
    virtual void onBecomeInactive(void);
public:
    engine::IClump* findClump(const char* name);
};

#endif