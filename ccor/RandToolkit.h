/**
 * This source code is a part of Metathrone game project. 
 * (c) Perfect Play 2003.
 *
 * @author Sergey Alekhin
 */

#ifndef HEF000141_799C_41be_8C93_1E7F52B8A5C2
#define HEF000141_799C_41be_8C93_1E7F52B8A5C2
#include "../shared/ccor.h"
namespace ccor {

/**
 * simple implementation of random numbers generator
 *
 * source code from http://www.robertnz.net
 *
 */
class RandToolkit : public virtual IRandToolkit {

    enum { BUFFER_SIZE = 256 };

    long seed;

    long buffer[BUFFER_SIZE];

    long next() {
        long hi = seed / 127773L;                 // integer division
        long lo = seed - hi * 127773L;            // modulo
        seed = 16807 * lo - 2836 * hi;
        if (seed <= 0) seed += LONG_MAX;
        return seed;
    }

public:

    RandToolkit() {
        setSeed(1);
    }

    long nextUniform() {
        int i = next() % BUFFER_SIZE;
        long v = buffer[i];
        buffer[i] = next();
        return v;
    }

    virtual void __stdcall setSeed(long seed) { 
        if (unsigned long(seed) > unsigned long(LONG_MAX)) seed -= LONG_MAX;
        this->seed = seed;
        for (int i = 0; i<BUFFER_SIZE; i++) buffer[i] = next();
    }

    virtual void __stdcall resetSeed()          { setSeed(::time(NULL)); }

    virtual long __stdcall getSeed()            { return (long) seed; }

    virtual bool __stdcall isReshka()           { return nextUniform() & 1; }

    virtual float __stdcall getUniform()        { return static_cast<float>( nextUniform() / (LONG_MAX+1UL) ); }

    virtual float __stdcall getUniform(float a, float b) { return a + nextUniform() * (b-a) / (LONG_MAX+1UL); }

    virtual int __stdcall getUniformInt()       { return nextUniform(); }

    virtual float __stdcall getNorm()           { return 0; }

};


}
#endif
