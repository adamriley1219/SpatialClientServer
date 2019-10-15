#include "Shared/ActorBase.hpp"

#include "Shared/ActorBaseDefinition.hpp"
#include "Shared/AbilityBase.hpp"

//--------------------------------------------------------------------------
/**
* ActorBase
*/
ActorBase::ActorBase(const std::string& name, uint zone_id)
	: EntityBase(name, zone_id)
{
	const ActorBaseDefinition* def = ActorBaseDefinition::GetActorDefinitionByName(name);  

	m_basic_attack = def->basic_attack;
}

//--------------------------------------------------------------------------
/**
* ~ActorBase
*/
ActorBase::~ActorBase()
{

}

//--------------------------------------------------------------------------
/**
* PreformAbility
*/
void ActorBase::PreformAbility( const std::string& ability_name, const Vec2& input_position )
{
	if( ability_name == "basic_attack" )
	{
		BasicAttack( input_position );
	}
}

//--------------------------------------------------------------------------
/**
* BasicAttack
*/
void ActorBase::BasicAttack( const Vec2& input_position )
{
	if( m_basic_attack == "none" )
	{
		return;
	}
	Vec2 displacement = input_position - GetPosition();
	displacement.Normalize();
	AbilityBase* ability = new AbilityBase( m_basic_attack, m_owning_zone );
	ability->SetPosition( GetPosition() + displacement * 1.0f); // radius of actor
	ability->SetDirection( displacement );

}
