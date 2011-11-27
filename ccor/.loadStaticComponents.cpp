/**
 * This source code is a part of Metathrone game project. 
 * (c) Perfect Play 2003.
 *
 * @author Sergey Alekhin
 */

#include "headers.h"
#include "ComponentMgr.h"
#include "CoreImpl.h"
#include "CCorComp.h"
namespace ccor {

void ComponentMgr::loadStaticComponents() {
    ComponentChunk cc;
    cc.name     = "static$CoreComponent";
    cc.instance = NULL;
    cc.comp     = coreInitCoreComponent(SingleCore::getInstance());;
    cc.typeInfo = cc.comp->getTypeInfo();
    addComponent(cc);
}

}
