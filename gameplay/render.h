
#ifndef RENDERING_ABSTRACTION_INCLUDED
#define RENDERING_ABSTRACTION_INCLUDED

#include "../shared/engine.h"

/**
 * render source incapsulate object's rendering sequence
 */

class RenderSource
{
public:
    virtual Vector4f getClearColor(void) = 0;
    virtual float getBlur(void) = 0;
    virtual float getBrightPass(void) = 0;
    virtual float getBloom(void) = 0;
    virtual unsigned int getNumPasses(void) = 0;
    virtual float getPassNearClip(unsigned int passId) = 0;
    virtual float getPassFarClip(unsigned int passId) = 0;
    virtual unsigned int getPassClearFlag(unsigned int passId) = 0;
    virtual void renderPass(unsigned int passId) = 0;    
    virtual void renderLensFlares(void) = 0;
};

/**
 * render target incapsulate source rendering and post-effects
 */

class RenderTarget
{
public:
    virtual ~RenderTarget() {}
public:
    virtual void render(RenderSource* source, const Matrix4f& view, float fov, bool flares, bool wideScreen) = 0;
};

/**
 * simple render target
 */

class SimpleRT : public RenderTarget
{
public:
    // class implementation
    SimpleRT();
    virtual ~SimpleRT();
    // RenderTarget
    virtual void render(RenderSource* source, const Matrix4f& view, float fov, bool flares, bool wideScreen);
};

/**
 * render target with afterFx
 */

class AfterFxRT : public RenderTarget
{
private:
    engine::ITexture*      _motionBlurRT;
    engine::ICameraEffect* _motionBlurCamera;
    engine::ITexture*      _bloomRT;
    engine::ICameraEffect* _bloomCamera;
public:
    // class implementation
    AfterFxRT();
    virtual ~AfterFxRT();
    // RenderTarget
    virtual void render(RenderSource* source, const Matrix4f& view, float fov, bool flares, bool wideScreen);
};

/**
 * stereovision render target
 */

class StereoRT : public RenderTarget
{
private:
    engine::ITexture*      _stereoRT;
    engine::ICameraEffect* _stereoCamera;
public:
    // class implementation
    StereoRT();
    virtual ~StereoRT();
    // RenderTarget
    virtual void render(RenderSource* source, const Matrix4f& view, float fov, bool flares, bool wideScreen);
};

#endif