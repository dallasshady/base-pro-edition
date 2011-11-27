
#ifndef UNICODE_ROUTINE_INCLUDED
#define UNICODE_ROUTINE_INCLUDED

static inline std::wstring asciizToUnicode(const char* string)
{
    unsigned int bufferSize = strlen(string) + 1;
    wchar_t* buffer = new wchar_t[bufferSize];
    if( MultiByteToWideChar(
            CP_ACP,
            MB_PRECOMPOSED,
            string,
            bufferSize,
            buffer,
            bufferSize
    ))
    {
        std::wstring result( buffer );
        delete[] buffer;
        return result;
    }
    else
    {
        delete[] buffer;
        return L"";
    }
}

static inline std::string unicodeToAsciiz(const wchar_t* string)
{
    unsigned int bufferSize = wcslen(string) + 1;
    char* buffer = new char[bufferSize];
    if( WideCharToMultiByte(
            CP_ACP,
            WC_COMPOSITECHECK,
            string,
            bufferSize,
            buffer,
            bufferSize,
            NULL,
            NULL
    ))
    {
        std::string result( buffer );
        delete[] buffer;
        return result;
    }
    else
    {
        delete[] buffer;
        return "";
    }
}

#endif