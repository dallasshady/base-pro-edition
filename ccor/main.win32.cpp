/**
 * This source code is a part of Metathrone game project. 
 * (c) Perfect Play 2003.
 *
 * @author Sergey Alekhin
 * @bugfixes Alex Petryaev
 */

#include "headers.h"
#include <windows.h>
#include <tchar.h>
#include "../shared/ccor.h"
#include "CoreImpl.h"
using namespace ccor;


static const void parseCommandLine(LPSTR cmdLine) {

    IParamPack * ppack = SingleCore::getInstance()->getCoreParamPack();

    std::string name;
    std::string value;
    const char * c = cmdLine;
    while (*c) {
        for (int i=0; i<2 && *c=='-'; ++c);
        if (*c) {
            if (c[-1]=='-') {
                const char* d = c;
                while (*d && !isspace(*d) && *d!='=') 
                    ++d;
                if (*d=='=') {
                    name.assign(c, d-c);
                    c = ++d;
                    while (*d && !isspace(*d)) ++d;
                    value.assign(c, d-c);
                    ppack->set((std::string("startup.")+name).c_str(), value.c_str());
                }
                else {
                    name.assign(c, d-c);
                    ppack->set((std::string("startup.")+name).c_str(), 1);
                }
                c = d;
            }
            else ++c;
        }
    }
}


int PASCAL WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR cmdLine, int cmdShow)
{
    // --------------------------------------------
    // MEMORY LEAKING? - use allocation breakpoint!
    //_crtBreakAlloc = 66909;

    int rcode = 0;
    entid_t idMainEnt=-1;

    try {

        ICore * c = SingleCore::getInstance();
        static_cast<CoreImpl*>(c)->init();
        IParamPack * ppack = c->getCoreParamPack();

        parseCommandLine(cmdLine);
        ppack->set("startup.arguments.instance", (int) instance);
        ppack->set("startup.arguments.prevInstance", (int) prevInstance);
        ppack->set("startup.arguments.cmdShow", (int) cmdShow);

        // Create main entity and run it
        idMainEnt = c->createEntity("Main",-1,NULL);
        static_cast<CoreImpl*>(c)->act();

        // Never be here...
        assert(false);

    }
    catch(const CoreTerminateException& e) {
        SingleCore::getInstance()->logMessage("core: Exiting with code %d", e.code);
        rcode = e.code;
    }
    catch(const Exception& e) {
        SingleCore::getInstance()->logMessage("Exception! %s", e.getMsg());
        MessageBoxA(NULL,_T(e.getMsg()),_T("exception"),MB_OK | MB_ICONSTOP);
        rcode = 1;
#ifdef _DEBUG
#pragma warning(disable:4297)
        throw;
#pragma warning(default:4297)
#endif
    }
#ifndef _DEBUG
    catch(...) {
        SingleCore::getInstance()->logMessage("Unhandled exception!");
        MessageBoxA(NULL,_T(""),_T("unhandled exception"),MB_OK | MB_ICONSTOP);
        rcode = 1;
    }
#endif

    // Finalizing...

#ifndef _DEBUG
    try {
#endif
        SingleCore::getInstance()->destroyEntity(idMainEnt);
        SingleCore::releaseInstance();
#ifndef _DEBUG
    }
    catch(...) {
        MessageBoxA(NULL,_T(""),_T("unhandled exception while finalizing application"),MB_OK | MB_ICONSTOP);
        rcode = 1;
    }
#endif

    // memory leaks?
    _CrtDumpMemoryLeaks();

    return rcode;

}
