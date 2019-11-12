#include "Engine/Physics/DiscCollider2D.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Physics/Rigidbody2D.hpp"

#ifdef PLATFORM_WINDOWS
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Core/Vertex/Vertex_PCU.hpp"
#endif


//--------------------------------------------------------------------------
/**
* DiscCollider2D
*/
DiscCollider2D::DiscCollider2D( bool isTrigger, const Vec2& center, float radius )
	: Collider2D( isTrigger )
{
	m_localShape.m_center = center;
	m_localShape.m_radius = radius;
	m_type = eColliderType2D::COLLIDER_DISC;
}

//--------------------------------------------------------------------------
/**
* DebugRender
*/
#ifdef PLATFORM_WINDOWS
void DiscCollider2D::DebugRender( RenderContext* renderer, const Rgba& color ) const
{
	std::vector<Vertex_PCU> verts;
	AddVertsForRing2D(	verts, GetWorldShape().m_center, m_localShape.m_radius, 0.05f, color );

	renderer->DrawVertexArray( verts );
}
#endif

//--------------------------------------------------------------------------
/**
* GetMomentOfInertia
*/
float DiscCollider2D::GetMomentOfInertia()
{
	if( !m_rigidbody )
	{
		return 1.0f;
	}
	float discMass = m_rigidbody->GetMass();
	return 0.5f * discMass * m_localShape.m_radius * m_localShape.m_radius + m_localShape.m_center.GetLengthSquared() * discMass;
}

//--------------------------------------------------------------------------
/**
* GetLocalShape
*/
Disc DiscCollider2D::GetLocalShape() const
{
	return m_localShape;
}

//--------------------------------------------------------------------------
/**
* GetWorldShape
*/
Disc DiscCollider2D::GetWorldShape() const
{
	if( !m_rigidbody )
	{
		return m_localShape;
	}
	Disc toRet;
	toRet.m_center = m_localShape.m_center + m_rigidbody->GetTransform().m_position;
	toRet.m_radius = m_localShape.m_radius;
	return toRet;
}

