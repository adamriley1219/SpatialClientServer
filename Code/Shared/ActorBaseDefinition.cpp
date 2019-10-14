#include "Shared/ActorBaseDefinition.hpp"

#include "Engine/Core/EngineCommon.hpp"

std::map< std::string, ActorBaseDefinition* > ActorBaseDefinition::s_actorDefs;


//--------------------------------------------------------------------------
/**
* ActorBaseDefinition
*/
ActorBaseDefinition::ActorBaseDefinition(const XmlElement& element)
	: EntityBaseDefinition( element )
{
	
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
		return s_actorDefs[name];
	}
	return nullptr;
}
