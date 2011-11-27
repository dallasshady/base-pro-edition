/**
 * This source code is a part of D3 game project.
 * (c) Digital Dimension Development, 2004-2005
 *
 * @description localization resources
 *
 * @author bad3p
 */

#ifndef LANGUAGE_INTERFACE_INCLUDED
#define LANGUAGE_INTERFACE_INCLUDED

#include "../shared/ccor.h"

namespace language
{

/**
 * Interface for Language entity
 */

class ILanguage : public ccor::IBase
{
public:
    DECLARE_INTERFACE_ID(0xa0000);
public:
    virtual unsigned int __stdcall getNumStrings(void) = 0;
    virtual const wchar_t* __stdcall getUnicodeString(unsigned int stringId) = 0;
    virtual unsigned int __stdcall addUnicodeString(const wchar_t* string) = 0;
    virtual const wchar_t* __stdcall getVersionString(void) = 0;
    virtual void __stdcall reset(void) = 0;
};

}

#endif
