/**
 * This source code is a part of D3 game project.
 * (c) Digital Dimension Development, 2004-2005
 *
 * @description frame hierarchy routine
 *
 * @author bad3p
 */

#ifndef FRAME_IMPLEMENTATION_INCLUDED
#define FRAME_IMPLEMENTATION_INCLUDED

#include "headers.h"
#include "engine.h"

struct Frame;

/**
 * updatable object
 * this object will update its internal states with "onUpdate" message,
 * received from one of friendly objects - frame or world
 */

class Updatable
{
protected:
    friend struct Frame;
    friend class World;
protected:
    virtual void onUpdate(void) = 0;
};

/**
 * IFrame implementation
 */

struct Frame : public D3DXFRAME,
               virtual public engine::IFrame
{
private:
    struct Chunk
    {
        auid   id;
        char   name[engine::maxNameLength];
        Matrix matrix;
        auid   parent;
    };
public:
    MatrixA16  LTM;
    Frame*     pParentFrame;
    Updatable* pAttachedObject;
private:
    bool                _dirty;
    static unsigned int _numDirtyFrames;
    static Frame**      _dirtyFrames;
public:
    void synchronizeSafe(void);
    void synchronizeFast(void);
public:
    // class implementation
    Frame(const char* frameName);
    virtual ~Frame();
public:
    // IFrame
    virtual void __stdcall release(void);
    virtual const char* __stdcall getName(void);
    virtual Matrix4f __stdcall getMatrix(void);
    virtual Matrix4f __stdcall getLTM(void);
    virtual Vector3f __stdcall getRight(void);
    virtual Vector3f __stdcall getUp(void);
    virtual Vector3f __stdcall getAt(void);
    virtual Vector3f __stdcall getPos(void);
    virtual void __stdcall setPos(const Vector3f& pos);
    virtual void __stdcall setMatrix(const Matrix4f& matrix);
    virtual void __stdcall rotate(const Vector3f& axis, float angle);
    virtual void __stdcall rotateRelative(const Vector3f& axis, float angle);
    virtual void __stdcall translate(const Vector3f& translation);
    virtual void __stdcall setLTM(const Matrix4f& matrix);
    virtual engine::IFrame* __stdcall getParent(void);
    virtual void __stdcall setParent(engine::IFrame* frame);
    virtual void __stdcall forAllChildren(engine::IFrameCallBack callBack, void* data);
public:
    // module locals : inlines
    inline bool isDirtyHierarchy(void)
    { 
        Frame* f = this;
        while( f ) if( f->_dirty ) return true; else f = f->pParentFrame;
        return false;
    }
    /*bool isDirtyHierarchy(void)
    { 
        return _dirty ? true : ( pParentFrame ? pParentFrame->isDirtyHierarchy() : false );
    }*/
    inline bool isDirty(void) { return _dirty; }
    inline Frame* findFrame(const char* frameName) { return static_cast<Frame*>( D3DXFrameFind( this, frameName ) ); }
public:
    // module locals
    Frame* getRoot(void);    
    void dirty(void);
    void write(IResource* resource);
    static AssetObjectT read(IResource* resource, AssetObjectM& assetObjects);
public:
    static void synchronizeAll(void);
    static void init(void);
    static void term(void);
};

#endif
