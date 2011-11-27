/**
 * This source code is a part of Metathrone game project. 
 * (c) Perfect Play 2003.
 *
 * @author Sergey Alekhin
 */

#include "headers.h"
#include <windows.h>
#include <tchar.h>
#include "ComponentMgr.h"
#include "CoreImpl.h"
namespace ccor {

bool ComponentMgr::loadComponent(const char * name, Object * param) {

    char buf[_MAX_PATH];
    strcpy(buf, "sys/");
    strcat(buf, name);

    ICore * icore = SingleCore::getInstance();

    // Try to load library
    HINSTANCE hLib = LoadLibrary(buf);
    if (NULL==hLib) {
        icore->logMessage("core: LoadLibrary failed on %s", name);
        return false;
    }
    /*if (NULL==hLib) {
        strcpy(buf, name);
        hLib = LoadLibrary(buf);
        if (NULL==hLib) {
            icore->logMessage("core: LoadLibrary failed on %s", name);
            return false;
        }
    }*/

    // Find the entry point.
    CoreInitComponentProc proc = (CoreInitComponentProc) GetProcAddress(hLib, _T("coreInitComponent"));
    if (!proc) {
        icore->logMessage("core: GetProcAddress(\"coreInitComponent\") failed on %s", name);
        FreeLibrary(hLib);
        return false;
    }

    // Init component
    IComponent * comp = proc(icore);
    if (!comp) {
        icore->logMessage("core: coreInitComponent failed in %s", name);
        FreeLibrary(hLib);
        return false;
    }

    ComponentChunk cc;
    cc.name = name;
    cc.instance = hLib;
    cc.comp = comp;
    cc.typeInfo = comp->getTypeInfo();
    addComponent(cc);
    std::string s = name;
    s += " [";
    s += comp->getComponentLabel();
    icore->logMessage("core: %s] loaded", s.c_str());
    return true;
}


void ComponentMgr::unloadComponent(const ComponentChunk& cc) {

    if (NULL != cc.instance) {
        HINSTANCE hLib = (HINSTANCE) cc.instance;
        SingleCore::getInstance()->logMessage("core: %s unloaded", cc.name.c_str());
        FreeLibrary(hLib);
    }

}



}