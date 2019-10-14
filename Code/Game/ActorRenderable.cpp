#include "Game/ActorRenderable.hpp"


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