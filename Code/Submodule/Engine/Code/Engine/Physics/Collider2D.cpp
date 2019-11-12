#include "Engine/Physics/Collider2D.hpp"
#include "Engine/Physics/Rigidbody2D.hpp"
#include "Engine/Physics/Collision2D.hpp"
#include "Engine/Core/EngineCommon.hpp"



//--------------------------------------------------------------------------
/**
* SetRigidbody
*/
void Collider2D::SetRigidbody( Rigidbody2D* rigidbody )
{
	m_rigidbody = rigidbody;
}

//--------------------------------------------------------------------------
/**
* SetTrigger
*/
void Collider2D::SetTrigger( bool isTrigger )
{
	m_isTrigger = isTrigger;
}

//--------------------------------------------------------------------------
/**
* IsTouching
*/
bool Collider2D::IsTouching( Collider2D* otherCollider, Collision2D* collisionInfo )
{
	Collision2D scrapCollision;
	bool didCollide = Collision2D::GetCollisionInfo( !collisionInfo ? &scrapCollision : collisionInfo, this, otherCollider );
	return didCollide;
}

//--------------------------------------------------------------------------
/**
* FireHitEvent
*/
void Collider2D::FireHitEvent( Collider2D* colWith )
{
	if( m_hitEvent != "" )
	{
	}
}

//--------------------------------------------------------------------------
/**
* FireEnterEvent
*/
void Collider2D::FireEnterEvent( Collider2D* colWith )
{
	if( m_onEnterEvent != "" )
	{
	}
}

//--------------------------------------------------------------------------
/**
* FireExitEvent
*/
void Collider2D::FireExitEvent( Collider2D* colWith )
{
	if( m_onExitEvent != "" )
	{
	}
}

//--------------------------------------------------------------------------
/**
* FireStayEvent
*/
void Collider2D::FireStayEvent( Collider2D* colWith ) 
{
	if( m_onStayEvent != "" )
	{
	}
}

//--------------------------------------------------------------------------
/**
* ExitAllColliders
*/
void Collider2D::ExitAllColliders()
{
	for( Collider2D*& c : m_curColliding )
	{
		if( c )
		{
			FireExitEvent( c );
			c->FireExitEvent( this );
			Collider2D* toLeave = c;
			ExitCollider( c );
			toLeave->ExitCollider( this );
		}
	}
}

//--------------------------------------------------------------------------
/**
* IsOverlapped
*/
bool Collider2D::IsOverlapped( Collider2D* collider ) const
{
	for( Collider2D* c : m_curColliding )
	{
		if( c && c == collider )
		{
			return true;
		}
	}
	return false;
}

//--------------------------------------------------------------------------
/**
* EnterCollider
*/
bool Collider2D::EnterCollider( Collider2D* collider )
{
	bool wasColliding = IsOverlapped( collider );
	if( wasColliding )
	{
		return false;
	}

	for( Collider2D*& c : m_curColliding )
	{
		if( !c )
		{
			c = collider;
			return true;
		}
	}
	m_curColliding.push_back( collider );
	return true;
}

//--------------------------------------------------------------------------
/**
* ExitCollider
*/
bool Collider2D::ExitCollider( Collider2D* collider )
{
	bool wasColliding = IsOverlapped( collider );
	if( !wasColliding )
	{
		return false;
	}

	for( Collider2D*& c : m_curColliding )
	{
		if( c == collider )
		{
			c = nullptr;
			return true;
		}
	}
	return false;
}
