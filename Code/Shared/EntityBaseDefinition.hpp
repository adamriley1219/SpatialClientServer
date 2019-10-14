#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/XML/XMLUtils.hpp"

enum EntityType
{
	ENTITY_UNKNOWN_ENTITY_TYPE = -1,
	ENTITY_ACTOR,
	ENTITY_ABILITY,
	NUM_ENTITIES
};

class EntityBaseDefinition
{
	friend class EntityBase;
protected:
	EntityBaseDefinition(const XmlElement& element);


protected:
	std::string m_name = "NONE";

};