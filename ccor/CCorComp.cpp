#include "headers.h"
#include "CCorComp.h"

namespace ccor {

static EntityTypeInfo typeInfo[] = { 
    { NULL, NULL }
};

const char * __stdcall CCorComp::getComponentLabel() {
    return "ComponentCore v" PRODUCT_VERSION_LABEL " " __DATE__ " " __TIME__;
}

const EntityTypeInfo * __stdcall CCorComp::getTypeInfo() {
    return typeInfo;
}

ICore * EntityBase::icore = 0;


}
