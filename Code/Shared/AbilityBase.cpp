#include "Shared/AbilityBase.hpp"
#include "Shared/AbilityBaseDefinition.hpp"
#include "Shared/ActorBase.hpp"

#include "Engine/Physics/Collision2D.hpp"

//--------------------------------------------------------------------------
/**
* AbilityBase
*/
AbilityBase::AbilityBase( const std::string& name )
	: EntityBase( name )
{
	const AbilityBaseDefinition* def = AbilityBaseDefinition::GetAbilityDefinitionByName(name);
	m_collider->SetTrigger(def->m_isTrigger);
	m_life_time = def->m_life_time;
	m_speed = def->m_speed;
	m_type = def->m_type;
}

//--------------------------------------------------------------------------
/**
* ~AbilityBase
*/
AbilityBase::~AbilityBase()
{

}

//--------------------------------------------------------------------------
/**
* Update
*/
void AbilityBase::Update(float deltaSeconds)
{
	EntityBase::Update(deltaSeconds);
	m_time_alive += deltaSeconds;
	if( m_life_time < m_time_alive )
	{
		Die();
		return;
	}
	m_transform.m_position += m_direction * m_speed * deltaSeconds;
}

//--------------------------------------------------------------------------
/**
* SetDirection
*/
void AbilityBase::SetDirection( const Vec2& dir )
{
	m_direction = dir;
}

//--------------------------------------------------------------------------
/**
* SetOwner
*/
void AbilityBase::SetOwner(ActorBase* owner)
{
	m_owner = owner;
}
