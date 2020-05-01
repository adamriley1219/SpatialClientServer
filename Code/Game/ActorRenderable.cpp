#include "Game/ActorRenderable.hpp"
#include "Game/Game.hpp"

#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"

//--------------------------------------------------------------------------
/**
* ActorRenderable
*/
ActorRenderable::ActorRenderable( const std::string& name )
	: ActorBase( name )
{
	
}

//--------------------------------------------------------------------------
/**
* ~EntityBase
*/
ActorRenderable::~ActorRenderable()
{

}

//--------------------------------------------------------------------------
/**
* Render
*/
void ActorRenderable::Render() const
{
	std::vector<Vertex_PCU> verts;
	AABB2 box( 1.0f, 1.0f, m_transform.m_position );
	if( m_name == "player" )
	{
		Vec2 mins( 0.f, 0.75f );
		Vec2 maxs( 0.25f, 1.0f );
		g_theRenderer->BindTextureView( 0, (TextureView*) g_theRenderer->CreateOrGetTextureViewFromFile( "Data/Images/Engineer.png" ) );
		AddVertsForAABB2D( verts, box, Rgba::WHITE, mins, maxs );
	}
	if( m_name == "crawler" )
	{
		Vec2 mins(0.f, 0.f);
		Vec2 maxs(1.0f / 8.0f, 1.0f / 8.0f);
		g_theRenderer->BindTextureView(0, (TextureView*)g_theRenderer->CreateOrGetTextureViewFromFile("Data/Images/Cat0.png"));
		AddVertsForAABB2D(verts, box, Rgba::WHITE, mins, maxs);

	}
	if( m_name == "turret" )
	{
		Vec2 mins(0.7f, 9.0f / 10.0f);
		Vec2 maxs(0.8f, 1.0f);
		TextureView* view = (TextureView*)g_theRenderer->CreateOrGetTextureViewFromFile("Data/Images/Undead0.png");
		SpriteSheet sheet( view, IntVec2( 8, 10 ) );
		SpriteDefinition def = sheet.GetSpriteDefinition( 7, 2 );
		def.GetUVs( mins, maxs );
		g_theRenderer->BindTextureView(0, (TextureView*)g_theRenderer->CreateOrGetTextureViewFromFile("Data/Images/Undead0.png"));
		AddVertsForAABB2D(verts, box, Rgba::WHITE, mins, maxs);
	}
	if( !verts.empty() )
	{
		g_theRenderer->DrawVertexArray( verts );
	}
}



//--------------------------------------------------------------------------
/**
* EntityGetColor
*/
Rgba ActorRenderable::GetTint() const
{
	return m_tint;
}