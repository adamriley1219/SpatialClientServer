#include "Game/ActorRenderable.hpp"
#include "Game/Game.hpp"

//--------------------------------------------------------------------------
/**
* ActorRenderable
*/
ActorRenderable::ActorRenderable( const std::string& name )
	: SelfSubActor( name )
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

}



//--------------------------------------------------------------------------
/**
* EntityGetColor
*/
Rgba ActorRenderable::GetTint() const
{
	return m_tint;
}