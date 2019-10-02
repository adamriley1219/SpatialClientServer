#include "Shared/EntityBase.hpp"
#include "Shared/SharedCommon.hpp"

#include "Engine/Physics/PhysicsSystem.hpp"


//--------------------------------------------------------------------------
/**
* Entity
*/
EntityBase::EntityBase()
{
	m_rigidbody = g_thePhysicsSystem->CreateRigidbody( 1.0f );
	m_rigidbody->SetOriginalSimulationType( ePhysicsSimulationType::PHYSICS_SIM_DYNAMIC );

	m_rigidbody->SetObject( this, &m_transform );
	m_rigidbody->SetPhyMaterial( 0.0f, 0.0f, 5.0f, 5.0f );

	m_collider = g_thePhysicsSystem->CreateCollider( false, Vec2::ZERO, 0.5f );
	m_rigidbody->SetCollider( m_collider );

}


//--------------------------------------------------------------------------
/**
* Entity
*/
EntityBase::~EntityBase()
{
}

//--------------------------------------------------------------------------
/**
* Kill
*/
void EntityBase::Die()
{
	m_isDead = true;
	m_isGarbage = true;
}

//--------------------------------------------------------------------------
/**
* isDying
*/
bool EntityBase::IsAlive() const
{
	return !m_isDead;
}

//--------------------------------------------------------------------------
/**
* DieisGarbage
*/
bool EntityBase::IsGarbage() const
{
	return m_isGarbage;
}


//--------------------------------------------------------------------------
/**
* Update
*/
void EntityBase::Update(float deltaSeconds)
{
	
}

//--------------------------------------------------------------------------
/**
* ApplyForce
*/
void EntityBase::ApplyForce(const Vec2& force)
{
	m_rigidbody->AddForce( force * 13.0f );
}

//--------------------------------------------------------------------------
/**
* getVelocity
*/
Vec2 EntityBase::GetPosition() const
{
	return m_transform.m_position;
}

//--------------------------------------------------------------------------
/**
* TakeDamage
*/
void EntityBase::TakeDamage(float damage)
{
	if ((m_health -= damage) <= 0.0f)
	{
		Die();
	}
}

