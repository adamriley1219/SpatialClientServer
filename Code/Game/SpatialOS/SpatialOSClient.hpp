#pragma once



#include <improbable/worker.h>
#include <improbable/standard_library.h>


class SpatialOSClient
{
public:
	static void Startup();
	static void Shutdown();

private:
	static void Update();
	static void RequestEntityCreation( const worker::Entity& entity );

private:
	static SpatialOSClient* GetInstance();
	SpatialOSClient();
	~SpatialOSClient();

};