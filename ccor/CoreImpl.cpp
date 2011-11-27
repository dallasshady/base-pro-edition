/**
 * This source code is a part of Metathrone game project. 
 * (c) Perfect Play 2003.
 *
 * @author Sergey Alekhin
 */

#include "headers.h"
#include "CoreImpl.h"
#include "ParamPack.h"
#include "../shared/product_version.h"
#include "SmlProcessor.h"
namespace ccor {


SystemTime * SystemTime::instance = 0;

TimeMgr * TimeMgr::instance = 0;

static const char * g_comLabel = "Game core v" PRODUCT_VERSION_LABEL " " __DATE__ " " __TIME__;

static bool deletingCore = false;

ICore * SingleCore::getInstance() {
    if (deletingCore) return NULL;
    if (NULL==instance) {
        instance = new CoreImpl();
    }
    return instance;
}

void SingleCore::releaseInstance() {
    SingleComponentMgr::releaseInstance();
    if (!deletingCore && NULL!=instance) {
        instance->release();
        deletingCore = true;
        delete instance;
        instance = NULL;
        deletingCore = false;
    }
}

CoreImpl * SingleCore::instance = 0;


CoreImpl::CoreImpl() {
    flog = NULL;
    coreParamPack = NULL;
    paramPackFactory = new ParamPackFactory();
    EntityBase::icore = this;
    htmlLog = false;
    xdata = NULL;
}


void CoreImpl::init() {
    coreParamPack = paramPackFactory->load("cfg/ccor.config");
    startLog();
    // loading ids database
    if (idset.load("sys/iddb.dat")) {
        // Create triggers listed in database
        std::vector<trigid_t> ids;
        idset.enumTriggers(ids);
        for (unsigned int i=0; i < ids.size(); ++i) {
            tm.createTrigger(ids[i],trigImmediate | trigValueable,NULL);
        }
    }
    else logMessage("ccor : warning : can't open id database!");
    resMgr.loadPathMap("cfg/resmgr.config");
    // reset random seed
    paramid_t pidRandomize = coreParamPack->find("randomize");
    if (pidRandomize >= 0) {
        if (strcmp(coreParamPack->getv(pidRandomize,""), "timer")==0) 
            randToolkit.resetSeed();
        else {
            int seed;
            coreParamPack->get(pidRandomize,seed);
            randToolkit.setSeed(seed);
        }
    }
}


void CoreImpl::release() {

    tm.destroyAll();
    em.destroyAll();

    if (coreParamPack) {
        coreParamPack->release();
        coreParamPack = NULL;
    }
    if (paramPackFactory) {
        delete (ParamPackFactory*)paramPackFactory;
        paramPackFactory = NULL;
    }

    if (NULL!=flog) {
        if (htmlLog) ::fprintf(flog, "</pre></body></html>\n");
        ::fclose(flog);
        flog = NULL;
    }
}


CoreImpl::~CoreImpl() {
    release();
}


void CoreImpl::act() {
    logMessage("core: Performing act() cycle");
    while (true) {
        em.actEntities();
        
        processSystemMessages();

        tm.checkTriggers(&em);
    }
}


void CoreImpl::logMessageV(const char * fmt, va_list vl) {
    if (flog) {
        if (htmlLog) {
            char buf[8192];
            ::vsprintf(buf,fmt,vl);
            if (::strstr(buf, "rror")!=0) {
                ::fprintf(flog,"<div class=error>%s</div>", buf);
            }
            else if (::strstr(buf, "arning")!=0) {
                ::fprintf(flog,"<div class=warning>%s</div>", buf);
            }
            else if (::strstr(buf, "xception")!=0) {
                ::fprintf(flog,"<div class=exception>%s</div>", buf);
            }
            else if (::strstr(buf, "*** >")!=0) {
                ::fprintf(flog,"<div class=console>%s</div>", buf);
            }
            else if (::strstr(buf, "***")!=0) {
                ::fprintf(flog,"<div class=asterisks>%s</div>", buf);
            }
            else if (::strstr(buf, "core:")!=0) {
                ::fprintf(flog,"<div class=core>%s</div>", buf);
            }
            else ::fprintf(flog,"<div class=text>%s</div>", buf);
        }
        else {
            // put message to log
            ::vfprintf(flog,fmt,vl);
        }
        ::fputc('\n',flog);
        ::fflush(flog);
    }    
}


void CoreImpl::startLog() {
    // Determine log type
    paramid_t idLogType=coreParamPack->find("log.type");
    if (idLogType>=0) {
        const char * logType=coreParamPack->gets(idLogType);
        if (stricmp(logType,"html")==0) htmlLog=true;
    }
    if (htmlLog) {
        flog = ::fopen( htmlLog?"game_log.html":"game.log", "wt" );
        if (flog) ::fprintf(flog, "<html><head><link href='log.css' "
            "rel='stylesheet' type='text/css'></head><body><pre>");
    }
    else flog = ::fopen("game.log", "wt");
    logMessage(g_comLabel,"");
}


void CoreImpl::processSML(const char * smlText, SMLListener * lis) {

    SmlProcessor().parse(smlText, lis);

}


}
