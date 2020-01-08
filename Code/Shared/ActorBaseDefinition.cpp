#include "Shared/ActorBaseDefinition.hpp"

#include "Engine/Core/EngineCommon.hpp"

std::map< std::string, ActorBaseDefinition* > ActorBaseDefinition::s_actorDefs;

#include <iostream>

//--------------------------------------------------------------------------
/**
* ActorBaseDefinition
*/
ActorBaseDefinition::ActorBaseDefinition(const XmlElement& element)
	: EntityBaseDefinition( element )
{
	m_basic_attack = ParseXmlAttribute( element, "basic_attack", m_basic_attack );
	m_possessable = ParseXmlAttribute( element, "possess", m_possessable );
	m_speed = ParseXmlAttribute( element, "speed", m_speed );
	m_type = ENTITY_ACTOR;
}

//--------------------------------------------------------------------------
/**
* ~ActorBaseDefinition
*/
ActorBaseDefinition::~ActorBaseDefinition()
{

}

//--------------------------------------------------------------------------
/**
* AddActorDefinition
*/
void ActorBaseDefinition::AddActorDefinition(const XmlElement& element)
{
	std::string name = ParseXmlAttribute( element, "name", "none" );
	s_actorDefs[name] = new ActorBaseDefinition( element );
}

//--------------------------------------------------------------------------
/**
* GetActorDefinitionByName
*/
const ActorBaseDefinition* ActorBaseDefinition::GetActorDefinitionByName(std::string name)
{
	if( s_actorDefs.find( name ) != s_actorDefs.end() )
	{
		std::cout << "found a def" << std::endl;
		return s_actorDefs[name];
	}
	std::cout << "didnt find a def" << std::endl;
	return nullptr;
}
