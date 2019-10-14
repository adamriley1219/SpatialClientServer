#pragma once

#include "Shared/EntityBaseDefinition.hpp"

#include <map>

class AbilityBaseDefinition
	: public EntityBaseDefinition
{
	friend class ActorBase;
private:
	AbilityBaseDefinition(const XmlElement& element);
	~AbilityBaseDefinition();

	static std::map< std::string, AbilityBaseDefinition* > s_abilityDefs;

	bool m_ranged;
	float m_speed = 0.0f;

public:
	static void AddAbilityDefinition(const XmlElement& element);
	static const AbilityBaseDefinition* GetAbilityDefinitionByName(std::string name);

};