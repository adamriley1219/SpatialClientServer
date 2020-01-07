#include "Server/SelfSubActor.hpp"

#include "Server/SpatialOSServer.hpp"

//--------------------------------------------------------------------------
/**
* SelfSubBase
*/
SelfSubBase::SelfSubBase( const std::string& name )
	: ActorBase( name )
{
	SpatialOSServer::RequestEntityCreation( this );
}

//--------------------------------------------------------------------------
/**
* SelfSubBase
*/
SelfSubBase::SelfSubBase( const std::string& name, const Vec2& position )
	: ActorBase( name )
{
	m_transform.m_position = position;
	SpatialOSServer::RequestEntityCreation(this);
}

//--------------------------------------------------------------------------
/**
* ~SelfSubBase
*/
SelfSubBase::~SelfSubBase()
{

}
