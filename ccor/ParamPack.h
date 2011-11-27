/**
 * This source code is a part of Metathrone game project. 
 * (c) Perfect Play 2003.
 *
 * @description
 * Implementation of IParamPack interface
 *
 * @author Yury Egorov
 */

#ifndef HEDE20AF9_A1D1_476A_98CB_F8C050B07C7B
#define HEDE20AF9_A1D1_476A_98CB_F8C050B07C7B
#include "find_result_t.h"

#define BUF_SIZE 512

namespace ccor {

enum ParamType {

    PT_INT,
    PT_FLOAT,
    PT_BOOL,
    PT_OBJECT,
    PT_STRING,
    PT_VECTOR2,
    PT_VECTOR3,
    PT_VECTOR4

};

union ParamValue {

    int iValue;
    float fValue;
    bool bValue;
    Object * objValue;
    char * sValue;
    Vector4f * vecValue;

};

class StringPtrWrapper {
public:

    const char * s;

    StringPtrWrapper() { }
 
    StringPtrWrapper(const char * s) { this->s = s; }

    StringPtrWrapper(const std::string & s) { this->s = s.c_str(); }

    StringPtrWrapper& operator=(const StringPtrWrapper& b) {
        s = b.s;
        return * this;
    }

    bool operator==(const StringPtrWrapper& b) const { return !::stricmp(s, b.s); }

    operator const char * () const { return s; }
};

struct StringPtrWrapperComparator {
    bool operator()(const StringPtrWrapper & a, const StringPtrWrapper & b) const { return ::strcmp(a.s, b.s) < 0; }
};

class ObjParam {

friend class ParamPack;

private:

    std::string name;
    ParamType type;
    ParamValue value;

public:

    ObjParam(const char * name);

    void release();

    ~ObjParam() { release(); }
};

typedef std::vector<ObjParam*> ParamVector;
typedef std::map<StringPtrWrapper,paramid_t,StringPtrWrapperComparator> ParamMap;
typedef std::vector<ParamPackListener *> ListenerVector;
typedef std::map<wchar_t, char> UTF8Map;
typedef std::map<char, wchar_t> ASCIIMap;
typedef std::vector<std::string> StringVector;
typedef std::set<std::string> StringSet;

class UTF8Converter {
private:

    UTF8Map utftoascii;
    ASCIIMap asciitoutf;
    char signature[4];

public:

    UTF8Converter();

    int fromUTF8ToASCII(const char * utf_s, char * s, int slen);

    int fromASCIIToUTF8(const char * ascii_s, char * s, int slen);

    const char * getUTF8Signature() { return signature; }
};

union ConfigItemContent {
    char * s;
    paramid_t id;
};

class ConfigItem {

friend class ConfigLayout;
friend class ParamPackFactory;

private :

    bool param;
    ConfigItemContent content;

public:

    ConfigItem() { }

    ConfigItem(const char * s);

    ConfigItem(paramid_t id);

    void release();

    ~ConfigItem() { release(); }
};

typedef std::vector<ConfigItem *> ConfigItemVector;

class ConfigLayout : public Object {
private:

    ConfigItemVector items;

public:

    ConfigLayout() { }

    ~ConfigLayout() { clear(); }

    int GetNumItems() { return items.size(); }

    void addString(const char * s);

    void addParam(paramid_t id);

    const ConfigItem * getConfigItem(int item);

    void clear();
};

class ParamPack : public IParamPack {

friend class ParamPackFactory;

private:

    ParamMap ids;
    ParamVector params;
    char tmpbuf[BUF_SIZE];
    ListenerVector listeners;
    ConfigLayout layout;
    std::string resName;
    time_t lastModified;
    bool changed;
    std::vector<paramid_t> findBuf;
    find_param_t paramResult;

    void CheckID(paramid_t id);

    void ChangeParam(paramid_t id);

public:

    ParamPack();

    ~ParamPack(){ clear(); }

    void clear();
    
    ParamType getType(paramid_t id) {
        CheckID(id);
        return params[id]->type;
    }

    int __stdcall getNumParams() { return params.size(); }

    paramid_t __stdcall add(const char * qualifiedName);

    int __stdcall getMaxIndex(paramid_t id);

    int __stdcall getMaxIndex(const char * name) {

        paramid_t id = find(name);
        if(id != -1) return getMaxIndex(id);
        return -1;
    }

    const char * __stdcall getName(paramid_t id) {
        CheckID(id);
        return params[id]->name.c_str();
    }

    void __stdcall set(paramid_t id, int value);
    void __stdcall set(const char * name, int value) { set(add(name), value); }
    void __stdcall set(paramid_t id, float value);
    void __stdcall set(const char * name, float value) { set(add(name), value); }
    void __stdcall set(paramid_t id, bool value);
    void __stdcall set(const char * name, bool value) { set(add(name), value); }
    void __stdcall set(paramid_t id, Object * value);
    void __stdcall set(const char * name, Object * value) { set(add(name), value); }
    void __stdcall set(paramid_t id, const char * text);
    void __stdcall set(const char * name, const char * text) { set(add(name), text); }
    void __stdcall set(paramid_t id, const Vector2f & vec);
    void __stdcall set(const char * name, const Vector2f & vec) { set(add(name), vec); }
    void __stdcall set(paramid_t id, const Vector3f & vec);
    void __stdcall set(const char * name, const Vector3f & vec) { set(add(name), vec); }
    void __stdcall set(paramid_t id, const Vector4f & vec);
    void __stdcall set(const char * name, const Vector4f & vec) { set(add(name), vec); }

    void __stdcall get(paramid_t id, int & value);
    void __stdcall get(const char * name, int & value) { get(find(name), value); }
    void __stdcall get(paramid_t id, float & value);
    void __stdcall get(const char * name, float & value) { get(find(name), value); }
    void __stdcall get(paramid_t id, bool & value);
    void __stdcall get(const char * name, bool & value) { get(find(name), value); }
    void __stdcall get(paramid_t id, Object *& value);
    void __stdcall get(const char * name, Object *& value) { get(find(name), value); }
    const char * __stdcall gets(paramid_t id);
    const char * __stdcall gets(const char * name) { return gets(find(name)); }
    void __stdcall get(paramid_t id, Vector2f & vec);
    void __stdcall get(const char * name, Vector2f & vec) { get(find(name), vec); }
    void __stdcall get(paramid_t id, Vector3f & vec);
    void __stdcall get(const char * name, Vector3f & vec) { get(find(name), vec); }
    void __stdcall get(paramid_t id, Vector4f & vec);
    void __stdcall get(const char * name, Vector4f & vec) { get(find(name), vec); }

    const char * __stdcall getv(paramid_t id, const char * dv);
    const char * __stdcall getv(const char * name, const char * dv) { return getv(find(name),dv); }
    int          __stdcall getv(paramid_t id, int dv);
    int          __stdcall getv(const char * name, int dv) { return getv(find(name),dv); }
    float        __stdcall getv(paramid_t id, float dv);
    float        __stdcall getv(const char * name, float dv) { return getv(find(name),dv); }
    bool         __stdcall getv(paramid_t id, bool dv);
    bool         __stdcall getv(const char * name, bool dv) { return getv(find(name),dv); }
    Object *     __stdcall getv(paramid_t id, Object * dv);
    Object *     __stdcall getv(const char * name, Object * dv) { return getv(find(name),dv); }
    Vector2f     __stdcall getv(paramid_t id, Vector2f dv);
    Vector2f     __stdcall getv(const char * name, Vector2f dv) { return getv(find(name),dv); }
    Vector3f     __stdcall getv(paramid_t id, const Vector3f& dv);
    Vector3f     __stdcall getv(const char * name, const Vector3f& dv) { return getv(find(name),dv); }
    Vector4f     __stdcall getv(paramid_t id, const Vector4f& dv);
    Vector4f     __stdcall getv(const char * name, const Vector4f& dv) { return getv(find(name),dv); }

    void __stdcall optimizeValue(paramid_t id);
    void __stdcall optimizeValue(const char * name) { optimizeValue(find(name)); }
    void __stdcall optimizeValues();

    paramid_t __stdcall find(const char * qualifiedName);

    int __stdcall findParams(const char * qualifiedName);

    void __stdcall copyParamResult(paramid_t * array);

    void __stdcall addListener(ParamPackListener * listener);

    void __stdcall removeListener(ParamPackListener * listener);

    void __stdcall copySubsetFrom(IParamPack * pack, const char * templateName, const char * subst = 0);

    void __stdcall release() { delete this; }

    /**
     * Check whether this pack is readonly. Any attempt
     * to modify a paramter in readonly pack will cause Exception.
     * Should be implemented in future.
     */
    bool __stdcall isReadonly() { return false; }

    /**
     * Protect/unprotect this object from being modified.
     * Should be implemented in future.
     */
    void __stdcall setReadonly(bool readOnly) { }
};

class ParamPackFactory : public IParamPackFactory {

friend class ParamPack;

struct READBUF {
    char buffer[BUF_SIZE];
    int length;
    StringVector passComments;
};

private:

    UTF8Converter utfconv;
    READBUF * preadbuf;
    IResource * res;

    int getParamName(char * name, const char * src);

    void getParamValue(char * value, const char * src);

    void getParamValues(StringVector & values, const char * src);

    static int toOct(const char * s, char & ch);

    static int toHex(const char * s, char & ch);

    static int translateEscapeSequences(char * s);

    static void backTranslateEscapeSequences(char * s);

    static bool isInt(const char * s);

    static bool isFloat(const char * s);

    static void strTrim(const char * src, char * dest);

    static int strToVector(const char * s, float * vec, int maxveclen);

    static void vectorToStr(const float * vec, int veclen, char * s);

    static bool parseValue(ParamPack * pack, paramid_t id, const char * value);

    static bool isPrintable(unsigned char ch);

    static bool isComment(const char * s);

    static void getParentDir(const char * fname, char * dir);

    static void getFullParamName( char * name,
                                  const char * srcName,
                                  const std::string & prefix,
                                  const std::string & substSrc,
                                  const std::string & substDst );

    void nextLine(ParamPack * pack);

    int nextString(ParamPack * pack, char * buf, paramid_t * id);

    void loadFromFile( ParamPack * pack,
                       const char * fname,
                       StringSet & loaded,
                       const char * type = NULL,
                       const std::string & prefix = "",
                       const std::string & substSrc = "",
                       const std::string & substDst = "" );

    void saveParam(ParamPack * pack, paramid_t id);

    void saveToFile(ParamPack * pack, bool saveUTF8Signature = false);

    void checkLocalization();

public:

    virtual IParamPack * __stdcall createInstance() { return new ParamPack(); }

    virtual IParamPack * __stdcall load(const char * name, const char * type);

    virtual void __stdcall load(IParamPack * ppack, const char * name, bool append = false, const char * type = NULL);

    virtual void __stdcall save(IParamPack * pack, const char * name = NULL);

    virtual void __stdcall update(IParamPack * pack);

};

}

#endif
