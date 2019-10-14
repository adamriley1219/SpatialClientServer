#pragma once
#include "Shared/EntityBaseDefinition.hpp"

#include <map>

class ActorBaseDefinition
	: public EntityBaseDefinition
{
	friend class ActorBase;
private:
	ActorBaseDefinition( const XmlElement& element );
	~ActorBaseDefinition();

	static std::map< std::string, ActorBaseDefinition* > s_actorDefs;

public:
	static void AddActorDefinition(const XmlElement& element);
	static const ActorBaseDefinition* GetActorDefinitionByName(std::string name);

};