#pragma once

#include "Shared/EntityBaseDefinition.hpp"

#include <map>

class AbilityBaseDefinition
	: public EntityBaseDefinition
{
	friend class AbilityBase;
protected:
	AbilityBaseDefinition(const XmlElement& element);
	~AbilityBaseDefinition();

	static std::map< std::string, AbilityBaseDefinition* > s_abilityDefs;

	bool m_ranged;
	bool m_isTrigger = true;
	float m_speed = 0.0f;
	float m_life_time = 0.1f;

public:
	static void AddAbilityDefinition(const XmlElement& element);
	static const AbilityBaseDefinition* GetAbilityDefinitionByName(std::string name);

};