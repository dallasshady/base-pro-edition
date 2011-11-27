
#ifndef OGG_FILES_INCLUDED
#define OGG_FILES_INCLUDED

#include "headers.h"

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
    ALuint getSize(void);
    ALuint getNumChannels(void);
    ALsizei getSamplingRate(void);
    void readBlock(void* buffer, unsigned int size);
    void reset(void);
    const char* getFileName(void);
};

#endif