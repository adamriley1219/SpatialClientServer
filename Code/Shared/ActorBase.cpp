#include "Shared/ActorBase.hpp"
#include "Shared/ActorBaseDefinition.hpp"


//--------------------------------------------------------------------------
/**
* ActorBase
*/
ActorBase::ActorBase(const std::string& name)
	: EntityBase(name)
{
	const ActorBaseDefinition* def = ActorBaseDefinition::GetActorDefinitionByName(name);  
}

//--------------------------------------------------------------------------
/**
* ~ActorBase
*/
ActorBase::~ActorBase()
{

}
