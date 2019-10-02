#include "Game/Entity.hpp"


//--------------------------------------------------------------------------
/**
* EntityBase
*/
Entity::Entity() : EntityBase()
{

}

//--------------------------------------------------------------------------
/**
* ~EntityBase
*/
Entity::~Entity()
{
	EntityBase::~EntityBase();
}

//--------------------------------------------------------------------------
/**
* Render
*/
void Entity::Render() const
{

}



//--------------------------------------------------------------------------
/**
* EntityGetColor
*/
Rgba Entity::GetTint() const
{
	return m_tint;
}