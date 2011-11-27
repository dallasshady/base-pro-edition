/**
 * This source code is a part of D3 game project.
 * (c) Digital Dimension Development, 2004-2005
 *
 * @description memory management & debugging chains
 *
 * @author bad3p
 */

#ifndef IMEM_INCLUDED
#define IMEM_INCLUDED

#include "../common/istring.h"

/**
 * debugging chain
 */

class Chain
{
private:
    std::string _context; // this chain link context information
    Chain*      _prev;    // previous chain link
    Chain*      _next;    // next chain link
public:
    Chain();
    virtual ~Chain();
public:
    inline const char* getContext(void) { return _context.c_str(); }
    inline Chain* getPrevLink(void) { return _prev; }
    inline Chain* getNextLink(void) { return _prev; }
public:
    static std::string context; // current context, new links gets this string
    static Chain*      first;   // first chain link
};

/**
 * debugging chain context macro
 */

#define _chain(V) Chain::context = strformat( "%s : %d", __FILE__, __LINE__ ); V;

#endif
