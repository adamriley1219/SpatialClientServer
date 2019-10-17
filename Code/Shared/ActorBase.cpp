#include "Shared/ActorBase.hpp"

#include "Shared/ActorBaseDefinition.hpp"
#include "Shared/AbilityBase.hpp"
#include "Shared/ControllerBase.hpp"
#include "Shared/Zone.hpp"


//--------------------------------------------------------------------------
/**
* ActorBase
*/
ActorBase::ActorBase(const std::string& name, uint zone_id)
	: EntityBase(name, zone_id)
{
	const ActorBaseDefinition* def = ActorBaseDefinition::GetActorDefinitionByName(name);  

	m_basic_attack = def->m_basic_attack;
	m_possessable = def->m_possessable;
	m_speed = def->m_speed;

	m_type = def->m_type;
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
* Update
*/
void ActorBase::Update(float deltaTime)
{
	EntityBase::Update(deltaTime);
	
}

//--------------------------------------------------------------------------
/**
* PreformAbility
*/
void ActorBase::PreformAbility( const std::string& ability_name, const Vec2& target_position )
{
	if( ability_name == "basic_attack" )
	{
		BasicAttack( target_position );
	}
}

//--------------------------------------------------------------------------
/**
* Possess
*/
bool ActorBase::Possess(ControllerBase* controller)
{
	// If the controller that is passed in is a player interface (receives input from players)
	// And if it's not possessable by players
	// Then don't allow player to possess the actor
	if( !m_possessable && controller->m_player_interface )
	{
		return false;
	}

	controller->SetControlled(this);
	m_owner = controller;

	Zone::GetZone(m_owning_zone)->AddController(controller);
	
	return true;
}

//--------------------------------------------------------------------------
/**
* GetSpeed
*/
float ActorBase::GetSpeed() const
{
	return m_speed;
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
