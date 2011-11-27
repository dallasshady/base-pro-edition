#ifndef H73921A02_EA03_4388_9A76_1CADC6DCA503
#define H73921A02_EA03_4388_9A76_1CADC6DCA503
#include "../shared/ccor.h"

class SerializeStreamImpl : public ccor::SerializeStream {

public:

    SerializeStreamImpl(ccor::IResource * resource, bool saving);

    virtual bool __stdcall isSaving() { return _fpSave!=0; }

    virtual void __stdcall setRules(long rules) { }

    virtual void __stdcall assertVersion(long versionId);

    virtual void __stdcall setLabel(const char * label);

    virtual void __stdcall serialize(int * value, int maxElements = 1) { serializeData(value,'i',sizeof(int),maxElements); }

    virtual void __stdcall serialize(float * value, int maxElements = 1) { serializeData(value,'f',sizeof(float),maxElements); }

    virtual void __stdcall serialize(bool * value, int maxElements = 1) { serializeData(value,'b',sizeof(bool),maxElements); }

    virtual void __stdcall serialize(char * value, int maxElements = 1) { serializeData(value,'c',sizeof(char),maxElements); }

    virtual void __stdcall serialize(Vector2f * value, int maxElements = 1) { serializeData(value,'d',sizeof(Vector2f),maxElements); }

    virtual void __stdcall serialize(Vector3f * value, int maxElements = 1) { serializeData(value,'t',sizeof(Vector3f),maxElements); }

    virtual void __stdcall serialize(Vector4f * value, int maxElements = 1) { serializeData(value,'q',sizeof(Vector4f),maxElements); }

    virtual void __stdcall serializeIds(ccor::globid_t * value, int maxElements = 1) { serializeData(value,'g',sizeof(ccor::globid_t),maxElements); }

    virtual int __stdcall getNumElements();

private:

    FILE * _fpSave;
    FILE * _fpLoad;
    ccor::IResource * _resource;
    long _lastNextEntry;

    typedef std::map<std::string, long> LabelMap;
    LabelMap _labelMap;

    void serializeData(void * data, char fmt, int itemSize, int numItems);

    long findLabelInFile(const char * label);

};


#endif
