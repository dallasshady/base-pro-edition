
#include "headers.h"
#include "../shared/ccor.h"
#include "../shared/product_version.h"
#include "../shared/language.h"
#include "../gameplay/version.h"
#include <string>
using namespace std;


using namespace ccor;

/**
 * unicode text processor, 
 * search & replace sequences like "\n" of "\"" by same unicode strings
 */

static void processUnicodeFormatting(std::wstring& unicodeString)
{
    std::wstring result;
    unsigned int i = 0;
    while( i < unicodeString.length() )
    {
        if( i < unicodeString.length() - 1 )
        {            
            if( unicodeString[i] == '\\' && unicodeString[i+1] == 'n' )
            {
                result += L"\n";
                i += 2;
            }
            else if( unicodeString[i] == '\\' && unicodeString[i+1] == '\"' )
            {
                result += L"\"";
                i += 2;
            }
            else
            {
                result += unicodeString[i];
                i++;
            }
        }
        else
        {
            result += unicodeString[i];
            i++;
        }
    }
    unicodeString = result;
}

TiXmlElement* getConfigElement(const char* name)
{
    TiXmlNode* child = m_config->FirstChild(); assert( child );
    if( child != NULL ) do 
    {
        if( child->Type() == TiXmlNode::ELEMENT && strcmp( child->Value(), name ) == 0 )
        {
            return static_cast<TiXmlElement*>( child );
        }
        child = child->NextSibling();
    }
    while( child != NULL );
    return NULL;
}

/**
 * ILanguage implementation
 */
class Language : public EntityBase, 
                 virtual public language::ILanguage
{
private:
    std::vector<std::wstring> _unicodeStrings;
public:
    Language() {} 
    ~Language() {}
public:
    // language builder
    void buildUnicodeStrings(unsigned int dataSize, void* data)
    {       
        // prepare to parse strings
        assert( dataSize % 2 == 0 );
        unsigned int numUnicodeChars = dataSize / 2;
        wchar_t* buffer = (wchar_t*)( data );

        // zero string
        std::wstring unicodeString;
        unicodeString.clear();
        _unicodeStrings.push_back( unicodeString );

        // parse        
        unsigned int pos = 1; /* (skip unicode header at pos==0) */
        while( pos < numUnicodeChars )
        {
            wchar_t unicodeChar = buffer[pos];
            if( unicodeChar == 0x000D )
            {
                // preprocess unicode string to be valid for formatting
                processUnicodeFormatting( unicodeString );
                // add string to string list
                _unicodeStrings.push_back( unicodeString );
                unicodeString.clear();
                pos++; /* (means sequence of 0x000D 0x000A in text file) */
            }
            else
            {
                unicodeString += unicodeChar;
            }
            pos++;
        }

        // parse last string
        if( unicodeString.length() )
        {
            // preprocess unicode string to be valid for formatting
            processUnicodeFormatting( unicodeString );
            // add string to string list
            _unicodeStrings.push_back( unicodeString );
        }
    }
public:
    // EntityBase
    virtual void __stdcall entityInit(Object * p) 
    {
		    // load config
			m_config = new TiXmlDocument( "./cfg/config.xml" );
			bool configIsLoaded = m_config->LoadFile();
			assert( configIsLoaded );

			// Check config for users language
			TiXmlElement* details = getConfigElement( "details" ); assert( details );  
			int langID;
			details->Attribute( "language", &langID );

			// Strings to make path to correct language file
			char *slanguagefile = "language.txt";
			char *path = "./lng/";

			// load specific language depending on the language selected
			if (langID == 0)
			{
				 // eng
				FILE* file = fopen( "./lng/english.txt", "rb" );
				if( !file ) throw Exception( "External language file was not found!" );
				// read resource
				fseek( file, 0, SEEK_END );
				unsigned int dataSize = ftell( file );
				char* data = new char[dataSize];
				fseek( file, 0, SEEK_SET );
				fread( data, dataSize, 1, file );
				fclose( file );
				// build strings
				buildUnicodeStrings( dataSize, data );
				delete[] data;
				unsigned int numStrings = _unicodeStrings.size();
			}
			if (langID == 1)
			{
				// ru
				FILE* file = fopen( "./lng/russian.txt", "rb" );
				if( !file ) throw Exception( "External language file was not found!" );
				// read resource
				fseek( file, 0, SEEK_END );
				unsigned int dataSize = ftell( file );
				char* data = new char[dataSize];
				fseek( file, 0, SEEK_SET );
				fread( data, dataSize, 1, file );
				fclose( file );
				// build strings
				buildUnicodeStrings( dataSize, data );
				delete[] data;
				unsigned int numStrings = _unicodeStrings.size();
			}
			if (langID == 2)
			{
				// pl
				FILE* file = fopen( "./lng/polish.txt", "rb" );
				if( !file ) throw Exception( "External language file was not found!" );
				// read resource
				fseek( file, 0, SEEK_END );
				unsigned int dataSize = ftell( file );
				char* data = new char[dataSize];
				fseek( file, 0, SEEK_SET );
				fread( data, dataSize, 1, file );
				fclose( file );
				// build strings
				buildUnicodeStrings( dataSize, data );
				delete[] data;
				unsigned int numStrings = _unicodeStrings.size();
			}
			if (langID == 3)
			{
				// de
				FILE* file = fopen( "./lng/deutsch.txt", "rb" );
				if( !file ) throw Exception( "External language file was not found!" );
				// read resource
				fseek( file, 0, SEEK_END );
				unsigned int dataSize = ftell( file );
				char* data = new char[dataSize];
				fseek( file, 0, SEEK_SET );
				fread( data, dataSize, 1, file );
				fclose( file );
				// build strings
				buildUnicodeStrings( dataSize, data );
				delete[] data;
				unsigned int numStrings = _unicodeStrings.size();
			}
			// add strings together
			//strcat("./lng", slanguagefile);

			// load language text file
            //FILE* file = fopen( "./lng/language.txt", "rb" );
            //if( !file ) throw Exception( "External language file was not found!" );
            //// read resource
            //fseek( file, 0, SEEK_END );
            //unsigned int dataSize = ftell( file );
            //char* data = new char[dataSize];
            //fseek( file, 0, SEEK_SET );
            //fread( data, dataSize, 1, file );
            //fclose( file );
            //// build strings
            //buildUnicodeStrings( dataSize, data );
            //delete[] data;
            //unsigned int numStrings = _unicodeStrings.size();        
    }
    virtual void __stdcall entityAct(float dt) 
    {
    }
    virtual IBase* __stdcall entityAskInterface(iid_t id) 
    { 
        if( id == language::ILanguage::iid ) return this;
        return NULL;
    }
    static EntityBase* creator() 
    { 
        return new Language; 
    }
    virtual void __stdcall entityDestroy() 
    { 
        delete this; 
    }
    virtual unsigned int __stdcall getNumStrings(void)
    {
        return _unicodeStrings.size();
    }    
    virtual const wchar_t* __stdcall getUnicodeString(unsigned int stringId)
    {
        if( stringId >= _unicodeStrings.size() ) 
        {
            return L"";
        }
        else
        {
            return _unicodeStrings[stringId].c_str();
        }
    }
    virtual unsigned int __stdcall addUnicodeString(const wchar_t* string)
    {
        unsigned int result = _unicodeStrings.size();
        
        // parse string
        std::wstring unicodeString = string;
        if( unicodeString.length() )
        {
            // preprocess unicode string to be valid for formatting
            processUnicodeFormatting( unicodeString );
            // add string to string list
            _unicodeStrings.push_back( unicodeString );
        }
        else
        {
            _unicodeStrings.push_back( L"" );
        }

        return result;
    }
    virtual const wchar_t* __stdcall getVersionString(void)
    {
        return ::version.getVersionString();
    }
    virtual void __stdcall reset(void)
    {
        for( unsigned int i=0; i<_unicodeStrings.size(); i++ )
        {
            _unicodeStrings[i] = L"";
        }
    }
};

SINGLE_ENTITY_COMPONENT(Language);
