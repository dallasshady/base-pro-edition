
#ifndef MISSION_BROWSER_MODE_INCLUDED
#define MISSION_BROWSER_MODE_INCLUDED

#include "headers.h"
#include "scene.h"
#include "callback.h"
#include "sensor.h"

/**
 * mission browser
 */

#define MBGUI_NUMITEMS 5

class MissionBrowser;

class BrowserSource
{
public:
    virtual ~BrowserSource() {}
public:
    virtual const char* getDefaultThumbnail(void) = 0;
    virtual unsigned int getNumItems(void) = 0;
    virtual const wchar_t* getName(unsigned int itemId) = 0;
    virtual const wchar_t* getBriefing(unsigned int itemId) = 0;
    virtual const char* getThumbnail(unsigned int itemId) = 0;
    virtual float getDuration(unsigned int itemId) = 0;
    virtual bool getWalkthroughFlag(Career* career, unsigned int itemId) = 0;
    virtual bool getWeatherClearance(unsigned int itemId, WeatherType weatherType) = 0;
    virtual bool getWindClearance(unsigned int itemId, Vector3f dir, float ambient, float blast) = 0;
    virtual bool equip(unsigned int itemId, Career* career, float ambient, float blast) = 0;
    virtual bool browseBack(unsigned int itemId) = 0;
    virtual BrowserSource* browse(unsigned int itemId, Scene* scene, MissionBrowser* browser) = 0;
};

class TournamentSource : public BrowserSource
{
private:
    unsigned int              _locationId;
    std::vector<unsigned int> _tournaments;
public:
    // class implementation
    TournamentSource(unsigned int locationId, Career* career);
    virtual ~TournamentSource();
    // BrowserSource
    virtual const char* getDefaultThumbnail(void);
    virtual unsigned int getNumItems(void);
    virtual const wchar_t* getName(unsigned int itemId);
    virtual const wchar_t* getBriefing(unsigned int itemId);
    virtual const char* getThumbnail(unsigned int itemId);
    virtual float getDuration(unsigned int itemId);
    virtual bool getWalkthroughFlag(Career* career, unsigned int itemId);
    virtual bool getWeatherClearance(unsigned int itemId, WeatherType weatherType);
    virtual bool getWindClearance(unsigned int itemId, Vector3f dir, float ambient, float blast);
    virtual bool equip(unsigned int itemId, Career* career, float ambient, float blast);
    virtual bool browseBack(unsigned int itemId);
    virtual BrowserSource* browse(unsigned int itemId, Scene* scene, MissionBrowser* browser);
};

class MissionSource : public BrowserSource
{
private:
    unsigned int _tournamentId;
public:
    // class implementation
    MissionSource(unsigned int tournamentId);
    virtual ~MissionSource();
    // BrowserSource
    virtual const char* getDefaultThumbnail(void);
    virtual unsigned int getNumItems(void);
    virtual const wchar_t* getName(unsigned int itemId);
    virtual const wchar_t* getBriefing(unsigned int itemId);
    virtual const char* getThumbnail(unsigned int itemId);
    virtual float getDuration(unsigned int itemId);
    virtual bool getWalkthroughFlag(Career* career, unsigned int itemId);
    virtual bool getWeatherClearance(unsigned int itemId, WeatherType weatherType);
    virtual bool getWindClearance(unsigned int itemId, Vector3f dir, float ambient, float blast);
    virtual bool equip(unsigned int itemId, Career* career, float ambient, float blast);
    virtual bool browseBack(unsigned int itemId);
    virtual BrowserSource* browse(unsigned int itemId, Scene* scene, MissionBrowser* browser);
};

class MissionBrowser : public Mode
{
private:
    typedef std::stack<BrowserSource*> BrowserSourceStack;
    typedef std::pair<std::string,engine::ITexture*> Thumbnail;
    typedef std::map<std::string,engine::ITexture*> Thumbnails;
    typedef Thumbnails::iterator ThumbnailI;
private:
    /**
     * local camera
     */
    class Camera : public Actor
    {
    private:      
        float    _cameraFOV;
        Matrix4f _cameraMatrix;
    public:
        // actor abstracts
        virtual void onUpdateActivity(float dt);
        virtual Matrix4f getPose(void) { return _cameraMatrix; }
    public:
        // class implementation
        Camera(Scene* scene, Matrix4f pose, float fov);
        virtual ~Camera();
        // class behaviour
        void setMatrix(const Matrix4f& matrix);
    };
private:
    Camera*            _camera;
    gui::IGuiWindow*   _browser;
    Thumbnails         _thumbnails;
    BrowserSourceStack _source;
    int                _topItem;
    int                _selectedItem;
    gui::IGuiWindow*   _footnote;
    float              _footnoteTime;
private:
    // gui messaging
    static void messageCallback(gui::Message* message, void* userData);
private:
    // private behaviour
    void updateGui(void);
    void onClickItem(unsigned int itemId);
    engine::ITexture* getThumbnail(const char* resource);
    void releaseThumbnails(void);    
public:
    // actor abstracts
    virtual void onUpdateActivity(float dt);
    // mode abstracts
    virtual void onSuspend(void);
    virtual void onResume(void);
    virtual bool endOfMode(void);
public:
    // class implementation
    MissionBrowser(Scene* scene, const Matrix4f& cameraPose, float cameraFOV);
    virtual ~MissionBrowser();
public:
    // public behaviour
    void setFootnote(const wchar_t* text, float time);
};

#endif