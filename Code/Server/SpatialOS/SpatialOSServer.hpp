#pragma once



#include <improbable/worker.h>
#include <improbable/standard_library.h>

#include <thread>


class SpatialOSServer
{
public:
	static void Startup( const std::vector<std::string>& args );
	static void Shutdown();

public:
	static void RequestEntityCreation( const worker::Entity& entity );
	static bool IsRunning();

private:
	static void Run( std::vector<std::string> args );

private:
	static SpatialOSServer* GetInstance();
	SpatialOSServer();
	~SpatialOSServer();

private:
	bool isRunning = false;
	std::thread server_thread;

	worker::Dispatcher* dispatcher;
	worker::Connection* connection;
};