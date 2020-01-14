#pragma once
#include "Shared/EntityBaseDefinition.hpp"

#include <map>

class ActorBaseDefinition
	: public EntityBaseDefinition
{
	friend class ActorBase;
	friend class AIController;
protected:
	ActorBaseDefinition( const XmlElement& element );
	~ActorBaseDefinition();

	static std::map< std::string, ActorBaseDefinition* > s_actorDefs;
	
	std::string m_basic_attack = "none";
	bool m_possessable = false;

	float m_speed = 0.0f;

public:
	static void AddActorDefinition(const XmlElement& element);
	static const ActorBaseDefinition* GetActorDefinitionByName( const std::string& name );
	static bool DoesDefExist( const std::string& name );

};