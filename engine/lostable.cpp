
#include "headers.h"
#include "engine.h"

Lostable::Lostables Lostable::lostables;

Lostable::Lostable()
{
    lostables.push_back( this );
}

Lostable::~Lostable()
{
    for( LostableI lostableI = lostables.begin(); lostableI != lostables.end(); lostableI++ )
    {
        if( (*lostableI) == this )
        {
            lostables.erase( lostableI );
            break;
        }
    }
}