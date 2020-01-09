#include "Game/SelfSubActor.hpp"
#include "Game/Game.hpp"
#include "Game/SpatialOSClient.hpp"


//--------------------------------------------------------------------------
/**
* SelfSubActor
*/
SelfSubActor::SelfSubActor( const std::string& name )
	: ActorBase( name )
{
	SpatialOSClient::RequestEntityCreation( this );
}

//--------------------------------------------------------------------------
/**
* SelfSubActor
*/
SelfSubActor::SelfSubActor(const std::string& name, const Vec2& position)
	: ActorBase(name)
{
	m_transform.m_position = position;
	SpatialOSClient::RequestEntityCreation( this );
}

//--------------------------------------------------------------------------
/**
* ~SelfSubActor
*/
SelfSubActor::~SelfSubActor()
{

}
