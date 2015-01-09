
#ifndef ACTIVITY_ABSTRACTION_INCLUDED
#define ACTIVITY_ABSTRACTION_INCLUDED

/**
 * activity determines methodology of interaction with complex program objects
 *  - gameplay entity has a callstack of activities
 *  - gameplay entity interact only with top-level activity
 *  - in any time new activity can be placed at the top of the callstack
 *  - if activity completes, it should return true with endOfActivity() method
 *  - when no activities in callstack, gameplay terminates
 */

class Activity
{
protected:
    friend class Gameplay;
protected:
    virtual ~Activity() {}
public:
    virtual void updateActivity(float dt) = 0;
    virtual bool endOfActivity(void) = 0;
    virtual void onBecomeActive(void) = 0;
    virtual void onBecomeInactive(void) = 0;
    virtual void onReturnFromActivity(Activity* activity) {}
public:
    void updateCooperativeActivity(float dt);
};

#endif