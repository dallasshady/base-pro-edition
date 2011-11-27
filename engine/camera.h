/**
 * This source code is a part of D3 game project.
 * (c) Digital Dimension Development, 2004-2005
 *
 * @description cameras
 *
 * @author bad3p
 */

#ifndef CAMERA_IMPLEMENTATION_INCLUDED
#define CAMERA_IMPLEMENTATION_INCLUDED

#include "headers.h"
#include "engine.h"
#include "texture.h"
#include "frame.h"

/**
 * base class for a quantity of ICamera implementations
 */

class Camera : public engine::ICamera
{
protected:
    Frame*       _frame;
    D3DVIEWPORT9 _viewPort;
    float        _fov;
    float        _nearClipPlane;
    float        _farClipPlane;
    Matrix       _projection;
protected:
    static Camera* _currentCamera;     // actual inside begin/end of scene
    static float   _prevNearClipPlane; // for stencil shadows
    static float   _prevFarClipPlane;  // for stencil shadows
public:    
    static D3DXPLANE    frustrum[6];      // current world space frustrum
    static D3DVIEWPORT9 viewPort;         // current camera viewport
    static Matrix       projectionMatrix; // current projection matrix
    static Matrix       viewMatrix;       // current view matrix
    static Vector       eyePos;           // current eye position
    static Vector       eyeDirection;     // current eye direction
    static float        fov;              // current field of view
    static float        nearClipPlane;
    static float        farClipPlane;    
protected:
    void updateProjection(void);
    void updateFrustrum(void);
public:
    // ICamera
    virtual engine::IFrame* __stdcall getFrame(void);
    virtual void __stdcall setFrame(engine::IFrame* frame);
    virtual float __stdcall getNearClipPlane(void);
    virtual void __stdcall setNearClipPlane(float nearClip);
    virtual float __stdcall getFarClipPlane(void);
    virtual void __stdcall setFarClipPlane(float farClip);
    virtual float __stdcall getFOV(void);
    virtual void __stdcall setFOV(float fov);
    virtual void __stdcall renderTexture(engine::ITexture* texture);
	virtual void __stdcall renderTextureAdditive(engine::ITexture* texture);
    virtual void __stdcall buildPickRay(float x, float y, Vector3f& start, Vector3f& dir);
    virtual Vector3f __stdcall projectPosition(const Vector3f& position);
public:
    inline Frame* frame(void) { return _frame; }
public:
    // stencil shadows support
    static Camera* getCurrentCamera();
    static void beginStencilShadows(float nearClipPlane, float farClipPlane);
    static void endStencilShadows(void);
};

/**
 * simple, direct camera (render-to-swapchain)
 */

class ScreenCamera : public Camera
{
public:
    ScreenCamera(unsigned int width, unsigned int height);
    virtual ~ScreenCamera();
public:
    // ICamera
    virtual void __stdcall release(void);
    virtual void __stdcall beginScene(unsigned int clearMode, const Vector4f& clearColor);
    virtual void __stdcall endScene(void);
};

/**
 * camera with post-effects
 */

class CameraEffect : public Camera,
                     virtual public engine::ICameraEffect,
                     virtual Lostable
{
private:
    friend class Engine;
private:
    Texture*               _newImage;      // render scene into this image
    Texture*               _prevImage;     // previous image
    Texture*               _renderTarget;  // result & output image
    Texture*               _effectTexture; // effect texture argument
    ID3DXRenderToSurface*  _rts;           // render-to-surface helper object
    unsigned int           _quality;       // post-effect quality
    float                  _weight;        // effect weight
    Quartector             _vector;        // 2,3,4-dimensional vector argument
    bool                   _prevIsEmpty;   // previous image is empty
    engine::PostEffectType _currentEffect; // current effect
    bool                   _effectBlocked;    
private:
    static ID3DXEffect*    _pEffect;         // shading techniques
    static StaticLostable* _pEffectLostable; // lostable object
private:
    void applyMotionBlur(void);
    void applyDOF(void);
    void applyBloom(void);
private:
    // support of cooperative work
    static void onStaticLostDevice(void);
    static void onStaticResetDevice(void);
public:
    // class implementation
    CameraEffect(Texture* renderTarget);
    virtual ~CameraEffect();
    // Lostable
    virtual void onLostDevice(void);
    virtual void onResetDevice(void);
    // ICamera
    virtual void __stdcall release(void);
    virtual void __stdcall beginScene(unsigned int clearMode, const Vector4f& clearColor);
    virtual void __stdcall endScene(void);
    // ICameraEffect    
    virtual engine::ICamera* __stdcall getCamera(void);
    virtual engine::PostEffectType __stdcall getPfx(void);
    virtual void __stdcall setPfx(engine::PostEffectType pfxType);
    virtual unsigned int __stdcall getQuality(void);
    virtual void __stdcall setQuality(unsigned int quality);
    virtual float __stdcall getWeight(void);
    virtual void __stdcall setWeight(float value);
    virtual Vector4f __stdcall getVector(void);
    virtual void __stdcall setVector(const Vector4f& value);
    virtual engine::ITexture* __stdcall getTexture(void);
    virtual void __stdcall setTexture(engine::ITexture* texture);
    virtual void __stdcall applyEffect(void);
public:
    // initialization & etc.
    static void init(void);
    static void term(void);    
};

#endif
