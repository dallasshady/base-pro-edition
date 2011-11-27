
#include "headers.h"
#include "ogg.h"
#include "../shared/ccor.h"

/**
 * class implementation
 */

OggFile::OggFile(const char* fileName) : _fileInfo( NULL )
{
    _fileName = fileName;
    FILE* file = ::fopen( fileName, "rb" );
    if( file == NULL )
    {
        throw ccor::Exception( "Cannot open file '%s'", fileName );
    }
    if( ::ov_open( file, &_file, NULL, 0 ) != 0 )
    {
        throw ccor::Exception( "Cannot open ogg file '%s' for decoding", fileName );
    }
    if( ::ov_seekable( &_file ) == 0 )
    {
        throw ccor::Exception( "Ogg file '%s' is seekable", fileName );
    }
    _fileInfo = ::ov_info( &_file, -1 );
}

OggFile::~OggFile()
{
    ::ov_clear( &_file );
}

/**
 * SoundFile
 */

unsigned int OggFile::getSize(void)
{
    return unsigned int( ::ov_pcm_total( &_file, -1 ) * _fileInfo->channels * 2 );
}

unsigned int OggFile::getNumChannels(void)
{
    return _fileInfo->channels;
}

unsigned int OggFile::getSamplingRate(void)
{
    return _fileInfo->rate;
}

unsigned int OggFile::readBlock(void* buffer, unsigned int size)
{
    unsigned int result = 0;
    int offset = 0, bytesRead, bitStream;
    while( size )
    {
        bytesRead = ::ov_read( &_file, ((char*)(buffer)) + offset, size, 0, 2, 1, &bitStream );
        if( bytesRead < 0 )
        {
            throw ccor::Exception( "Error while reading ogg-file \"%s\"", _fileName.c_str() );
        }
        if( bytesRead == 0 )
        {
            break;
        }
        result += bytesRead;
        offset += bytesRead;
        size -= bytesRead;
    }
    return result;
}

void OggFile::reset(void)
{
    ::ov_pcm_seek( &_file, 0 );
}

const char* OggFile::getFileName(void)
{
    return _fileName.c_str();
}