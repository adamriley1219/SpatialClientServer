#pragma once
#include "Shared/EntityBaseDefinition.hpp"

#include <map>

class ActorBaseDefinition
	: public EntityBaseDefinition
{
	friend class ActorBase;
protected:
	ActorBaseDefinition( const XmlElement& element );
	~ActorBaseDefinition();

	static std::map< std::string, ActorBaseDefinition* > s_actorDefs;
	
	std::string basic_attack = "none";

public:
	static void AddActorDefinition(const XmlElement& element);
	static const ActorBaseDefinition* GetActorDefinitionByName(std::string name);

};