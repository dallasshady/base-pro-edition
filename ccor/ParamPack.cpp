#include <sys/types.h>
#include <sys/stat.h>
#include "headers.h"
#include "../shared/ccor.h"
#include "../shared/product_version.h"
#include "../shared/vector.h"
#include "CoreImpl.h"
#include "ParamPack.h"

namespace ccor {

ParamPack::ParamPack() {
    resName = "";
    changed = false;
    tmpbuf[0] = '\0';
    //add("$.tmp$");
    params.push_back(new ObjParam("$.tmp$"));
}

void ParamPack::CheckID(paramid_t id) {
    if(id < 0 && id >= int(params.size()))
        throw Exception("ccor : cannot find parameter \"%s\"", tmpbuf);
}

void ParamPack::clear() {

    for(ListenerVector::iterator lvit = listeners.begin(); lvit != listeners.end(); ++lvit)
        (*lvit)->onParamPackDestroy(this);
    layout.clear();
    listeners.clear();

    ids.clear();

    for(ParamVector::iterator pvit = params.begin(); pvit != params.end(); ++pvit)
        delete *pvit;
    params.clear();
}

void ParamPack::ChangeParam(paramid_t id) {
    ListenerVector::iterator it;
    for(it = listeners.begin(); it != listeners.end(); ++it)
        if(!(*it)->onParamPreChange(this, id, 0))
            return;

    ParamType tmptype = params[0]->type;
    ParamValue tmpvalue = params[0]->value;
    params[0]->type = params[id]->type;
    params[0]->value = params[id]->value;
    params[id]->type = tmptype;
    params[id]->value = tmpvalue;

    for(it = listeners.begin(); it != listeners.end(); ++it)
        (*it)->onParamChanged(this, id, 0);
}

paramid_t ParamPack::add(const char * qualifiedName) {

    paramid_t id = find(qualifiedName);
    if(id != -1) return id;

    int nameLength = ::strlen(qualifiedName);
    if(!nameLength)
        throw Exception("ccor : empty qualified name of new parameter");
    if(qualifiedName[nameLength - 1] == '@') {
        char nameEx[BUF_SIZE];
        ::strncpy(nameEx, qualifiedName, nameLength - 1);
        int i = 0;
        do ::sprintf(nameEx + nameLength - 1, "%04d", ++i);
        while(ids.find(nameEx) != ids.end());
        if(i >= 10000) throw Exception("ccor : too many parameters \"%s\"", qualifiedName);
        params.push_back(new ObjParam(nameEx));
        id = params.size() - 1;
        ids[params.back()->name.c_str()] = id;
    }
    else {

        const char * leftBracket = ::strrchr(qualifiedName, '[');
        if(leftBracket) {

            int namelen = leftBracket - qualifiedName;
            char name[BUF_SIZE] = "";
            ::strncpy(name, qualifiedName, namelen);

            int i = ::atoi(leftBracket + 1);
            int imax = getMaxIndex(name);
            while(++imax <= i) {

                ::sprintf(name + namelen, "[%04d]", imax);
                params.push_back(new ObjParam(name));
                id = params.size() - 1;
                ids[params.back()->name.c_str()] = id;
            }
        }
        else {

            params.push_back(new ObjParam(qualifiedName));
            id = params.size() - 1;
            ids[params.back()->name.c_str()] = id;
        }
    }

    return id;
}

int ParamPack::getMaxIndex(paramid_t id) {

    CheckID(id);

    std::string param = params[id]->name.c_str();
    ParamMap::iterator it = ids.upper_bound((param + "[\xFF").c_str());
    paramid_t upperId = (--it)->second;
    if(params[upperId]->name.length() > param.length())
        if(params[upperId]->name[param.length()] == '[')
            return ::atoi(params[upperId]->name.c_str() + param.length() + 1);

    return 0;
}

void ParamPack::set(paramid_t id, int value) {
    CheckID(id);

    ObjParam * tmpparam = params[0];
    tmpparam->release();
    tmpparam->type = PT_INT;
    tmpparam->value.iValue = value;

    if(id) {
        ChangeParam(id);
        changed = true;
    }
}

void ParamPack::set(paramid_t id, float value) {
    CheckID(id);

    ObjParam * tmpparam = params[0];
    tmpparam->release();
    tmpparam->type = PT_FLOAT;
    tmpparam->value.fValue = value;

    if(id) {
        ChangeParam(id);
        changed = true;
    }
}

void ParamPack::set(paramid_t id, bool value) {
    CheckID(id);

    ObjParam * tmpparam = params[0];
    tmpparam->release();
    tmpparam->type = PT_BOOL;
    tmpparam->value.bValue = value;

    if(id) {
        ChangeParam(id);
        changed = true;
    }
}

void ParamPack::set(paramid_t id, Object * value) {
    CheckID(id);

    ObjParam * tmpparam = params[0];
    tmpparam->release();
    tmpparam->type = PT_OBJECT;
    tmpparam->value.objValue = value;

    if(id) {
        ChangeParam(id);
        changed = true;
    }
}

void ParamPack::set(paramid_t id, const char * text) {
    if(ParamPackFactory::parseValue(this, id, text))
        return;

    CheckID(id);

    ObjParam * tmpparam = params[0];
    tmpparam->release();

    tmpparam->type = PT_STRING;
    tmpparam->value.sValue = ::strdup(text);

    if(id) {
        ChangeParam(id);
        changed = true;
    }
}

void ParamPack::set(paramid_t id, const Vector2f & vec) {
    CheckID(id);

    ObjParam * tmpparam = params[0];
    tmpparam->release();
    tmpparam->type = PT_VECTOR2;
    tmpparam->value.vecValue = new Vector4f(vec[0], vec[1], 0.0f, 0.0f);

    if(id) {
        ChangeParam(id);
        changed = true;
    }
}

void ParamPack::set(paramid_t id, const Vector3f & vec) {
    CheckID(id);
        
    ObjParam * tmpparam = params[0];
    tmpparam->release();
    tmpparam->type = PT_VECTOR3;
    tmpparam->value.vecValue = new Vector4f(vec[0], vec[1], vec[2], 0.0f);

    if(id) {
        ChangeParam(id);
        changed = true;
    }
}

void ParamPack::set(paramid_t id, const Vector4f & vec) {
    CheckID(id);

    ObjParam * tmpparam = params[0];
    tmpparam->release();
    tmpparam->type = PT_VECTOR4;
    tmpparam->value.vecValue = new Vector4f(vec);

    if(id) {
        ChangeParam(id);
        changed = true;
    }
}

void ParamPack::get(paramid_t id, int & value) {
    CheckID(id);

    switch(params[id]->type) {
        case PT_INT :
            value = params[id]->value.iValue;
            break;

        case PT_FLOAT :
            value = (int) params[id]->value.fValue;
            break;

        case PT_BOOL :
            value = params[id]->value.bValue ? 1 : 0;
            break;

        case PT_OBJECT :
            value = 0;
            break;

        case PT_STRING :
            value = ::atoi(params[id]->value.sValue);
            break;

        case PT_VECTOR2 :
        case PT_VECTOR3 :
        case PT_VECTOR4 :
            value = (int) params[id]->value.vecValue->getPtr()[0];
            break;

        default:
            throw Exception("ccor : bad parameter type");
    }
}

void ParamPack::get(paramid_t id, float & value) {
    CheckID(id);

    switch(params[id]->type) {
        case PT_INT :
            value = (float)params[id]->value.iValue;
            break;

        case PT_FLOAT :
            value = params[id]->value.fValue;
            break;

        case PT_BOOL :
            value = params[id]->value.bValue ? 1.0f : 0.0f;
            break;

        case PT_OBJECT :
            value = 0.0f;
            break;

        case PT_STRING :
            value = float( atof(params[id]->value.sValue) );
            break;

        case PT_VECTOR2 :
        case PT_VECTOR3 :
        case PT_VECTOR4 :
            value = params[id]->value.vecValue->getPtr()[0];
            break;

        default:
            throw Exception("ccor : bad parameter type");
    }
}

void ParamPack::get(paramid_t id, bool & value) {
    CheckID(id);

    switch(params[id]->type) {
        case PT_INT :
            value = params[id]->value.iValue ? true : false;
            break;

        case PT_FLOAT :
            value = params[id]->value.fValue != 0.0f;
            break;

        case PT_BOOL :
            value = params[id]->value.bValue;
            break;

        case PT_OBJECT :
            throw Exception("ccor : cannot convert (Object *) to bool");

        case PT_STRING :
            const char * sValue;
            sValue = params[id]->value.sValue;
            if(!::stricmp(sValue, "true"))
                value = true;
            else
                if(!::stricmp(sValue, "false"))
                    value = false;
                else {
                    int number = ::atoi(sValue);
                    if(number)
                        value = true;
                    else
                        if(sValue[0] == '0')
                            value = false;
                        else
                            throw Exception("ccor : cannot convert string to bool");
                }
            break;

        case PT_VECTOR2 :
        case PT_VECTOR3 :
        case PT_VECTOR4 :
            value = params[id]->value.vecValue->getPtr()[0] != 0.0f;
            break;

        default:
            throw Exception("ccor : bad parameter type");
    }
}

void ParamPack::get(paramid_t id, Object *& value) {
    CheckID(id);

    switch(params[id]->type) {
        case PT_OBJECT :
            value = params[id]->value.objValue;
            break;
        
        case PT_INT :
        case PT_FLOAT :
        case PT_STRING :
        case PT_VECTOR2 :
        case PT_VECTOR3 :
        case PT_VECTOR4 :
            value = NULL;
            break;

        case PT_BOOL :
            throw Exception("ccor : cannot convert bool to (Object *)");

        default:
            throw Exception("ccor : bad parameter type");
    }
}

const char * ParamPack::gets(paramid_t id) {
    CheckID(id);

    switch(params[id]->type) {
        case PT_INT :
            itoa(params[id]->value.iValue, tmpbuf, 10);
            break;

        case PT_FLOAT :
            ::sprintf(tmpbuf, "%f", params[id]->value.fValue);
            int pos;
            for(pos = ::strlen(tmpbuf) - 1; pos && tmpbuf[pos] == '0'; --pos);
            if(tmpbuf[pos] == '.')
                tmpbuf[++pos] = '0';
            tmpbuf[++pos] = '\0';
            break;

        case PT_BOOL :
            if(params[id]->value.bValue)
                ::strcpy(tmpbuf, "true");
            else
                ::strcpy(tmpbuf, "false");
            break;

        case PT_OBJECT :
            throw Exception("ccor : cannot convert (Object *) to string");

        case PT_STRING :
            ::strcpy(tmpbuf, params[id]->value.sValue);
            break;

        case PT_VECTOR2 :
            ParamPackFactory::vectorToStr(params[id]->value.vecValue->getPtr(), 2, tmpbuf);
            break;

        case PT_VECTOR3 :
            ParamPackFactory::vectorToStr(params[id]->value.vecValue->getPtr(), 3, tmpbuf);
            break;

        case PT_VECTOR4 :
            ParamPackFactory::vectorToStr(params[id]->value.vecValue->getPtr(), 4, tmpbuf);
            break;

        default:
            throw Exception("ccor : bad parameter type");
    }

    return tmpbuf;
}

void ParamPack::get(paramid_t id, Vector2f & vec) {
    CheckID(id);
    
    Vector2f tmpvec;

    switch(params[id]->type) {
        case PT_INT :
            vec[0] = float( params[id]->value.iValue );
            vec[1] = 0;
            break;

        case PT_FLOAT :
            vec[0] = params[id]->value.fValue;
            vec[1] = 0;
            break;

        case PT_BOOL :
            vec[0] = float( params[id]->value.bValue ? 1 : 0 );
            vec[1] = 0;
            break;

        case PT_OBJECT :
            throw Exception("ccor : cannot convert (Object *) to Vector2f");

        case PT_STRING :
            if(ParamPackFactory::strToVector(params[id]->value.sValue, tmpvec.getPtr(), 2) == 2)
                vec = tmpvec;
            else
                throw Exception("ccor : cannot convert String to Vector2f");
            break;

        case PT_VECTOR2 :
        case PT_VECTOR3 :
        case PT_VECTOR4 :
            vec[0] = params[id]->value.vecValue->getPtr()[0];
            vec[1] = params[id]->value.vecValue->getPtr()[1];
            break;

        default:
            throw Exception("ccor : bad parameter type");
    }
}

void ParamPack::get(paramid_t id, Vector3f & vec) {
    CheckID(id);

    Vector3f tmpvec;

    switch(params[id]->type) {
        case PT_INT :
            vec[0] = float( params[id]->value.iValue );
            vec[1] = vec[2] = 0;
            break;

        case PT_FLOAT :
            vec[0] = params[id]->value.fValue;
            vec[1] = vec[2] = 0;
            break;

        case PT_BOOL :
            vec[0] = float( params[id]->value.bValue ? 1 : 0 );
            vec[1] = vec[2] = 0;
            break;

        case PT_OBJECT :
            throw Exception("ccor : cannot convert (Object *) to Vector3f");

        case PT_STRING :
            if(ParamPackFactory::strToVector(params[id]->value.sValue, tmpvec.getPtr(), 3) == 3)
                vec = tmpvec;
            else
                throw Exception("ccor : cannot convert String to Vector3f");
            break;

        case PT_VECTOR2 :
        case PT_VECTOR3 :
        case PT_VECTOR4 :
            vec[0] = params[id]->value.vecValue->getPtr()[0];
            vec[1] = params[id]->value.vecValue->getPtr()[1];
            vec[2] = params[id]->value.vecValue->getPtr()[2];
            break;

        default:
            throw Exception("ccor : bad parameter type");
    }
}

void ParamPack::get(paramid_t id, Vector4f & vec) {
    CheckID(id);

    Vector4f tmpvec;

    switch(params[id]->type) {
        case PT_INT :
            vec[0] = float( params[id]->value.iValue );
            vec[1] = vec[2] = vec[3] = 0;
            break;

        case PT_FLOAT :
            vec[0] = params[id]->value.fValue;
            vec[1] = vec[2] = vec[3] = 0;
            break;

        case PT_BOOL :
            vec[0] = float( params[id]->value.bValue ? 1 : 0 );
            vec[1] = vec[2] = vec[3] = 0;
            break;

        case PT_OBJECT :
            throw Exception("ccor : cannot convert (Object *) to Vector4f");

        case PT_STRING :
            if(ParamPackFactory::strToVector(params[id]->value.sValue, (float *) &tmpvec, 4) == 4)
                vec = tmpvec;
            else
                throw Exception("ccor : cannot convert String to Vector4f");
            break;

        case PT_VECTOR2 :
        case PT_VECTOR3 :
        case PT_VECTOR4 :
            vec = *params[id]->value.vecValue;
            break;

        default:
            throw Exception("ccor : bad parameter type");
    }
}

const char * __stdcall ParamPack::getv(paramid_t id, const char * dv) {
    if (id >= 0) return ParamPack::gets(id);
    return dv; 
}

int __stdcall ParamPack::getv(paramid_t id, int dv) {
    if (id >= 0) ParamPack::get(id, dv);
    return dv; 
}

float __stdcall ParamPack::getv(paramid_t id, float dv) {
    if (id >= 0) ParamPack::get(id, dv);
    return dv; 
}

bool __stdcall ParamPack::getv(paramid_t id, bool dv) {
    if (id >= 0) ParamPack::get(id, dv);
    return dv; 
}

Object * __stdcall ParamPack::getv(paramid_t id, Object * dv) {
    if (id >= 0) ParamPack::get(id, dv);
    return dv; 
}

Vector2f __stdcall ParamPack::getv(paramid_t id, Vector2f dv) {
    if (id >= 0) ParamPack::get(id, dv);
    return dv; 
}

Vector3f __stdcall ParamPack::getv(paramid_t id, const Vector3f& dv) {
    Vector3f v = dv;
    if (id >= 0) ParamPack::get(id, v);
    return v; 
}

Vector4f __stdcall ParamPack::getv(paramid_t id, const Vector4f& dv) {
    Vector4f v = dv;
    if (id >= 0) ParamPack::get(id, v);
    return v;
}

void ParamPack::optimizeValue(paramid_t id) {
    CheckID(id);

    ObjParam * param = params[id];

    if(param->type == PT_VECTOR2 || param->type == PT_VECTOR3 || param->type == PT_VECTOR4) {
        float * vec = param->value.vecValue->getPtr();
        if(param->type == PT_VECTOR4 && vec[3] == 0.0f) {
            param->type = PT_VECTOR3;
        }
        if(param->type == PT_VECTOR3 && vec[2] == 0.0f) {
            param->type = PT_VECTOR2;
        }
        if(param->type == PT_VECTOR2 && vec[1] == 0.0f) {
            float fValue = vec[0];
            param->release();
            param->type = PT_FLOAT;
            param->value.fValue = fValue;
        }
    }

    if(param->type == PT_FLOAT) {
        int iValue = (int) param->value.fValue;
        if(((float) iValue) == param->value.fValue) {
            param->type = PT_INT;
            param->value.iValue = iValue;
        }
    }
}

void ParamPack::optimizeValues() {
     for(paramid_t id = 0; id < int(params.size()); ++id)
         optimizeValue(id);
}

paramid_t ParamPack::find(const char * qualifiedName) {
    if(!qualifiedName || !qualifiedName[0])
        throw Exception("ccor : bad qualified name");

    tmpbuf[0] = '\0';

    char findname[BUF_SIZE];
    ::strcpy(findname, qualifiedName);

    ParamMap::iterator it;
    paramid_t id = -1;
    char * templend = ::strchr(findname, '*');
    if(templend) {
        *templend = '\0';
        if((it = ids.lower_bound(findname)) != ids.end())
            if(!::strncmp(it->first, findname, ::strlen(findname)))
                id = it->second;
    }
    else {

        char * leftBracket = ::strrchr(findname, '[');
        if(leftBracket) {

            int i = ::atoi(leftBracket + 1);
            if(i)
                ::sprintf(leftBracket + 1, "%04d]", i);
            else
                *leftBracket = '\0';
        }

        if((it = ids.find(findname)) != ids.end())
            id = it->second;
    }

    if(id == -1) ::strcpy(tmpbuf, qualifiedName);

    return id;
}

int ParamPack::findParams(const char * qualifiedName) {
    findBuf.clear();

    char findname[BUF_SIZE];
    ::strcpy(findname, qualifiedName);
    ParamMap::iterator it;
    char * templend = ::strchr(findname, '*');
    if(templend) {
        *templend = '\0';
        it = ids.lower_bound(findname);

        *templend = '\xFF';
        *(templend + 1) = '\0';
        ParamMap::iterator upperit = ids.upper_bound(findname);

        while(it != upperit) {
            findBuf.push_back(it->second);
            ++it;
        }
    }
    else {
        it = ids.find(qualifiedName);
        if(it != ids.end())
            findBuf.push_back(it->second);
    }

    paramResult = findBuf;
    return findBuf.size();
}

void ParamPack::copyParamResult(paramid_t * array) {
    for (int i=0; i < paramResult.size(); ++i) array[i] = paramResult[i];
}

inline void ParamPack::addListener(ParamPackListener * listener) {
    listeners.push_back(listener);
}

void ParamPack::removeListener(ParamPackListener * listener) {
    for(ListenerVector::iterator it = listeners.begin(); it != listeners.end(); ++it)
        if(*it == listener) {
            listeners.erase(it);
            return;
        }

    throw Exception("ccor : bad listener pointer");
}

void ParamPack::copySubsetFrom(IParamPack * pack, const char * templateName, const char * subst) {
    if (pack==NULL) return;
    int numItems = pack->findParams(templateName);
    //find_param_t fpResult = pack->findParams(templateName);
    int asteriskPos = -1;
    if (subst!=NULL) {
        const char * p = strchr(templateName, '*');
        if (p) asteriskPos = p - templateName;
    }
    std::string sname;
    for (int i=0; i < numItems; ++i) {
        paramid_t id = paramResult[i];
        const char * name = pack->getName(id);
        const char * sval = pack->gets(id);
        if (subst!=NULL) {
            sname = subst;
            sname += &name[asteriskPos];
            name = sname.c_str();
        }
        this->set(name, sval);
    }
}

UTF8Converter::UTF8Converter() {
    utftoascii[39376] = 'É';
    utftoascii[42704] = 'Ö';
    utftoascii[41936] = 'Ó';
    utftoascii[39632] = 'Ê';
    utftoascii[38352] = 'Å';
    utftoascii[40400] = 'Í';
    utftoascii[37840] = 'Ã';
    utftoascii[43216] = 'Ø';
    utftoascii[43472] = 'Ù';
    utftoascii[38864] = 'Ç';
    utftoascii[42448] = 'Õ';
    utftoascii[43728] = 'Ú';
    utftoascii[42192] = 'Ô';
    utftoascii[43984] = 'Û';
    utftoascii[37584] = 'Â';
    utftoascii[37072] = 'À';
    utftoascii[40912] = 'Ï';
    utftoascii[41168] = 'Ð';
    utftoascii[40656] = 'Î';
    utftoascii[39888] = 'Ë';
    utftoascii[38096] = 'Ä';
    utftoascii[38608] = 'Æ';
    utftoascii[44496] = 'Ý';
    utftoascii[45008] = 'ß';
    utftoascii[42960] = '×';
    utftoascii[41424] = 'Ñ';
    utftoascii[40144] = 'Ì';
    utftoascii[39120] = 'È';
    utftoascii[41680] = 'Ò';
    utftoascii[44240] = 'Ü';
    utftoascii[37328] = 'Á';
    utftoascii[44752] = 'Þ';
    utftoascii[47568] = 'é';
    utftoascii[34513] = 'ö';
    utftoascii[33745] = 'ó';
    utftoascii[47824] = 'ê';
    utftoascii[46544] = 'å';
    utftoascii[48592] = 'í';
    utftoascii[46032] = 'ã';
    utftoascii[35025] = 'ø';
    utftoascii[35281] = 'ù';
    utftoascii[47056] = 'ç';
    utftoascii[34257] = 'õ';
    utftoascii[35537] = 'ú';
    utftoascii[34001] = 'ô';
    utftoascii[35793] = 'û';
    utftoascii[45776] = 'â';
    utftoascii[45264] = 'à';
    utftoascii[49104] = 'ï';
    utftoascii[32977] = 'ð';
    utftoascii[48848] = 'î';
    utftoascii[48080] = 'ë';
    utftoascii[46288] = 'ä';
    utftoascii[46800] = 'æ';
    utftoascii[36305] = 'ý';
    utftoascii[36817] = 'ÿ';
    utftoascii[34769] = '÷';
    utftoascii[33233] = 'ñ';
    utftoascii[48336] = 'ì';
    utftoascii[47312] = 'è';
    utftoascii[33489] = 'ò';
    utftoascii[36049] = 'ü';
    utftoascii[45520] = 'á';
    utftoascii[36561] = 'þ';
    utftoascii[33232] = '¨';
    utftoascii[37329] = '¸';

    asciitoutf['É'] = 39376;
    asciitoutf['Ö'] = 42704;
    asciitoutf['Ó'] = 41936;
    asciitoutf['Ê'] = 39632;
    asciitoutf['Å'] = 38352;
    asciitoutf['Í'] = 40400;
    asciitoutf['Ã'] = 37840;
    asciitoutf['Ø'] = 43216;
    asciitoutf['Ù'] = 43472;
    asciitoutf['Ç'] = 38864;
    asciitoutf['Õ'] = 42448;
    asciitoutf['Ú'] = 43728;
    asciitoutf['Ô'] = 42192;
    asciitoutf['Û'] = 43984;
    asciitoutf['Â'] = 37584;
    asciitoutf['À'] = 37072;
    asciitoutf['Ï'] = 40912;
    asciitoutf['Ð'] = 41168;
    asciitoutf['Î'] = 40656;
    asciitoutf['Ë'] = 39888;
    asciitoutf['Ä'] = 38096;
    asciitoutf['Æ'] = 38608;
    asciitoutf['Ý'] = 44496;
    asciitoutf['ß'] = 45008;
    asciitoutf['×'] = 42960;
    asciitoutf['Ñ'] = 41424;
    asciitoutf['Ì'] = 40144;
    asciitoutf['È'] = 39120;
    asciitoutf['Ò'] = 41680;
    asciitoutf['Ü'] = 44240;
    asciitoutf['Á'] = 37328;
    asciitoutf['Þ'] = 44752;
    asciitoutf['é'] = 47568;
    asciitoutf['ö'] = 34513;
    asciitoutf['ó'] = 33745;
    asciitoutf['ê'] = 47824;
    asciitoutf['å'] = 46544;
    asciitoutf['í'] = 48592;
    asciitoutf['ã'] = 46032;
    asciitoutf['ø'] = 35025;
    asciitoutf['ù'] = 35281;
    asciitoutf['ç'] = 47056;
    asciitoutf['õ'] = 34257;
    asciitoutf['ú'] = 35537;
    asciitoutf['ô'] = 34001;
    asciitoutf['û'] = 35793;
    asciitoutf['â'] = 45776;
    asciitoutf['à'] = 45264;
    asciitoutf['ï'] = 49104;
    asciitoutf['ð'] = 32977;
    asciitoutf['î'] = 48848;
    asciitoutf['ë'] = 48080;
    asciitoutf['ä'] = 46288;
    asciitoutf['æ'] = 46800;
    asciitoutf['ý'] = 36305;
    asciitoutf['ÿ'] = 36817;
    asciitoutf['÷'] = 34769;
    asciitoutf['ñ'] = 33233;
    asciitoutf['ì'] = 48336;
    asciitoutf['è'] = 47312;
    asciitoutf['ò'] = 33489;
    asciitoutf['ü'] = 36049;
    asciitoutf['á'] = 45520;
    asciitoutf['þ'] = 36561;
    asciitoutf['¨'] = 33232;
    asciitoutf['¸'] = 37329;

    signature[0] = '\xEF';
    signature[1] = '\xBB';
    signature[2] = '\xBF';
    signature[3] = '\0';
}

int UTF8Converter::fromUTF8ToASCII(const char * utf_s, char * s, int slen) {
    char ascii_s[BUF_SIZE];
    UTF8Map::iterator it;
    int read = 0, write = 0;
    while(s[read]) {
        it = utftoascii.find(*(wchar_t*)(s + read));
        if(it == utftoascii.end())
            ascii_s[write] = s[read++];
        else {
            ascii_s[write] = it->second;
            read += 2;
        }
        ++write;
    }
    ascii_s[write] = '\0';
    if(write > slen - 1)
        throw Exception("ccor : too long line");
    ::strcpy(s, ascii_s);

    return write;
}

int UTF8Converter::fromASCIIToUTF8(const char * ascii_s, char * s, int slen) {
    char utf_s[BUF_SIZE];
    ASCIIMap::iterator it;
    int read = 0, write = 0;
    while(ascii_s[read]) {
        it = asciitoutf.find(ascii_s[read]);
        if(it == asciitoutf.end())
            utf_s[write++] = ascii_s[read];
        else {
            *(wchar_t*)(utf_s + write) = it->second;
            write += 2;
        }
        ++read;
    }
    utf_s[write] = '\0';
    if(write > slen - 1)
        throw Exception("ccor : too long line");
    ::strcpy(s, utf_s);

    return write;
}

inline ObjParam::ObjParam(const char * name) {
    this->name = name;
    type = PT_INT;
    this->value.iValue = 0;
}

void ObjParam::release() {
    switch(type) {
        case PT_STRING :
            free(value.sValue);
            break;

        case PT_VECTOR2 :
        case PT_VECTOR3 :
        case PT_VECTOR4 :
            delete value.vecValue;
            break;
    }
}

inline ConfigItem::ConfigItem(const char * s) {
    param = false;
    content.s = ::strdup(s);
}

inline ConfigItem::ConfigItem(paramid_t id) {
    param = true;
    content.id = id;
}

inline void ConfigItem::release() {
    if(!param)
        free(content.s);
}

inline void ConfigLayout::addString(const char * s) {
    items.push_back(new ConfigItem(s));
}

inline void ConfigLayout::addParam(paramid_t id) {
    items.insert(items.end(), new ConfigItem(id));
}

inline const ConfigItem * ConfigLayout::getConfigItem(int i) {
    if(i < 0 || i >= int(items.size()))
        throw Exception("ccor : bad config item index");

    return items[i];
}

void ConfigLayout::clear() {
    for(ConfigItemVector::iterator it = items.begin(); it != items.end(); ++it)
        delete *it;
    items.clear();
}

int ParamPackFactory::getParamName(char * name, const char * src) {
    for( int srcpos = 0, namepos = 0; src[srcpos] && src[srcpos] != '='; ++srcpos )
        if(src[srcpos] != ' ')
            name[namepos++] = src[srcpos];
    name[namepos] = '\0';
    //
    if(src[srcpos] == '=')
        ++srcpos;
    //
    return srcpos;
}

// NOT USED !!!
void ParamPackFactory::getParamValue(char * value, const char * src) {
    int srcpos = 0, valuepos = 0;
    char curbracket = '\0';
    bool prevbracket = false;
    while(src[srcpos]/* && (curbracket || (src[srcpos] != ';' && src[srcpos] != '#'))*/) {
        switch(src[srcpos]) {
            case '\'' :
            case '\"' :
                if(!curbracket && strchr(src + srcpos + 1, src[srcpos])) {
                    curbracket = src[srcpos];
                    if(prevbracket)
                        --valuepos;
                }
                else
                    if(curbracket == src[srcpos]) {
                        curbracket = '\0';
                        prevbracket = true;
                    }
                    else
                        value[valuepos++] = src[srcpos];
                break;

            case ' ' :
                if( curbracket ||
                    valuepos &&
                    src[srcpos + 1] &&
                    src[srcpos + 1] != ' ' /*&&
                    src[srcpos + 1] != ';' &&
                    src[srcpos + 1] != '#'*/ )

                    value[valuepos++] = src[srcpos];
                break;

            default:
                value[valuepos++] = src[srcpos];
                if(!curbracket)
                    prevbracket = false;
        }
        ++srcpos;
    }

    value[valuepos] = '\0';
}

void ParamPackFactory::getParamValues(StringVector & values, const char * src) {

    values.clear();

    char value[BUF_SIZE] = "";
    int srcpos = 0, valuepos = 0;
    char curbracket = '\0';
    bool prevbracket = false;
    //
    int leftBracketPos = -1;
    //
    while(src[srcpos]/* && (curbracket || (src[srcpos] != ';' && src[srcpos] != '#'))*/) {
        switch(src[srcpos]) {
            case '\'' :
            case '\"' :
                if(!curbracket && strchr(src + srcpos + 1, src[srcpos])) {
                    curbracket = src[srcpos];
                    if(prevbracket)
                        --valuepos;
                    //
                    leftBracketPos = valuepos;
                    //
                }
                else
                    if(curbracket == src[srcpos]) {
                        curbracket = '\0';
                        prevbracket = true;
                        //
                        value[valuepos] = '\0';
                        valuepos -= translateEscapeSequences(value + leftBracketPos);
                        leftBracketPos = -1;
                        //
                    }
                    else
                        value[valuepos++] = src[srcpos];
                break;

            case ' ' :
                if( curbracket ||
                    valuepos &&
                    src[srcpos + 1] &&
                    src[srcpos + 1] != ' ' &&
                    src[srcpos + 1] != '|' /*&&
                    src[srcpos + 1] != ';' &&
                    src[srcpos + 1] != '#'*/ )

                    value[valuepos++] = src[srcpos];
                break;

            case '|' :
                if(!curbracket) {

                    value[valuepos] = '\0';
                    values.push_back(value);
                    valuepos = 0;
                }
                else
                    value[valuepos++] = src[srcpos];
                break;

            default:
                value[valuepos++] = src[srcpos];
                if(!curbracket)
                    prevbracket = false;
        }
        ++srcpos;
    }

    value[valuepos] = '\0';
    values.push_back(value);
}

int ParamPackFactory::toOct(const char * s, char & ch) {
    int len;
    for( ch = 0, len = 0; len < 3; ++len)
        if(s[len] >= '0' && s[len] <= '9')
            ch = ch*8 + (s[len] - '0');
        else
            break;

    return len;
}

int ParamPackFactory::toHex(const char * s, char & ch) {
    int len;
    for( ch = 0, len = 0; len < 2; ++len)
        if(s[len] >= '0' && s[len] <= '9')
            ch = 16*ch + s[len] - '0';
        else
            if(s[len] >= 'a' && s[len] <= 'f')
                ch = 16*ch + 10 + s[len] - 'a';
            else
                if(s[len] >= 'A' && s[len] <= 'F')
                    ch = 16*ch + 10 + s[len] - 'A';
                else
                    break;

    return len;
}

int ParamPackFactory::translateEscapeSequences(char * s) {
    int read = 0, write = 0, len;
    char ch;
    for( ; s[read]; ++read, ++write)
        if(s[read] == '\\') {
            switch(s[read + 1]) {
                case 'a' : s[write] = '\a'; break;
                case 'b' : s[write] = '\b'; break;
                case 'f' : s[write] = '\f'; break;
                case 'n' : s[write] = '\n'; break;
                case 'r' : s[write] = '\r'; break;
                case 't' : s[write] = '\t'; break;
                case 'v' : s[write] = '\v'; break;
                case '?' : s[write] = '\?'; break;
                case '\\' : s[write] = '\\'; break;

                case 'x' :
                case 'X' :
                    if(len = toHex(s + read + 2, ch)) {
                        read += len;
                        s[write] = ch;
                    }
                    break;

                default:
                    if(len = toOct(s + read + 1, ch)) {
                        read += len - 1;
                        s[write] = ch;
                    }
                    else {
                        --write;
                        continue;
                    }
            }

            ++read;
        }
        else
            s[write] = s[read];

    s[write] = '\0';

    return read - write;
}

void ParamPackFactory::backTranslateEscapeSequences(char * s) {
    std::string trans = "";
    int read = 0;
    unsigned char hex;
    for( ; s[read]; ++read) {
       switch(s[read]) {
            case '\a' : trans += "\\a"; break;
            case '\b' : trans += "\\b"; break;
            case '\f' : trans += "\\f"; break;
            case '\n' : trans += "\\n"; break;
            case '\r' : trans += "\\r"; break;
            case '\t' : trans += "\\t"; break;
            case '\v' : trans += "\\v"; break;
            case '\?' : trans += "\\?"; break;
            case '\\' : trans += "\\\\"; break;

            default:
                if(isPrintable(s[read]))
                    trans += s[read];
                else {
                    trans += "\\x";
                    if((hex = ((unsigned char) s[read])/16) >= 10)
                        hex += 'a' - 10;
                    else
                        hex += '0';
                    trans += hex;

                    if((hex = ((unsigned char) s[read])%16) >= 10)
                        hex += 'a' - 10;
                    else
                        hex += '0';
                    trans += hex;
                }
        }
    }

    if(trans.length() > BUF_SIZE - 1)
        throw Exception("ccor : too long translated string");

    ::strcpy(s, trans.c_str());
}

bool ParamPackFactory::isInt(const char * s) {
    for(int pos = 0; s[pos]; ++pos )
        if(s[pos] < '0' || s[pos] > '9' )
            return false;

    return pos != 0;
}

bool ParamPackFactory::isFloat(const char * s) {
    bool point = false;
    int pos = 0;
    for( ; s[pos]; ++pos )
        if(s[pos] < '0' || s[pos] > '9' )
            if(s[pos] == '.')
                if(!point)
                    point = true;
                else
                    return false;
            else
                if(!(s[pos] == '-' && pos == 0))
                    return false;

    return pos != 0;
}

void ParamPackFactory::strTrim(const char * src, char * dest ) {
    int read = 0, write = 0;

    while(src[read] == ' ')
        ++read;

    while(src[read])
        dest[write++] = src[read++];

    if(write)
        while(dest[write - 1] == ' ')
            --write;

    dest[write] = '\0';
}

#pragma warning(disable:144)
#pragma warning(disable:556)

int ParamPackFactory::strToVector(const char * s, float * vec, int maxveclen) {
    if(s[0] == '\0')
        return 0;

    int pos, start, end;
    for(pos = 0; s[pos] == ' '; ++pos);
    if(s[pos] != '{')
        return 0;
    start = pos + 1;

    for(pos = ::strlen(s) - 1; pos > 0 && s[pos] == ' '; --pos);
    if(s[pos] != '}')
        return 0;
    end = pos;

    char buf[BUF_SIZE], field[BUF_SIZE];
    ::strncpy(buf, s + start, end -= start);
    buf[end] = '\0';
    for(pos = 0; buf[pos]; ++pos)
        if(buf[pos] == ',')
            buf[pos] = '\0';

    int veclen = 0;
    pos = 0;
    while(pos < end) {
        if(veclen == maxveclen)
            return 0;

        strTrim(buf + pos, field);
        if(isFloat(field)) {
            vec[veclen++] = float( atof(field) );
            pos += ::strlen(buf + pos) + 1;
        }
        else
            return 0;
    }

    return veclen;
}

void ParamPackFactory::vectorToStr(const float * vec, int veclen, char * s) {
    char * eye = s;
    *(eye++) = '{';
    for(int i = 0; i < veclen; ++i) {
        *(eye++) = ' ';
        eye += ::sprintf(eye, "%f", vec[i]);
        while(*(eye - 1) == '0')
            --eye;
        if(*(eye - 1) == '.')
            *(eye++) = '0';
        *(eye++) = ',';
    }
    *(eye - 1) = ' ';
    *(eye++) = '}';
    *eye = '\0';
}

#pragma warning(default:144)
#pragma warning(default:556)

bool ParamPackFactory::parseValue(ParamPack * pack, paramid_t id, const char * value) {
    Vector4f vec;
    int veclen;

    if(isInt(value))
        pack->set(id, atoi(value));
    else if(isFloat(value))
        pack->set(id, (float)atof(value));
    else if(!::stricmp(value, "true"))
        pack->set(id, true);
    else if(!::stricmp(value, "on"))
        pack->set(id, true);
    else if(!::stricmp(value, "false"))
        pack->set(id, false);
    else if(!::stricmp(value, "off"))
        pack->set(id, false);
    else if(veclen = strToVector(value, vec.getPtr(), 4), veclen == 2)
        pack->set(id, Vector2f(vec[0], vec[1]));
    else if(veclen == 3)
        pack->set(id, Vector3f(vec[0], vec[1], vec[2]));
    else if(veclen == 4)
        pack->set(id, Vector4f(vec[0], vec[1], vec[2], vec[3]));
    else
        return false;

    return true;
}

inline bool ParamPackFactory::isPrintable(unsigned char ch) {
    return ch >= 0x20;
}

bool ParamPackFactory::isComment(const char * s) {
    int pos = 0;
    while(s[pos] == ' ')
        ++pos;
    if(s[pos] == '#' || s[pos] == ';')
        return true;

    return pos != 0;
}

void ParamPackFactory::getParentDir(const char * fname, char * dir) {

    int separator = 0;
    for(int i = 0; fname[i]; ++i) {

        dir[i] = fname[i];
        if(fname[i] == '\\' || fname[i] == '/') separator = i;
    }
    dir[separator ? separator + 1 : 0] = '\0';
}

void ParamPackFactory::getFullParamName( char * name,
                                         const char * srcName,
                                         const std::string & prefix,
                                         const std::string & substSrc,
                                         const std::string & substDst ) {

    ::strncpy(name, prefix.c_str(), prefix.length());
    name += prefix.length();

    if(substSrc.empty())
        ::strcpy(name, srcName);
    else {

        for(const char * ch = srcName; *ch; ) {

            if(!::strncmp(ch, substSrc.c_str(), substSrc.length())) {

                ::strncpy(name, substDst.c_str(), substDst.length());
                ch += substSrc.length();
                name += substDst.length();
            }
            else
                *(name++) = *(ch++);
        }
        *name = '\0';
    }
}

void ParamPackFactory::nextLine(ParamPack * pack) {
    preadbuf->length = 0;
    preadbuf->buffer[0] = '\0';
    while(!preadbuf->length) {
        preadbuf->buffer[0] = '\0';
        ::fgets(preadbuf->buffer, BUF_SIZE, res->getFile());
        preadbuf->length = strlen(preadbuf->buffer);
        if(feof(res->getFile()) && !preadbuf->length)
            break;

        if(preadbuf->buffer[preadbuf->length - 1] == '\n')
            preadbuf->buffer[--preadbuf->length] = '\0';
        else
            if(preadbuf->length == BUF_SIZE - 1)
                throw Exception("ccor : too long line");

        preadbuf->length = utfconv.fromUTF8ToASCII(preadbuf->buffer, preadbuf->buffer, BUF_SIZE);

        if(!preadbuf->length || isComment(preadbuf->buffer)) {
            //pack->layout.addString(preadbuf->buffer);
            //++preadbuf->passComments;
            preadbuf->passComments.push_back(preadbuf->buffer);
            //
            preadbuf->length = 0;
        }
    }

    //
    char ch, bracket = '\0';
    for(int i = 0; i < preadbuf->length; ++i) {

        ch = preadbuf->buffer[i];
        switch(ch) {

            case '\'' :
            case '\"' :
                if(bracket) {

                    if(bracket == ch)  bracket = '\0';
                }
                else
                    bracket = ch;
                break;

            case '#' :
            case ';' :
                if(!bracket) {

                    preadbuf->buffer[i] = '\0';
                    return;
                }
        }
    }
    //
}

int ParamPackFactory::nextString(ParamPack * pack, char * buf, paramid_t * id) {
    int buflen = 0, tmpbufpos = 0;
    buf[0] = '\0';

    if(!preadbuf->length)
        nextLine(pack);

    while(preadbuf->length - tmpbufpos > 0) {
        buflen += preadbuf->length - tmpbufpos;
        if(buflen >= BUF_SIZE - 1)
            throw Exception("ccor : too long line");
        ::strcat(buf, preadbuf->buffer + tmpbufpos);
        //preadbuf->passComments = 0;
        for(StringVector::iterator it = preadbuf->passComments.begin(); it != preadbuf->passComments.end(); ++it)
            pack->layout.addString(it->c_str());
        preadbuf->passComments.clear();
        //

        nextLine(pack);
        tmpbufpos = 0;

        while(buf[buflen - 1] == ' ')
            buf[--buflen] = '\0';

        if(buf[buflen - 1] == '\\') {
            buf[--buflen] = '\0';
            continue;
        }
        else {
            while(preadbuf->buffer[tmpbufpos] == ' ')
                ++tmpbufpos;

            if(preadbuf->buffer[tmpbufpos] == '\\') {
               ++tmpbufpos;
                continue;
            }
        }

        break;
    }

    return buflen;
}

void ParamPackFactory::saveParam(ParamPack * pack, paramid_t id) {
/*
    ParamType type = pack->getType(id);
    if(type != PT_OBJECT) {
        char param[BUF_SIZE];
        ::strcpy(param, pack->gets(id));
        //if(::strncmp(param, "$.", 2)) {
            backTranslateEscapeSequences(param);
            char utf_s[BUF_SIZE];
            utfconv.fromASCIIToUTF8(param, utf_s, BUF_SIZE);
            if(type == PT_STRING)
                fprintf(res->getFile(), "%s = \"%s\"\n", pack->getName(id), utf_s);
            else
                fprintf(res->getFile(), "%s = %s\n", pack->getName(id), utf_s);
        //}
    }
*/

    if(::strrchr(pack->getName(id), '[')) return;

    char param[BUF_SIZE];
    ::strcpy(param, pack->getName(id));
    ParamType type = pack->getType(id);
    char value[BUF_SIZE];
    char utf_s[BUF_SIZE];
    if(type != PT_OBJECT) {

        ::strcpy(value, pack->gets(id));
        backTranslateEscapeSequences(value);
        utfconv.fromASCIIToUTF8(value, utf_s, BUF_SIZE);
        if(type == PT_STRING)
            fprintf(res->getFile(), "%s = \"%s\"", param, utf_s);
        else
            fprintf(res->getFile(), "%s = %s", param, utf_s);
    }

    ::strcat(param, "[*");
    int numItems = pack->findParams(param);
    paramid_t* ids = new paramid_t[numItems];
    pack->copyParamResult(ids);
    for(int i = 0; i < numItems; ++i)
        if((type = pack->getType(ids[i])) != PT_OBJECT) {

            ::strcpy(value, pack->gets(ids[i]));
            backTranslateEscapeSequences(value);
            utfconv.fromASCIIToUTF8(value, utf_s, BUF_SIZE);
            if(type == PT_STRING)
                fprintf(res->getFile(), " | \"%s\"", utf_s);
            else
                fprintf(res->getFile(), " | %s", utf_s);
        }
	delete[] ids;
    fprintf(res->getFile(), "\n");
}

void ParamPackFactory::loadFromFile( ParamPack * pack,
                                     const char * fname,
                                     StringSet & loaded,
                                     const char * type,
                                     const std::string & prefix,
                                     const std::string & substSrc,
                                     const std::string & substDst ) {

    std::string includePrefix = prefix;
    std::string includeSubstSrc = "";
    std::string includeSubstDst = "";

    std::pair<StringSet::iterator, bool> insertResult = loaded.insert(fname);
    if(!insertResult.second) return;

    ICore* icore = SingleCore::getInstance();
    res = icore->getResource(fname, "rt", type);
    if(!res) throw Exception("ccor: cannot load %s", fname);

    if(loaded.size() == 1) pack->resName = fname;

    const char * signature = utfconv.getUTF8Signature();
    unsigned int signlen = ::strlen(signature);
    READBUF readbuf;
    readbuf.buffer[0] = '\0';
    ::fread(readbuf.buffer, 1, signlen, res->getFile());
    if(::strncmp(signature, readbuf.buffer, signlen))
        ::rewind(res->getFile());
    readbuf.buffer[0] = '\0';
    readbuf.length = 0;

    preadbuf = &readbuf;
    char str[BUF_SIZE], /*value[BUF_SIZE],*/ name[BUF_SIZE], fullName[BUF_SIZE];
    //
    StringVector values;
    //
    paramid_t id;
    int strpos;
    while(nextString(pack, str, &id))
        if(str[0]) {

            strpos = getParamName(name, str);
            //getParamValue(value, str + strpos/* + 1*/);
            //
            //translateEscapeSequences(str + strpos);
            getParamValues(values, str + strpos);
            //
            //translateEscapeSequences(value);
            if(!::strcmp(name, "$.include")) {

                char path[BUF_SIZE];
                getParentDir(pack->resName.c_str(), path);
                ::strcat(path, values[0].c_str());

                IResource * curRes = res;
                loadFromFile(pack, path, loaded, NULL, includePrefix, includeSubstSrc, includeSubstDst);
                res = curRes;
                preadbuf = &readbuf;
            }
            else if(!::strcmp(name, "$.include_prefix")) {

                includePrefix = prefix;
                includePrefix += values[0].c_str();
            }
            else if(!::strcmp(name, "$.include_subst_src")) {

                includeSubstSrc = values[0].c_str();
            }
            else if(!::strcmp(name, "$.include_subst_dst")) {

                includeSubstDst = values[0].c_str();
            }
            else {

                //
                getFullParamName(fullName, name, prefix, substSrc, substDst);
                //::strncpy(fullName, includePrefix.c_str(), includePrefix.length());
                //::strcpy(fullName + includePrefix.length(), name);
                //

                id = pack->add(fullName);
                //pack->set(id, value);
                //
                pack->set(id, values[0].c_str());
                //
                pack->layout.addParam(id);

                for(StringVector::iterator it = readbuf.passComments.begin(); it != readbuf.passComments.end(); ++it)
                    pack->layout.addString(it->c_str());
                readbuf.passComments.clear();
                //
                char value[BUF_SIZE] = "";
                int namelen = ::strlen(name);
                for(int i = 1; i < int(values.size()); ++i) {

                    ::sprintf(name + namelen, "[%04d]", i);
                    ::strcpy(value, values[i].c_str());
                    //translateEscapeSequences(value);
                    pack->set(pack->add(name), value);
                }
                //
            }
        }

    preadbuf = NULL;

    res->release();
    res = NULL;

    loaded.erase(insertResult.first);
}

IParamPack * ParamPackFactory::load(const char * name, const char * type) {

    if(!name) throw Exception("ccor : bad resource name");

    checkLocalization();

    ParamPack * ppack = (ParamPack *) createInstance();
    loadFromFile(ppack, name, StringSet(), type);

    ICore* icore = SingleCore::getInstance();
    ((ParamPack *) ppack)->lastModified = icore->getResourceTime(name);
    ppack->changed = false;

    return ppack;
}

void ParamPackFactory::load(IParamPack * ppack, const char * name, bool append, const char * type) {

    if(!name) throw Exception("ccor : bad resource name");

    checkLocalization();

    ParamPack * pack = (ParamPack *) ppack;

    if(!append) {

        pack->ids.clear();

        assert(!pack->params.empty());
        for(ParamVector::iterator pvit = pack->params.begin() + 1; pvit != pack->params.end(); ++pvit)
            delete *pvit;
        //pack->params.clear();
        pack->params.erase(pack->params.begin() + 1, pack->params.end());

        pack->layout.clear();
    }

    loadFromFile(pack, name, StringSet(), type);

    ICore* icore = SingleCore::getInstance();
    ((ParamPack *) ppack)->lastModified = icore->getResourceTime(name);
    pack->changed = false;
}

void ParamPackFactory::saveToFile(ParamPack * pack, bool saveUTF8Signature) {
    if(saveUTF8Signature) {
        const char * signature = utfconv.getUTF8Signature();
        ::fwrite(signature, ::strlen(signature), 1, res->getFile());
    }

    int i, number;
    char utf_s[BUF_SIZE];
    paramid_t id = 0;
    const ConfigItem * item;
    for(i = 0, number = pack->layout.GetNumItems(); i < number; ++i) {
        item = pack->layout.getConfigItem(i);
        if(item->param)
            saveParam(pack, id = item->content.id);
        else {
            utfconv.fromASCIIToUTF8(item->content.s, utf_s, BUF_SIZE);
            ::strcat(utf_s, "\n");
            ::fwrite(utf_s, ::strlen(utf_s), 1, res->getFile());
        }
    }

    for(++id, number = pack->getNumParams(); id < number; ++id)
        saveParam(pack, id);
}

void ParamPackFactory::save(IParamPack * ppack, const char * name) {
    checkLocalization();

    const char * resName = name ? name : ((ParamPack *) ppack)->resName.c_str();
    ICore* icore = SingleCore::getInstance();
    res = icore->getResource(resName, "wt");
    if (!res) throw Exception("ccor : cannot open resource \"%s\" for writing", resName);

    saveToFile((ParamPack *) ppack);

    res->release();
    res = NULL;

    ((ParamPack *) ppack)->lastModified = icore->getResourceTime(resName);
    ((ParamPack *) ppack)->changed = false;
}

void ParamPackFactory::update(IParamPack * ppack) {
    checkLocalization();

    ParamPack * paramPack = (ParamPack *) ppack;
    ICore* icore = SingleCore::getInstance();
    time_t lastModified = icore->getResourceTime(paramPack->resName.c_str());

    if(paramPack->lastModified < lastModified) {
        paramPack->clear();
        //paramPack->add("$.tmp$");
        paramPack->params.push_back(new ObjParam("$.tmp$"));

//      res = icore->getResource(paramPack->resName.c_str(), "rt");

        loadFromFile(paramPack, paramPack->resName.c_str(), StringSet());

//      res->release();
//      res = NULL;

        paramPack->lastModified = lastModified;
        paramPack->changed = false;
    }
    else
        if(paramPack->changed) {
            res = icore->getResource(paramPack->resName.c_str(), "wt");

            saveToFile(paramPack);

            res->release();
            res = NULL;

            paramPack->lastModified = icore->getResourceTime(paramPack->resName.c_str());
            paramPack->changed = false;
        }
}

inline void ParamPackFactory::checkLocalization() {
    assert(!::stricmp(PRODUCT_LOCALIZATION, "ru"));
}

}
