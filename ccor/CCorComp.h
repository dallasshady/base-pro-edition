#ifndef HE753026F_0C56_43AF_979E_8A0F0FE1ABEE
#define HE753026F_0C56_43AF_979E_8A0F0FE1ABEE
#include "../shared/ccor.h"

namespace ccor {

class CCorComp : public IComponent {
public:
    CCorComp(ICore * core) { this->core = core; }

    virtual const char * __stdcall getComponentLabel();

    virtual const EntityTypeInfo * __stdcall getTypeInfo();

    virtual void __stdcall release() { delete this; }

private:
    ICore * core;
};


inline IComponent * coreInitCoreComponent(ICore * core) {
    return new CCorComp(core);
}

}
#endif
