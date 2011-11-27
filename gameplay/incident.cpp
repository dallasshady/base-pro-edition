
#include "headers.h"
#include "fatalitylist.h"
#include "gameplay.h"
#include "crypt.h"
#include "../common/istring.h"

/**
 * class implementation
 */

Incident::Incident(Career* career, unsigned int locationId)
{
    _name = career->getName();
    _virtues = *career->getVirtues();
    _locationId = locationId;
}

Incident::Incident(TiXmlElement* node)
{
    assert( strcmp( node->Value(), "incident" ) == 0 );

    // read jumper name
    _name = node->Attribute( "name" );

    // read incident location
    int location;
    node->Attribute( "location", &location ); assert( location >= 0 );
    _locationId = location;

    // read other info
    TiXmlNode* child = node->FirstChild(); assert( child );
    if( child != NULL ) do 
    {
        // load virtues
        if( child->Type() == TiXmlNode::ELEMENT && strcmp( child->Value(), "virtues" ) == 0 )
        {
            int cs;
            static_cast<TiXmlElement*>( child )->Attribute( "checksum", &cs );
            std::string data = static_cast<TiXmlElement*>( child )->Attribute( "data" );
            decrypt( &_virtues, sizeof(Virtues), data, _name.c_str() );
            if( cs != checksum( &_virtues, sizeof(Virtues) ) )
            {
                throw ccor::Exception( "User database entry corrupted: \"%s\"! Cheating not allowed!", _name.c_str() );
            }
        }
        child = child->NextSibling();
    }
    while( child != NULL );
}

Incident::~Incident()
{
}

/**
 * class behaviour
 */

void Incident::save(TiXmlDocument* xmlDocument)
{
    std::string data;

    // create incident node
    TiXmlElement* incidentElement = new TiXmlElement( "incident" );
    incidentElement->SetAttribute( "name", _name.c_str() );
    incidentElement->SetAttribute( "location", _locationId );

    // create virtues node        
    TiXmlElement* virtuesElement = new TiXmlElement( "virtues" );
    virtuesElement->SetAttribute( "checksum", checksum( &_virtues, sizeof(Virtues) ) );
    encrypt( data, &_virtues, sizeof(Virtues), _name.c_str() );
    virtuesElement->SetAttribute( "data", data.c_str() );
    incidentElement->InsertEndChild( *virtuesElement );
    delete virtuesElement;

    xmlDocument->InsertEndChild( *incidentElement );
    delete incidentElement;
}
