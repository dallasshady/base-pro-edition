
#include "headers.h"
#include "currenttime.h"
#include "../shared/ccor.h"

bool isGreaterTime(SYSTEMTIME* s1, SYSTEMTIME* s2)
{
    if( s1->wYear > s2->wYear ) return true;
    else if( s1->wYear < s2->wYear ) return false;
    else
    {
        if( s1->wMonth > s2->wMonth ) return true;
        else if( s1->wMonth < s2->wMonth ) return false;
        else 
        {
            if( s1->wDay > s2->wDay ) return true;
            else if( s1->wDay < s2->wDay ) return false;
            else
            {
                if( s1->wHour > s2->wHour ) return true;
                else if( s1->wHour < s2->wHour ) return false;
                else
                {
                    if( s1->wMinute > s2->wMinute ) return true;
                    else if( s1->wMinute < s2->wMinute ) return false;
                    else 
                    {
                        if( s1->wSecond > s2->wSecond ) return true;
                        else if( s1->wSecond < s2->wSecond ) return false;
                        else
                        {
                            if( s1->wMilliseconds > s2->wMilliseconds ) return true;
                            else return false;
                        }
                    }
                }
            }
        }
    }
}

void getLatestFileTimeR(std::string path, SYSTEMTIMEBOOL* latestFileTime)
{
    WIN32_FIND_DATA fileFindData;
    HANDLE          fileFindHandle;
    SYSTEMTIME      referenceFileTime;
    SYSTEMTIME      fileTime;    
    
    std::string mask = path + "*.*";
    fileFindHandle = FindFirstFile( mask.c_str(), &fileFindData );
    if( fileFindHandle != INVALID_HANDLE_VALUE )
    {
        do
        {
            // update latest file time
            FileTimeToSystemTime( &fileFindData.ftLastAccessTime, &referenceFileTime );
            SystemTimeToTzSpecificLocalTime( NULL, &referenceFileTime, &fileTime );
            if( latestFileTime->second )
            {
                if( isGreaterTime( &fileTime, &latestFileTime->first ) )
                {
                    latestFileTime->first = fileTime;
                }
            }
            else
            {
                latestFileTime->first = fileTime;
                latestFileTime->second = true;
            }

            // go through subfolders
            if( fileFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
            {
                std::string subfolderPath = path + "\\" + fileFindData.cFileName;
                getLatestFileTimeR( subfolderPath, latestFileTime );
            }
        }
        while( FindNextFile( fileFindHandle, &fileFindData ) );

        FindClose( fileFindHandle );
    }
}

bool getLatestFileTimeA(std::string path, SYSTEMTIME* systemTime)
{
    SYSTEMTIMEBOOL result;
    result.second = false;

    getLatestFileTimeR( path, &result );
    
    if( result.second ) (*systemTime) = result.first;    
    return result.second;
}

bool getLatestFileTimeB(SYSTEMTIME* systemTime)
{
    SYSTEMTIME temp;
    bool       result;
    
    char currentDirectory[MAX_PATH];
    GetCurrentDirectory( MAX_PATH, currentDirectory );

    std::string currentDrive = "X:\\";
    currentDrive[0] = currentDirectory[0];
    ccor::getCore()->logMessage( "Detected current drive: %s", currentDrive.c_str() );

    result = getLatestFileTimeA( currentDrive, &temp );
    result = result || getLatestFileTimeA( "c:\\", &temp );

    if( result )
    {
        (*systemTime) = temp;
        ccor::getCore()->logMessage( 
            "Detected latest system time: %d.%d.%d %d:%d:%d",
            temp.wDay,
            temp.wMonth,
            temp.wYear,
            temp.wHour,
            temp.wMinute,
            temp.wSecond
        );
    }
    return result;
}