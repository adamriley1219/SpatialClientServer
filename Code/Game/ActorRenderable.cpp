#include "Game/ActorRenderable.hpp"
#include "Game/Game.hpp"

//--------------------------------------------------------------------------
/**
* ActorRenderable
*/
ActorRenderable::ActorRenderable( const std::string& name )
	: ActorBase( name, g_theGame->GetZoneID() )
{

}

//--------------------------------------------------------------------------
/**
* ~EntityBase
*/
ActorRenderable::~ActorRenderable()
{
	EntityBase::~EntityBase();
}

//--------------------------------------------------------------------------
/**
* Render
*/
void ActorRenderable::Render() const
{

}



//--------------------------------------------------------------------------
/**
* EntityGetColor
*/
Rgba ActorRenderable::GetTint() const
{
	return m_tint;
}