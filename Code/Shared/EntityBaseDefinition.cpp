#include "Shared/EntityBaseDefinition.hpp"

//--------------------------------------------------------------------------
/**
* EntityBaseDefinition
*/
EntityBaseDefinition::EntityBaseDefinition(const XmlElement& element)
{
	m_name = ParseXmlAttribute( element, "name", "none" );
}
