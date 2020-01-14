#include "Shared/EntityBaseDefinition.hpp"

//--------------------------------------------------------------------------
/**
* IsGameType
*/
bool EntityBaseDefinition::IsGameType(const std::string& name)
{
	return name == "player" 
		|| name == "turret"
		|| name == "crawler"
		;
}

//--------------------------------------------------------------------------
/**
* EntityBaseDefinition
*/
EntityBaseDefinition::EntityBaseDefinition(const XmlElement& element)
{
	m_name = ParseXmlAttribute( element, "name", "none" );
}
