#include "Shared/EntityBase.hpp"
#include "Shared/SharedCommon.hpp"
#include "Shared/ControllerBase.hpp"

#include "Engine/Physics/PhysicsSystem.hpp"

#include "Shared/EntityBaseDefinition.hpp"
#include "Shared/Zone.hpp"

//--------------------------------------------------------------------------
/**
* Entity
*/
EntityBase::EntityBase(  const std::string& name, uint zone_id )
{
	m_name = name;

	m_owning_zone = zone_id;
//	Zone* zone = Zone::GetZone(zone_id);
// 	ASSERT_RECOVERABLE( zone, "Adding entity to a zone that doesnt exist" );
// 	if( zone )
// 	{
// 		m_rigidbody = zone->m_physics_system->CreateRigidbody( 1.0f );
// 		m_rigidbody->SetOriginalSimulationType( ePhysicsSimulationType::PHYSICS_SIM_DYNAMIC );
// 
// 		m_rigidbody->SetObject( this, &m_transform );
// 		m_rigidbody->SetPhyMaterial( 0.0f, 0.0f, 13.0f, 8.0f );
// 
// 		m_rigidbody->SetRestrictions( false, false, true );
// 
// 		m_collider = zone->m_physics_system->CreateCollider( false, Vec2::ZERO, 0.5f );
// 		m_rigidbody->SetCollider( m_collider );
// 
// 		
// 		zone->AddEntity( this );
// 	}

}


//--------------------------------------------------------------------------
/**
* Entity
*/
EntityBase::~EntityBase()
{
	Zone* zone = Zone::GetZone( m_owning_zone );
	if (zone)
	{
		zone->m_physics_system->RemoveRigidbody(m_rigidbody);
	}
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
	UNUSED(deltaSeconds);
}

//--------------------------------------------------------------------------
/**
* ApplyForce
*/
void EntityBase::ApplyForce(const Vec2& force)
{
	m_rigidbody->AddForce( force * 10.0f );
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
* SetPosition
*/
void EntityBase::SetPosition( const Vec2& pos )
{
	m_transform.m_position = pos;
}

//--------------------------------------------------------------------------
/**
* SetPosition
*/
void EntityBase::SetPosition( float x, float y )
{
	m_transform.m_position.x = x;
	m_transform.m_position.y = y;
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

//--------------------------------------------------------------------------
/**
* GetResidingZone
*/
uint EntityBase::GetResidingZone() const
{
	return m_owning_zone;
}

//--------------------------------------------------------------------------
/**
* GetType
*/
EntityType EntityBase::GetType() const
{
	return m_type;
}

//--------------------------------------------------------------------------
/**
* GetName
*/
std::string EntityBase::GetName() const
{
	return m_name;
}

