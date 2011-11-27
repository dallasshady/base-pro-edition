
#include "headers.h"
#include "bitfield.h"

/**
 * simple 8-bits field
 */

Bitfield8::Bitfield8()
{
    _b00 = _b01 = _b02 = _b03 = _b04 = _b05 = _b06 = _b07 = 0;    
}

unsigned int Bitfield8::getBit(unsigned int bitIndex)
{
    assert( bitIndex < 8 );
    switch( bitIndex )
    {
    case 0  : return _b00;
    case 1  : return _b01;
    case 2  : return _b02;
    case 3  : return _b03;
    case 4  : return _b04;
    case 5  : return _b05;
    case 6  : return _b06;
    case 7  : return _b07;
    default : return 0;
    }
}

void Bitfield8::setBit(unsigned int bitIndex, unsigned int bitValue)
{
    assert( bitIndex < 8 );
    switch( bitIndex )
    {
    case 0  : _b00 = bitValue; break;
    case 1  : _b01 = bitValue; break;
    case 2  : _b02 = bitValue; break;
    case 3  : _b03 = bitValue; break;
    case 4  : _b04 = bitValue; break;
    case 5  : _b05 = bitValue; break;
    case 6  : _b06 = bitValue; break;
    case 7  : _b07 = bitValue; break;
    }
}

/**
 * compound 32-bits field
 */

unsigned int Bitfield32::getBit(unsigned int bitIndex)
{
    assert( bitIndex < 32 );
    if( bitIndex < 32 )
    {
        if( bitIndex < 8 )
        {
            return _b00.getBit( bitIndex );
        }
        else if( bitIndex < 16 )
        {
            return _b01.getBit( bitIndex - 8 );
        }
        else if( bitIndex < 24 )
        {
            return _b02.getBit( bitIndex - 16 );
        }
        else
        {
            return _b03.getBit( bitIndex - 24 );
        }
    }
    else
    {
        return 0;
    }
}

void Bitfield32::setBit(unsigned int bitIndex, unsigned int bitValue)
{
    assert( bitIndex < 32 );
    if( bitIndex < 32 )
    {
        if( bitIndex < 8 )
        {
            _b00.setBit( bitIndex, bitValue );
        }
        else if( bitIndex < 16 )
        {
            _b01.setBit( bitIndex - 8, bitValue );
        }
        else if( bitIndex < 24 )
        {
            _b02.setBit( bitIndex - 16, bitValue );
        }
        else
        {
            _b03.setBit( bitIndex - 24, bitValue );
        }
    }
}