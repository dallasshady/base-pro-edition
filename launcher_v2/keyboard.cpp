
#include "stdafx.h"
#include "keyboard.h"


static struct Mapping
{
public:
    unsigned int code;
    const char*  hint;
} 
mapping[] =
{
    { 0x02,"1" },
    { 0x03,"2" },
    { 0x04,"3" },
    { 0x05,"4" },
    { 0x06,"5" },
    { 0x07,"6" },
    { 0x08,"7" },
    { 0x09,"8" },
    { 0x0A,"9" },
    { 0x0B,"0" },
    { 0x0C,"-" },
    { 0x0D,"=" },
    { 0x0E,"Back" },
    { 0x0F,"Tab" },
    { 0x10,"Q" },
    { 0x11,"W" },
    { 0x12,"E" },
    { 0x13,"R" },
    { 0x14,"T" },
    { 0x15,"Y" },
    { 0x16,"U" },
    { 0x17,"I" },
    { 0x18,"O" },
    { 0x19,"P" },
    { 0x1A,"[" },
    { 0x1B,"]" },
    { 0x1C,"Return" },
    { 0x1D,"Left Ctrl" },
    { 0x1E,"A" },
    { 0x1F,"S" },
    { 0x20,"D" },
    { 0x21,"F" },
    { 0x22,"G" },
    { 0x23,"H" },
    { 0x24,"J" },
    { 0x25,"K" },
    { 0x26,"L" },
    { 0x27,";" },
    { 0x28,"\"" },
    { 0x29,"|" },
    { 0x2A,"Left shift" },
    { 0x2B,"\\" },
    { 0x2C,"Z" },
    { 0x2D,"X" },
    { 0x2E,"C" },
    { 0x2F,"V" },
    { 0x30,"B" },
    { 0x31,"N" },
    { 0x32,"M" },
    { 0x33,"<" },
    { 0x34,">" },
    { 0x35,"/" },
    { 0x36,"Right shift" },
    { 0x37,"Num *" },
    { 0x38,"Left Alt" },
    { 0x39,"Space" },
    { 0x3B,"F1" },
    { 0x3C,"F2" },
    { 0x3D,"F3" },
    { 0x3E,"F4" },
    { 0x3F,"F5" },
    { 0x40,"F6" },
    { 0x41,"F7" },
    { 0x42,"F8" },
    { 0x43,"F9" },
    { 0x44,"F10" },
    { 0x45,"Num lock" },
    { 0x46,"Scroll lock" },
    { 0x47,"Num 7" },
    { 0x48,"Num 8" },
    { 0x49,"Num 9" },
    { 0x4A,"Num -" },
    { 0x4B,"Num 4" },
    { 0x4C,"Num 5" },
    { 0x4D,"Num 6" },
    { 0x4E,"Num +" },
    { 0x4F,"Num 1" },
    { 0x50,"Num 2" },
    { 0x51,"Num 3" },
    { 0x52,"Num 0" },
    { 0x53,"Num ." },
    { 0x57,"F11" },
    { 0x58,"F12" },
    { 0xB8,"Right alt" },
    { 0xC5,"Pause" },
    { 0xC7,"Home" },
    { 0xC8,"Up" },
    { 0xC9,"Page up" },
    { 0xCB,"Left" },
    { 0xCD,"Right" },
    { 0xCF,"End" },
    { 0xD0,"Down" },
    { 0xD1,"Page down" },
    { 0xD2,"Insert" },
    { 0xD3,"Delete" },
    { 0, NULL }
};

unsigned int getNumCodes(void)
{
    unsigned int result = 0;
    unsigned int i = 0;
    while( mapping[i].code != 0 ) i++, result++;
    return result;
}

unsigned int getCode(unsigned int index)
{
    assert( index < getNumCodes() );
    return mapping[index].code;
}

const char* getCodeHint(unsigned int index)
{
    assert( index < getNumCodes() );
    return mapping[index].hint;
}

unsigned int getCodeIndex(unsigned int code)
{
    unsigned int numCodes = getNumCodes();
    for( unsigned int i=0; i<numCodes; i++ )
    {
        if( mapping[i].code == code ) return i;
    }
    return INVALID_CODE;
}