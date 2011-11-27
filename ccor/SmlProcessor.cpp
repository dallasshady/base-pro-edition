#include "headers.h"
#include "SmlProcessor.h"
#include "ParamPack.h"
namespace ccor {


void SmlProcessor::parse(const char * smlText, SMLListener * lis) {

    _smlText = smlText;
    _lis = lis;
    _attribPack = getCore()->getParamPackFactory()->createInstance();

    _lis->onSmlBegin();

    std::string result;
    parse(_smlText, result);

    _lis->onSmlEnd(result.c_str());

    _attribPack->release();

}


void SmlProcessor::parse(const std::string & smlText, std::string& result) {

    typedef std::string::size_type pos_t;

    std::string type;
    std::string inner;
    std::string parsedInner;
    std::string closed;

    result = "";

    pos_t pos = 0;
    while( true ) {

        pos_t pos1 = smlText.find('<', pos);
        if (pos1 == smlText.npos) { result.append(&smlText[pos]); break; }
        else result.append(&smlText[pos], pos1-pos);

        pos_t pos2 = smlText.find('>', pos);
        if (pos2 == smlText.npos) { result.append(&smlText[pos]); break; }

        type.assign(smlText, pos1+1, pos2-pos1-1);
        
        closed = "</";
        closed += type;
        closed += ">";
        pos_t pos3 = smlText.find(closed, pos2+1);
        if (pos3 == smlText.npos) inner.assign(&smlText[pos2+1]);
        else {

            int sz = pos3-1-pos2;
            if (sz < 0) break;
            inner.assign(smlText, pos2+1, sz);
        }

        parse(inner, parsedInner);

        static_cast<ParamPack*>(_attribPack)->clear();

        result += _lis->onSmlNode(type.c_str(), _attribPack, parsedInner.c_str());

        if (pos3 == smlText.npos) break;
        pos = pos3 + closed.size();
        
    }

}


}
