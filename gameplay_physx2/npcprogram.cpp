
#include "headers.h"
#include "npc.h"

/**
 * class implementation
 */

NPCProgram::NPCProgram(NPC* npc) : _npc(npc), _subProgram(NULL)
{
}

NPCProgram::~NPCProgram()
{
    if( _subProgram ) delete _subProgram;
}

/**
 * base behaviour
 */

void NPCProgram::update(float dt)
{
    // if subprogram is called
    if( _subProgram )
    {
        // act subprogram instead of this program
        _subProgram->update( dt );
        // check end of subprogram
        if( _subProgram->isEndOfProgram() ) 
        {
            // retrieve current subprogram
            NPCProgram* currentSubProgram = _subProgram;
            // call handler
            onEndOfSubProgram( currentSubProgram );
            // if handler doesn't call another subprogram
            if( _subProgram == currentSubProgram ) 
            {
                // delete current subprogram
                delete currentSubProgram;
                // set no subprogram state
                _subProgram = NULL;                
            }
        }
    }
    else
    {
        // update this program
        onUpdate( dt );
    }
}

void NPCProgram::call(NPCProgram* program)
{
    if( _subProgram ) delete _subProgram;
    _subProgram = program;
}