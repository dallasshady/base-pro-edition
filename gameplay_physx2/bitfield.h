
#ifndef BITFIELD_IMPLEMENTATION_INCLUDED
#define BITFIELD_IMPLEMENTATION_INCLUDED

/**
 * simple 8-bits field
 */

class Bitfield8
{
private:
    unsigned int _b00 : 1;
    unsigned int _b01 : 1;
    unsigned int _b02 : 1;
    unsigned int _b03 : 1;
    unsigned int _b04 : 1;
    unsigned int _b05 : 1;
    unsigned int _b06 : 1;
    unsigned int _b07 : 1;
public:
    Bitfield8();
public:
    unsigned int getBit(unsigned int bitIndex);
    void setBit(unsigned int bitIndex, unsigned int bitValue);
};

/**
 * compound 32-bits field
 */

class Bitfield32
{
private:
    Bitfield8 _b00;
    Bitfield8 _b01;
    Bitfield8 _b02;
    Bitfield8 _b03;
public:
    unsigned int getBit(unsigned int bitIndex);
    void setBit(unsigned int bitIndex, unsigned int bitValue);
};

#endif