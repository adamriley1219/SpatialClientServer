#include "Shared/AbilityBaseDefinition.hpp"

std::map< std::string, AbilityBaseDefinition* > AbilityBaseDefinition::s_abilityDefs;

//--------------------------------------------------------------------------
/**
* AbilityBaseDefinition
*/
AbilityBaseDefinition::AbilityBaseDefinition(const XmlElement& element)
	: EntityBaseDefinition(element)
{
	m_ranged = ParseXmlAttribute( element, "ranged", m_ranged );
	m_speed = ParseXmlAttribute( element, "speed", m_speed );
}

//--------------------------------------------------------------------------
/**
* ~AbilityBaseDefinition
*/
AbilityBaseDefinition::~AbilityBaseDefinition()
{

}

//--------------------------------------------------------------------------
/**
* AddAbilityDefinition
*/
void AbilityBaseDefinition::AddAbilityDefinition(const XmlElement& element)
{
	std::string name = ParseXmlAttribute( element, "name", "none" );
	s_abilityDefs[name] = new AbilityBaseDefinition(element);
}

//--------------------------------------------------------------------------
/**
* GetAbilityDefinitionByName
*/
const AbilityBaseDefinition* AbilityBaseDefinition::GetAbilityDefinitionByName(std::string name)
{
	if( s_abilityDefs.find( name ) != s_abilityDefs.end() )
	{
		return s_abilityDefs[name];
	}
	return nullptr;
}
