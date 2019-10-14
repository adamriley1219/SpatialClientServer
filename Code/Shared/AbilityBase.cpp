#include "Shared/AbilityBase.hpp"
#include "Shared/AbilityBaseDefinition.hpp"

//--------------------------------------------------------------------------
/**
* AbilityBase
*/
AbilityBase::AbilityBase(const std::string& name)
	: EntityBase(name)
{
	const AbilityBaseDefinition* def = AbilityBaseDefinition::GetAbilityDefinitionByName(name);

}

//--------------------------------------------------------------------------
/**
* ~AbilityBase
*/
AbilityBase::~AbilityBase()
{

}
