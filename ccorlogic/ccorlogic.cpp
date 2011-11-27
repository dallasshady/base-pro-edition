#include "headers.h"
#include "../shared/ccor.h"
#include "../shared/product_version.h"

/*
#include "../shared/console.h"
#include "../shared/engine.h"
*/

using namespace ccor;

class Main : public EntityBase {
public:

    virtual void __stdcall entityDestroy() { delete this; }

    static EntityBase * creator() { return new Main; }

    virtual void __stdcall entityInit(Object * p) {

        IParamPackFactory * pf = icore->getParamPackFactory();
        IParamPack * ppack = pf->load("cfg/ccorlogic.config");

        // Create extension entities
		int numParams = ppack->findParams("loadentity*");
        paramid_t* params = new paramid_t[numParams];
        ppack->copyParamResult( params );
        if (numParams == 0) throw ccor::Exception("ccorlogic : No entities to load");
        for (int i=0; i < numParams; ++i) 
        {
            icore->createEntity(ppack->gets(params[i]),getid(),NULL);
        }
        ppack->release();
        delete[] params;

        cmdExecuteDelay = 10;
        loadStartupCommnds();

    }

    virtual void __stdcall entityHandleEvent(evtid_t id, trigid_t trigId, Object * param) {
    }

    virtual void __stdcall entityAct(float dt) {

        while (--cmdExecuteDelay <= 0) {
            processStartupCommand();
            cmdExecuteDelay = 2;
        }

    }

    void loadStartupCommnds() {
    }

    void processStartupCommand() {
    }

    int cmdExecuteDelay;

    std::stack<std::string> startupCommands;

};


SIMPLE_COMPONENT_BEGIN(CoreLogic)
    DECLARE_COMPONENT_ENTITY(Main)
SIMPLE_COMPONENT_END;
