#include "Shared/SharedCommon.hpp"
#include "Shared/AbilityBaseDefinition.hpp"
#include "Shared/ActorBaseDefinition.hpp"

#include "Engine/Core/XML/XMLUtils.hpp"


//--------------------------------------------------------------------------
/**
* LoadActors
*/
void LoadActors()
{
	tinyxml2::XMLDocument gameConfig;
	gameConfig.LoadFile("Data/Gameplay/ActorDefinitions.xml");
	XmlElement* root = gameConfig.RootElement();
	for (XmlElement* element = root->FirstChildElement(); element != nullptr; element = element->NextSiblingElement())
	{
		ActorBaseDefinition::AddActorDefinition( *element );
	}
}

//--------------------------------------------------------------------------
/**
* LoadAbilities
*/
void LoadAbilities()
{
	tinyxml2::XMLDocument gameConfig;
	gameConfig.LoadFile("Data/Gameplay/AbilityDefinitions.xml");
	XmlElement* root = gameConfig.RootElement();
	for (XmlElement* element = root->FirstChildElement(); element != nullptr; element = element->NextSiblingElement())
	{
		AbilityBaseDefinition::AddAbilityDefinition( *element );
	}
}
