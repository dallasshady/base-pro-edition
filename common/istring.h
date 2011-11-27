/**
 * This source code is a part of D3 game project
 * (c) Digital Dimension Development 2004-2005
 *
 * @description string utilites
 *
 * @author bad3p
 */

#if !defined(ISTRING_INCLUDED)
#define ISTRING_INCLUDED

#include <string>
#include <list>
#include <map>
#include <vector>

// format string using format specifiers & arguments
static inline std::string strformat(const char* str, ...)
{
    char buf[1024];
    va_list args;
    va_start(args,str);
    vsprintf(buf,str,args);
    va_end(args);
    return std::string(buf);
}

// format unicode string using format specifiers & arguments
static inline std::wstring wstrformat(const wchar_t* str, ...)
{
    wchar_t buf[1024];
    va_list args;
    va_start(args,str);
    vswprintf(buf,str,args);
    va_end(args);
    return std::wstring(buf);
}

typedef std::pair<std::string,std::string> StringP;
typedef std::map<std::string,std::string> StringM;
typedef std::vector<std::string> StringV;
typedef std::list<std::string> StringL;
typedef StringL::iterator StringI;

// separate string using specified separate sequence
// @return separation result as a string pair, separation sequence is excluded
static inline StringP& strseparate(    
    const std::string& str, 
    const std::string& separator,
    StringP& result )
{
    const char* temp = strstr( str.c_str(), separator.c_str() );
    // "str" does not contains separator?
    if( !temp )
    {
        result.first = str;
        result.second = "";
        return result;
    }
    // "str" contains separator
    result.second = temp + separator.length();
    result.first = "";
    if( result.second.size() < str.length() )
    {
        result.first.append( str.c_str(), str.length() - result.second.size() - separator.length() );
    }
    return result;
}

// separate string using specified separate sequence
// @return separation result as a string list, separation sequences are excluded
static inline StringL& strseparate(
    const std::string& str, 
    const std::string& separator, 
    StringL& result )
{    
    std::string tempS;
    StringP     tempSP;
 
    result.clear();
    tempS = str;
    while( tempS != "" )
    {
        strseparate( tempS, separator, tempSP );
        result.push_back( tempSP.first );
        tempS = tempSP.second;
    }
    return result;
}

// convert each slash symbol to backslash in specified string
static inline std::string strbackslash(const std::string& str)
{
    std::string result = str;
    for( unsigned int i=0; i<result.length(); i++ )
    {
        if( result[i] == '\\' ) result[i] = '/';
    }
    return result;
}

// extract pure path from full path using "/" separation symbol
static inline std::string expath(const std::string& fullpath) 
{
    StringL tempSL;
    strseparate( fullpath, "/", tempSL );
    if( !tempSL.size() ) return "";
    StringI lastI = tempSL.end(); lastI--;
    tempSL.erase( lastI );

    std::string result = "";
    for( StringI stringI = tempSL.begin();
                 stringI != tempSL.end();
                 stringI++ 
       )
    {
        result += *stringI, result += "/";
    }
    return result;
}

// extract file name from full path
static inline std::string exname(const std::string& fullpath)
{
    StringL tempSL;    
    strseparate( fullpath, "/", tempSL );
    if( !tempSL.size() ) return "";

    StringI tempSI = tempSL.end();
    tempSI--;
    StringP tempSP;
    strseparate( *tempSI, ".", tempSP );

    return tempSP.first;
}

// search for specified string within string list
static inline StringI strsearch(StringL& list, const std::string& str)
{
    for( StringI stringI = list.begin(); 
                 stringI != list.end(); 
                 stringI++ )
    {
        if( *stringI == str ) return stringI;
    }
    return list.end();
}

// removes specified leading and trailing characters from a string
static inline std::string strtrim(const std::string& s, const std::string& t)
{
    int k;

    // trim left
	unsigned int trimPos = 0;
    for( unsigned i=0; i<s.length(); i++, trimPos++ ) 
    {
        k = 0;
        for( unsigned j=0; j<t.length(); j++ ) if( s[i] == t[j] ) k++;
        if( k == 0 ) break;
    }
    std::string result = s.c_str()+trimPos;

    // trim right
    if( result.length() ) 
    {
        do
        {
            k = 0;
            for( unsigned j=0; j<t.length(); j++ ) if( result[result.length()-1] == t[j] ) k++;
            if( k ) result.erase( result.end() - 1 );
        }
        while( k );
    }
    return result;
}

static bool strIsNumber(const char* str)
{
    int length = int( strlen( str ) );
    for( int i=0; i<length; i++ )
    {
        if( ( str[i] < '0' || str[i] > '9' ) && 
            ( str[i] != '.' && str[i] != '-' && str[i] != 'e' ) )
        {
            return false;
        }
    }
    return true;
}

static bool strIsUnsignedInt(const char* str)
{
    int length = int( strlen( str ) );
    for( int i=0; i<length; i++ )
    {
        if( str[i] < '0' || str[i] > '9' ) return false;
    }
    return true;
}

#endif
