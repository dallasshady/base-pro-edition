#include <io.h>
#include <fcntl.h>
#include <direct.h>
#include "headers.h"
#include "../shared/ccor.h"
#include "CoreImpl.h"
#include "zlib/unzip.h"
#include "Resource.h"

#define BUF_SIZE 4096

#define MEM_FILE_PREFIX "mem:"

#define ZIP_FILE_SUFFIX ".zip"

namespace ccor {

Resource::~Resource() {
    if(file)
        ::fclose(file);
}

void Resource::openTempFile(bool textMode) {
    char fname[MAX_PATH];
    ::tmpnam(fname);
    ::strcat(fname, "~tmp");
    char mode[] = { 'w', '+', textMode ? 't': 'b', 'T', 'D', '\0' };
    if(!(file = ::fopen(fname[0] == '\\' || fname[0] == '/' ? fname + 1 : fname, mode)))
        throw Exception("resmgr: cannot open temporary file");
}

FileReader::FileReader(const char * fname, bool textMode) : Resource(fname) {
    if(!(file = ::fopen(fname, textMode ? "rt" : "rb"))) {
        openTempFile(false);

        char zipName[MAX_PATH];
        char zipedFileName[MAX_PATH];
        int zipNameLen;
        const char * separator = fname + ::strlen(fname);
        do {
            do
                if(--separator == fname)
                    break;
            while(*separator != '/');

            if(separator > fname) {
                zipNameLen = separator - fname;
                ::strncpy(zipName, fname, zipNameLen);
                ::strcpy(zipName + zipNameLen, ZIP_FILE_SUFFIX);
                ::strcpy(zipedFileName, separator + 1);
                if(openZipFile(zipName, zipedFileName))
                    break;
            }
        }
        while(separator > fname);
        if(separator == fname)
            throw Exception("resmgr: cannot find file \"%s\"", fname);

        ::rewind(file);
        if(textMode)
            ::setmode(_fileno(file), _O_TEXT);
    }
}

bool FileReader::openZipFile(const char * zipName, const char * zipedFileName) {
    unzFile zip;
    if(zip = unzOpen(zipName))
        if(unzLocateFile(zip, zipedFileName, 2) == UNZ_OK) {
            if(unzOpenCurrentFile(zip) != UNZ_OK)
                throw Exception("resmgr: cannot open compressed file \"%s\"", zipedFileName);

            int ret;
            char buf[BUF_SIZE];
            while((ret = unzReadCurrentFile(zip, buf, BUF_SIZE)) > 0)
                if(!::fwrite(buf, ret, 1, file))
                    throw Exception("resmgr: error occured while writing from zip to temporary file");

            unzCloseCurrentFile(zip);
            unzClose(zip);

            return true;
        }
    else
        unzClose(zip);

    return false;
}

MemFileReader::MemFileReader(MemFile * memFile, bool textMode, const char * name) : Resource(name) {

    openTempFile(false);

    if(!::fwrite(&*memFile->buf.begin(), memFile->buf.size(), 1, file))
        throw Exception("resmgr: error occured while writing from mem file to temporary file");

    ::rewind(file);
    if(textMode)
        ::setmode(_fileno(file), _O_TEXT);
}

bool FileWriter::openFile(const char * fname, bool textMode) {
    char path[MAX_PATH];
    ::strcpy(path, fname);
    char * separator = path;
    _finddata_t findData;
    long findHandle;
    bool dirExists = true;
    while(separator = ::strchr(separator, '/')) {
        *separator = '\0';

        if(dirExists) {
            if((findHandle = ::_findfirst(path, &findData)) == -1)
                dirExists = false;
            else {
                while(!(findData.attrib & _A_SUBDIR))
                    if(_findnext(findHandle, &findData) == -1)
                        break;
                if(!(findData.attrib & _A_SUBDIR))
                    dirExists = false;

                _findclose(findHandle);
            }
        }

        if(!dirExists)
            ::mkdir(path);

        *separator++ = '/';
    }

    return (file = ::fopen(fname, textMode ? "wt" : "wb")) ? true : false;
}

MemFileWriter::~MemFileWriter() {
    char ch;
    ::rewind(file);
    if(textMode)
        ::setmode(_fileno(file), _O_BINARY);
    while(::fread(&ch, 1, 1, file))
        memFile->buf.push_back(ch);
    memFile->lastModified = ::time(NULL);
}

ResourceMgr::~ResourceMgr() {
    for(MemFileMap::iterator it = memFileMap.begin(); it != memFileMap.end(); ++it)
        delete it->second;
}

void ResourceMgr::loadPathMap(const char * fileName) {

    IParamPack * ppack = SingleCore::getInstance()->getParamPackFactory()->load(fileName);

	unsigned int size = ppack->findParams("*");
    paramid_t* ids = new paramid_t[ size ];
	ppack->copyParamResult( ids );

    for( unsigned int i=0; i<size; ++i )
	{
        pathMap[ppack->getName(ids[i])] = ppack->gets(ids[i]);
	}
	delete[] ids;

    ppack->release();
}

char * ResourceMgr::getFullPath(char * fullPath, const char * fname, const char * type) {
/*
    for(int pos = ::strlen(fname); pos > 0; --pos) {
        if(fname[pos] == '.' || fname[pos] == '\\' || fname[pos] == '/')
            break;
    }
    if(fname[pos] == '.') {
        PathMap::iterator it = pathMap.find(fname + pos);
        if(it != pathMap.end()) {

            ::strcpy(fullPath, it->second.c_str());
            fullPath[it->second.length()] = '\\';
            fullPath[it->second.length() + 1] = '\0';
        }
    }
*/
    fullPath[0] = '\0';

    if(type) {

        PathMap::iterator it = pathMap.find(type);
        if(it != pathMap.end()) {

            ::strcpy(fullPath, it->second.c_str());
            fullPath[it->second.length()] = '/';
            fullPath[it->second.length() + 1] = '\0';
        }
    }
    ::strcat(fullPath, fname);
/*
    for(pos = 0; fullPath[pos]; ++pos)
        if(fullPath[pos] == '\\')
            fullPath[pos] = '/';
*/
    return fullPath;
}

IResource * ResourceMgr::getResourceReader(const char * resName, bool textMode) {
    if(!::strncmp(resName, MEM_FILE_PREFIX, ::strlen(MEM_FILE_PREFIX))) {
        MemFileMap::iterator it = memFileMap.find(resName);
        if(it == memFileMap.end())
            throw Exception("resmgr: cannot find mem file \"%s\"", resName);
        return new MemFileReader(it->second, textMode, resName);
    }

    return new FileReader(resName, textMode);
}

IResource * ResourceMgr::getResourceWriter(const char * resName, bool textMode) {
    if(!::strncmp(resName, MEM_FILE_PREFIX, ::strlen(MEM_FILE_PREFIX)))
        return new MemFileWriter(addMemFile(resName), textMode, resName);

    return new FileWriter(resName, textMode);
}

IResource * ResourceMgr::getResource(const char * resName, const char * mode, const char * resType) {

    const char * readMode = ::strchr(mode, 'r');
    const char * writeMode = ::strchr(mode, 'w');
    const char * textMode = ::strchr(mode, 't');
    const char * binaryMode = ::strchr(mode, 'b');

    char fullPath[MAX_PATH] = "";
    getFullPath(fullPath, resName, resType);

    if(readMode && writeMode || !readMode && !writeMode || textMode && binaryMode) {

        SingleCore::getInstance()->logMessage("resmgr: unallowed type of access \"%s\" to \"%s\"", mode, resName);
        return NULL;
    }

    try {

        if(readMode)
            return getResourceReader(fullPath, binaryMode == NULL);
        else
            return getResourceWriter(fullPath, binaryMode == NULL);
    }
    catch(Exception exception) {

        SingleCore::getInstance()->logMessage(exception);
        return NULL;
    }
}

time_t ResourceMgr::getLastModified(const char * resName) {
    if(!::strncmp(resName, MEM_FILE_PREFIX, ::strlen(MEM_FILE_PREFIX))) {
        MemFile * memFile = getMemFile(resName);
        if(memFile)
            return memFile->lastModified;
    }
    else {
        _finddata_t findData;
        long findHandle = ::_findfirst(resName, &findData);
        if(findHandle != -1) {
            while(findData.attrib & _A_SUBDIR)
                if(_findnext(findHandle, &findData) == -1)
                    break;

            _findclose(findHandle);

            if(!(findData.attrib & _A_SUBDIR))
                return findData.time_write;
        }
    }

    return 0;
}

MemFile * ResourceMgr::addMemFile(const char * name) {
    MemFileMap::iterator it = memFileMap.find(name);
    if(it == memFileMap.end())
        return memFileMap[name] = new MemFile();
    return it->second;
}

void ResourceMgr::removeMemFile(const char * name) {
    MemFileMap::iterator it = memFileMap.find(name);
    if(it == memFileMap.end())
        throw Exception("resmgr: cannot find mem file \"%s\"", name);
    delete it->second;
    memFileMap.erase(it);
}

MemFile * ResourceMgr::getMemFile(const char * name) {
    MemFileMap::iterator it = memFileMap.find(name);
    if(it == memFileMap.end())
        return NULL;
    return it->second;
}

}
