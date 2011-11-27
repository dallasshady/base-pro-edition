
#include "headers.h"
#include "gui.h"

Color xmlColor(TiXmlNode* node, const char* attributeName)
{
    assert( node->Type() == TiXmlNode::ELEMENT );
    const char* value = static_cast<TiXmlElement*>( node )->Attribute( attributeName );
    assert( value );
    
    StringL rgba;
    strseparate( value, ",", rgba );

    Vector4f color( 0,0,0,1 );
    unsigned int i = 0;
    for( StringI stringI = rgba.begin(); stringI != rgba.end(); stringI++, i++ )
    {
    	color[i] = float( atof( stringI->c_str() ) );
    }

    return wrap( color );
}

RECT xmlRect(TiXmlNode* node, const char* attributeName)
{
    assert( node->Type() == TiXmlNode::ELEMENT );
    const char* value = static_cast<TiXmlElement*>( node )->Attribute( attributeName ); assert( value );
    assert( value );

    StringL ltrb;
    strseparate( value, ",", ltrb );
    assert( ltrb.size() == 4 );

    RECT result;
    StringI stringI = ltrb.begin();
    result.left = atoi( stringI->c_str() ); stringI++;
    result.top = atoi( stringI->c_str() ); stringI++;
    result.right = atoi( stringI->c_str() ); stringI++;
    result.bottom = atoi( stringI->c_str() );
    return result;
}

Texture* xmlTexture(TiXmlNode* node, const char* attributeName)
{
    assert( node->Type() == TiXmlNode::ELEMENT );
    const char* value = static_cast<TiXmlElement*>( node )->Attribute( attributeName );
    assert( value );

    TextureI textureI = Texture::textures.find( value );
    assert( textureI != Texture::textures.end() );
    return textureI->second;
}

gui::AlignmentType xmlVerticalAlignment(TiXmlNode* node, const char* attributeName)
{
    assert( node->Type() == TiXmlNode::ELEMENT );
    const char* value = static_cast<TiXmlElement*>( node )->Attribute( attributeName );
    assert( value );
    
    assert( strlen( value ) == 2 );
    switch( value[0] )
    {
    case 'c': case 'C': return gui::atCenter;
    case 't': case 'T': return gui::atTop;
    case 'b': case 'B': return gui::atBottom;
    }
    assert( !"Invalid alignment symbol!" );
    return gui::atCenter;
}

gui::AlignmentType xmlHorizontalAlignment(TiXmlNode* node, const char* attributeName)
{
    assert( node->Type() == TiXmlNode::ELEMENT );
    const char* value = static_cast<TiXmlElement*>( node )->Attribute( attributeName );
    assert( value );
    
    assert( strlen( value ) == 2 );
    switch( value[1] )
    {
    case 'c': case 'C': return gui::atCenter;
    case 'l': case 'L': return gui::atLeft;
    case 'r': case 'R': return gui::atRight;
    }
    assert( !"Invalid alignment symbol!" );
    return gui::atCenter;
}