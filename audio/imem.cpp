
#include "headers.h"
#include "imem.h"

std::string Chain::context = "";
Chain*      Chain::first = NULL;

/**
 * class implementation
 */

Chain::Chain()
{
    // store context
    _context = context;
    // insert link in front of chain (decrease creation time)
    if( first ) first->_prev = this;
    _prev = NULL, _next = first, first = this;
}

Chain::~Chain()
{
    // remove link from chain
    if( _prev == NULL )
    {
        first = _next;
        if( _next ) _next->_prev = NULL;
    }
    else 
    {
        _prev->_next = _next;
        if( _next ) _next->_prev = _prev;
    }
}