/**
 * This source code is a part of D3 game project.
 * (c) Digital Dimension Development, 2007
 *
 * @description metagame interface
 *
 * @author bad3p
 */

#ifndef METAGAME_INTERFACE_INCLUDED
#define METAGAME_INTERFACE_INCLUDED

#include "../shared/ccor.h"

namespace metagame
{

class INode : public ccor::IBase
{
public:
    DECLARE_INTERFACE_ID(0x0);
public:   
    virtual const char* __stdcall getName(void) = 0;
    virtual const char* __stdcall getClassName(void) = 0;
    virtual void __stdcall release(void) = 0;
};

class IMetagame : public ccor::IBase
{
public:
    DECLARE_INTERFACE_ID(0x0);
public:
    /**
     * node manipulation
     */
    virtual INode* __stdcall getRootNode(void) = 0;
};

}

#endif
