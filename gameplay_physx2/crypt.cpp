
#include "headers.h"
#include "crypt.h"
#include "../common/istring.h"

#define CH_0_00 'a'
#define CH_0_01 '0'
#define CH_0_02 's'
#define CH_0_03 'g'
#define CH_0_04 'A'
#define CH_0_05 '6'
#define CH_0_06 'u'
#define CH_0_07 'C'
#define CH_0_08 'd'
#define CH_0_09 '8'
#define CH_0_10 'w'
#define CH_0_11 'j'
#define CH_0_12 '5'
#define CH_0_13 'o'
#define CH_0_14 'G'
#define CH_0_15 'l'

#define CH_1_00 'q'
#define CH_1_01 'I'
#define CH_1_02 '3'
#define CH_1_03 'b'
#define CH_1_04 'K'
#define CH_1_05 'F'
#define CH_1_06 'y'
#define CH_1_07 'x'
#define CH_1_08 'h'
#define CH_1_09 't'
#define CH_1_10 '1'
#define CH_1_11 'e'
#define CH_1_12 'J'
#define CH_1_13 'L'
#define CH_1_14 'p'
#define CH_1_15 'm'

#define CH_2_00 'D'
#define CH_2_01 '9'
#define CH_2_02 'k'
#define CH_2_03 '4'
#define CH_2_04 'z'
#define CH_2_05 '7'
#define CH_2_06 'f'
#define CH_2_07 'B'
#define CH_2_08 'r'
#define CH_2_09 'v'
#define CH_2_10 'i'
#define CH_2_11 'E'
#define CH_2_12 'c'
#define CH_2_13 'H'
#define CH_2_14 '2'
#define CH_2_15 'n'

static inline char scriptBits(unsigned char bits, unsigned int subsetId)
{
    assert( bits>=0 && bits<16 );

    switch( bits )
    {
    case 0:  return subsetId == 0 ? CH_0_00 : ( subsetId == 1 ? CH_1_00 : CH_2_00 );
    case 1:  return subsetId == 0 ? CH_0_01 : ( subsetId == 1 ? CH_1_01 : CH_2_01 );
    case 2:  return subsetId == 0 ? CH_0_02 : ( subsetId == 1 ? CH_1_02 : CH_2_02 );
    case 3:  return subsetId == 0 ? CH_0_03 : ( subsetId == 1 ? CH_1_03 : CH_2_03 );
    case 4:  return subsetId == 0 ? CH_0_04 : ( subsetId == 1 ? CH_1_04 : CH_2_04 );
    case 5:  return subsetId == 0 ? CH_0_05 : ( subsetId == 1 ? CH_1_05 : CH_2_05 );
    case 6:  return subsetId == 0 ? CH_0_06 : ( subsetId == 1 ? CH_1_06 : CH_2_06 );
    case 7:  return subsetId == 0 ? CH_0_07 : ( subsetId == 1 ? CH_1_07 : CH_2_07 );
    case 8:  return subsetId == 0 ? CH_0_08 : ( subsetId == 1 ? CH_1_08 : CH_2_08 );
    case 9:  return subsetId == 0 ? CH_0_09 : ( subsetId == 1 ? CH_1_09 : CH_2_09 );
    case 10: return subsetId == 0 ? CH_0_10 : ( subsetId == 1 ? CH_1_10 : CH_2_10 );
    case 11: return subsetId == 0 ? CH_0_11 : ( subsetId == 1 ? CH_1_11 : CH_2_11 );
    case 12: return subsetId == 0 ? CH_0_12 : ( subsetId == 1 ? CH_1_12 : CH_2_12 );
    case 13: return subsetId == 0 ? CH_0_13 : ( subsetId == 1 ? CH_1_13 : CH_2_13 );
    case 14: return subsetId == 0 ? CH_0_14 : ( subsetId == 1 ? CH_1_14 : CH_2_14 );
    case 15: return subsetId == 0 ? CH_0_15 : ( subsetId == 1 ? CH_1_15 : CH_2_15 );
    default:
        assert( !"shouldn't be here!" );
        return '0';
    }
}

static inline unsigned char descriptBits(char c)
{
    switch( c )
    {
    case CH_0_00: case CH_1_00: case CH_2_00: return 0;
    case CH_0_01: case CH_1_01: case CH_2_01: return 1;
    case CH_0_02: case CH_1_02: case CH_2_02: return 2;
    case CH_0_03: case CH_1_03: case CH_2_03: return 3;
    case CH_0_04: case CH_1_04: case CH_2_04: return 4;
    case CH_0_05: case CH_1_05: case CH_2_05: return 5;
    case CH_0_06: case CH_1_06: case CH_2_06: return 6;
    case CH_0_07: case CH_1_07: case CH_2_07: return 7;
    case CH_0_08: case CH_1_08: case CH_2_08: return 8;
    case CH_0_09: case CH_1_09: case CH_2_09: return 9;
    case CH_0_10: case CH_1_10: case CH_2_10: return 10;
    case CH_0_11: case CH_1_11: case CH_2_11: return 11;
    case CH_0_12: case CH_1_12: case CH_2_12: return 12;
    case CH_0_13: case CH_1_13: case CH_2_13: return 13;
    case CH_0_14: case CH_1_14: case CH_2_14: return 14;
    case CH_0_15: case CH_1_15: case CH_2_15: return 15;
    default:
        assert( !"shouldn't be here!" );
        return 0;
    }
}

void encrypt(std::string& out, void* in, unsigned int insize, const char* key)
{
    // validate key
    unsigned int keyLength = strlen( key ); assert( key );

    // cryptography routine
    unsigned const char* src;
    unsigned const char* cypher;
    unsigned int bufId = 0;
    unsigned int keyId = 0;
    char* incrupted = new char[insize];
    while( bufId < insize )
    {
        src = (unsigned const char*)(in) + bufId;
        cypher = (unsigned const char*)(key) + keyId;
        incrupted[bufId] = *src - *cypher;
        bufId++, keyId++;
        if( keyId >= keyLength ) keyId = 0;
    }

    // scripting routine
    unsigned const char* byte;
    unsigned char  lobits;
    unsigned char  hibits;    
    unsigned char  subsetId = 0;
    out = "";
    for( unsigned int i=0; i<insize; i++ )
    {
        byte = ((unsigned const char*)( incrupted )) + i;
        lobits = ( *byte & 0x0F );
        hibits = ( *byte & 0xF0 ) >> 4;
        out += scriptBits( lobits, subsetId );
        subsetId = lobits % 3;
        out += scriptBits( hibits, subsetId );
        subsetId = hibits % 3;
    }

    // release temporary resources
    delete[] incrupted;
}

bool decrypt(void* out, unsigned int outsize, std::string& in, const char* key)
{
	// sizes not matching, probably compatibility
	if ( in.size() != 2 * outsize ) {
		return false;
	}
    //assert( in.size() == 2 * outsize );

	// descripting routine
    unsigned char byte;
    unsigned char lobits;
    unsigned char hibits;
    unsigned int i=0;
    while( i<in.size() ) 
    {
        lobits = descriptBits( in[i] );
        hibits = descriptBits( in[i+1] );
        byte = ( hibits << 4 ) | lobits ;
        ((char*)out)[i/2] = *( (char*)(&byte) );
        i += 2;
    }

    // cryptography routine
    unsigned int keyLength = strlen( key ); assert( keyLength );
    char* decrupted = new char[outsize];
    unsigned const char* src;
    unsigned const char* cypher;
    unsigned int bufId = 0;
    unsigned int keyId = 0;
    while( bufId < outsize )
    {
        src = (unsigned const char*)(out) + bufId;
        cypher = (unsigned const char*)(key) + keyId;
        decrupted[bufId] = *src + *cypher;
        bufId++, keyId++;
        if( keyId >= keyLength ) keyId = 0;
    }

    // finalize
    memcpy( out, decrupted, outsize );
    delete[] decrupted;

	return true;
}

int checksum(void* in, unsigned int insize)
{
    int result = 0;
    for( unsigned int i=0; i<insize; i++ ) result += ((unsigned char*)(in))[i];
    return result;
}