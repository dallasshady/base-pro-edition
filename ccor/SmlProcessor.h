#ifndef H10BA4E55_652F_4250_8148_05EC60C16459
#define H10BA4E55_652F_4250_8148_05EC60C16459
#include "../shared/ccor.h"
#include <string>
namespace ccor {

class SmlProcessor {

    const char * _smlText;

    SMLListener * _lis;

    IParamPack * _attribPack;

public:

    SmlProcessor() : _smlText(0), _lis(0), _attribPack(0) { }

    void parse(const char * smlText, SMLListener * lis);

private:

    void parse(const std::string & smlText, std::string& result);

};


}
#endif
