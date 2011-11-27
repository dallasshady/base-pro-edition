#include "headers.h"
#include "VtableDisplace.h"
namespace ccor {

void * VtableDisplace::displaceInterface(void * pThis) {

    return pThis;

#if !defined(_M_IX86)
    return pThis;
#endif

    if (!pThis) return 0;

    // Examine vtable
    long * vt = *(long**)pThis;
    int numVf = 0;
    while (vt[numVf] != 0) ++numVf;

    int numBytes = 17*numVf + 8;
    char * p = new char[numBytes];

    *(long*)p = (long)(p + 4);
    int i;
    for (i=0; i < numVf; ++i) {
        int id = 4*(numVf + 1) + 13*i;
        char * s = (char*) (p + id + 4);
        ((long*)p)[i+1] = (long) s;
        // mov dword ptr [esp+4], pThis
        s[0] = (char) 0xc7; s[1] = (char) 0x44; s[2] = (char) 0x24; 
        s[3] = (char) 0x4; s[4] = (char) pThis; s[5] = (char)(int(pThis)>>8); 
        s[6] = (char)(int(pThis)>>16); s[7] = (char)(int(pThis)>>24); 
        // jmp dword ptr [vt[i] - eip]
        s[8] = (char) 0xe9; *(long*)&s[9] = vt[i] - (long) &s[13];
    }
    ((long*)p)[i+1] = 0;

    _p.push_back(p);

    return p;
    
}

void VtableDisplace::reset() {

    for (unsigned int i=0; i < _p.size(); ++i) {
        delete []_p[i];
        *_p[i] = 0;
    }

}

void VtableDisplace::clear() {

    for (unsigned int i=0; i < _p.size(); ++i) {
        delete []_p[i];
    }

    _p.clear();

}

}
