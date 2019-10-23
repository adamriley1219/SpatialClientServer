#include "Game/SpatialOS/SpatialOSClient.hpp"

//--------------------------------------------------------------------------
/**
* Startup
*/
void SpatialOSClient::Startup()
{

}

//--------------------------------------------------------------------------
/**
* Shutdown
*/
void SpatialOSClient::Shutdown()
{

}

//--------------------------------------------------------------------------
/**
* Update
*/
void SpatialOSClient::Update()
{

}

//--------------------------------------------------------------------------
/**
* RequestEntityCreation
*/
void SpatialOSClient::RequestEntityCreation( const worker::Entity& entity )
{

}

//--------------------------------------------------------------------------
/**
* GetInstance
*/
SpatialOSClient* SpatialOSClient::GetInstance()
{
	static SpatialOSClient* s_system = new SpatialOSClient(); 
	return s_system;  
}

//--------------------------------------------------------------------------
/**
* SpatialOSClient
*/
SpatialOSClient::SpatialOSClient()
{

}

//--------------------------------------------------------------------------
/**
* ~SpatialOSClient
*/
SpatialOSClient::~SpatialOSClient()
{

}
