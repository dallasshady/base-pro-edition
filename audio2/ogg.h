
#ifndef OGG_FILES_INCLUDED
#define OGG_FILES_INCLUDED

#include "headers.h"
#include "ogg/ogg.h"
#include "vorbis/vorbisfile.h"

/**
 * ogg sound file
 */

class OggFile
{
private:
    std::string    _fileName;
    OggVorbis_File _file;
    vorbis_info*   _fileInfo;
public:
    OggFile(const char* fileName);
    ~OggFile();
public:
    unsigned int getSize(void);
    unsigned int getNumChannels(void);
    unsigned int getSamplingRate(void);
    unsigned int readBlock(void* buffer, unsigned int size);
    void reset(void);
    const char* getFileName(void);
};

#endif