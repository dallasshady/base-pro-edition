
#ifndef CREDITS_ACTIVITY_INCLUDED
#define CREDITS_ACTIVITY_INCLUDED

#include "headers.h"
#include "../shared/ccor.h"
#include "../shared/engine.h"
#include "../shared/gui.h"

#include "activity.h"

using namespace ccor;

class Credits : public Activity
{
private:
    class Section
    {
    private:
        gui::Rect      _leftRect;
        gui::Rect      _rightRect;
        const wchar_t* _leftText;
        const wchar_t* _rightText;
    public:
        Section(TiXmlElement* element, const Vector3f& screenSize);
        virtual ~Section();
    public:
        void render(void);
    public:
        inline gui::Rect* getLeftRect(void) { return &_leftRect; }
        inline gui::Rect* getRightRect(void) { return &_rightRect; }
    };
private:
    typedef std::vector<Section*> Sections;
private:
    bool     _activityMode;
    Sections _sections;
    float    _scrollTimer;
private:
    void arrangeSections(void);
protected:
    virtual ~Credits();
public:
    Credits();
public:
    virtual void updateActivity(float dt);
    virtual bool endOfActivity(void);
    virtual void onBecomeActive(void);
    virtual void onBecomeInactive(void);
};

#endif