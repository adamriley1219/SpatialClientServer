#include "Server/SelfSubActor.hpp"

#include "Server/SpatialOSServer.hpp"

//--------------------------------------------------------------------------
/**
* SelfSubActor
*/
SelfSubActor::SelfSubActor( const std::string& name )
	: ActorBase( name )
{
	SpatialOSServer::RequestEntityCreation( this );
}

//--------------------------------------------------------------------------
/**
* SelfSubActor
*/
SelfSubActor::SelfSubActor( const std::string& name, const Vec2& position )
	: ActorBase( name )
{
	m_transform.m_position = position;
	SpatialOSServer::RequestEntityCreation(this);
}

//--------------------------------------------------------------------------
/**
* ~SelfSubActor
*/
SelfSubActor::~SelfSubActor()
{

}
