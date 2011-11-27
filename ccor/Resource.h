#ifndef HD50161DF_A28D_44FC_96A2_4B341D405044
#define HD50161DF_A28D_44FC_96A2_4B341D405044

#ifndef MAX_PATH
#define MAX_PATH 260
#endif

namespace ccor {

class Resource : public IResource {
protected:

    FILE * file;
    std::string name;

    Resource(const char * resName) : file(NULL), name(resName) { }

    ~Resource();

	void openTempFile(bool textMode);

public:

    virtual FILE * __stdcall getFile() { return file; }

    virtual const char * __stdcall getName() { return name.c_str(); }

    virtual void __stdcall release() { delete this; }
};

typedef std::vector<char> MemFileBuffer;

class MemFile {
public:

	MemFileBuffer buf;
	time_t lastModified;

    MemFile() : lastModified(::time(NULL)) { }
};
class FileReader : public Resource {

friend class ResourceMgr;

private:

    FileReader(const char * fname, bool textMode);

	bool openZipFile(const char * zipName, const char * fileName);

    ~FileReader() { }
};

class FileWriter : public Resource {

friend class ResourceMgr;

private:

    FileWriter(const char * fname, bool textMode) : Resource(fname) {
       	if(!openFile(fname, textMode))
	    	throw Exception("ccor : cannot open file \"%s\"", fname);
    }

    ~FileWriter() { }

	bool openFile(const char * fname, bool textMode);
};

class MemFileReader : public Resource {

friend class ResourceMgr;

private:

	MemFileReader(MemFile * memFile, bool textMode, const char * name);

    ~MemFileReader() { }
};

class MemFileWriter : public Resource {

friend class ResourceMgr;

private:

    MemFile * memFile;
    bool textMode;

    MemFileWriter(MemFile * memFile, bool textMode, const char * name) : Resource(name) {
	    this->memFile = memFile;
        this->textMode = textMode;

	    openTempFile(textMode);
    }

    ~MemFileWriter();
};

typedef std::map<std::string, MemFile *> MemFileMap;
typedef std::map<std::string, std::string> PathMap;

class ResourceMgr {
private:

    MemFileMap memFileMap;
    PathMap pathMap;

    IResource * getResourceReader(const char * resName, bool textMode);

    IResource * getResourceWriter(const char * resName, bool textMode);

public:

    ResourceMgr() { }

    ~ResourceMgr();

    void loadPathMap(const char * fileName);

	MemFile * addMemFile(const char * name);

	void removeMemFile(const char * name);

	MemFile * getMemFile(const char * name);

    char * ResourceMgr::getFullPath(char * fullPath, const char * fname, const char * type);

    IResource * getResource(const char * resName, const char * mode, const char * resType = NULL);

	time_t getLastModified(const char * resName);
};

}

#endif
