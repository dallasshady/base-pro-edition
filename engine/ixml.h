
#ifndef XML_INTELLIGENCE_INCLUDED
#define XML_INTELLIGENCE_INCLUDED

#include "tinyxml.h"
#include "../shared/vector.h"
#include "../shared/matrix.h"
#include "../common/istring.h"

static inline TiXmlElement* findChildElement(
    TiXmlNode* root, 
    const char* nodeName
)
{
    assert( root );
    
    TiXmlNode* node = root->FirstChild();
    if( node ) do
    {
        if( node->Type() == TiXmlNode::ELEMENT && stricmp( node->Value(), nodeName ) == 0 )            
        {
            return static_cast<TiXmlElement*>( node );
        }
        node = node->NextSibling();
    }
    while( node != NULL );
    return NULL;
}

static inline TiXmlElement* findChildElement(
    TiXmlNode* root, 
    const char* nodeName, 
    const char* attributeName, 
    const char* attributeValue
)
{
    assert( root );
    
    TiXmlNode* node = root->FirstChild();
    if( node ) do
    {
        if( node->Type() == TiXmlNode::ELEMENT && 
            stricmp( node->Value(), nodeName ) == 0 && 
            strcmp( 
                static_cast<TiXmlElement*>( node )->Attribute( attributeName ), 
                attributeValue
            ) == 0 )
        {
            return static_cast<TiXmlElement*>( node );
        }
        node = node->NextSibling();
    }
    while( node != NULL );
    return NULL;
}

static inline TiXmlElement* createChildElement(
    TiXmlNode* root, 
    const char* nodeName,
    bool infront
)
{
    TiXmlElement* element = new TiXmlElement( nodeName );
    TiXmlNode* result;
    if( infront )
    {
        TiXmlNode* firstChild = root->FirstChild();
        if( firstChild )
        {
            result = root->InsertBeforeChild( firstChild, *element );
        }
        else
        {
            result = root->InsertEndChild( *element );
        }
    }
    else
    {
        result = root->InsertEndChild( *element );
    }
    delete element;
    return static_cast<TiXmlElement*>( result );
}

static inline Vector2f strFlector(const char* string)
{
    StringL strings;
    StringI stringI;
    Vector2f result( 0,0 );
    unsigned int i;
    
    strseparate( string, ",", strings );
    for( stringI = strings.begin(), i = 0; 
         stringI != strings.end(); 
         stringI++, i++ )
    {
        result[i] = float( atof( stringI->c_str() ) );
        if( i == 1 ) break;
    }

    return result;
}

static inline Matrix4f xmlMatrix(TiXmlNode* node)
{
    assert( stricmp( node->Value(), "matrix" ) == 0 );

    StringL strings;
    StringI stringI;
    unsigned int i;
    Matrix4f result( 1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1 );

    i = 0; 
    strseparate( static_cast<TiXmlElement*>( node )->Attribute( "right" ), ",", strings );
    for( stringI=strings.begin(); stringI!=strings.end(); stringI++, i++ )
    {
        result[0][i] = float( atof( stringI->c_str() ) );
    }

    i = 0;
    strings.clear();
    strseparate( static_cast<TiXmlElement*>( node )->Attribute( "up" ), ",", strings );
    for( stringI=strings.begin(); stringI!=strings.end(); stringI++, i++ )
    {
        result[1][i] = float( atof( stringI->c_str() ) );
    }

    i = 0;
    strings.clear();
    strseparate( static_cast<TiXmlElement*>( node )->Attribute( "at" ), ",", strings );
    for( stringI=strings.begin(); stringI!=strings.end(); stringI++, i++ )
    {
        result[2][i] = float( atof( stringI->c_str() ) );
    }

    i = 0;
    strings.clear();
    strseparate( static_cast<TiXmlElement*>( node )->Attribute( "pos" ), ",", strings ); assert( strings.size() == 3 );
    for( stringI=strings.begin(); stringI!=strings.end(); stringI++, i++ )
    {
        result[3][i] = float( atof( stringI->c_str() ) );
    }

    return result;
}

#endif