
#include "Server/SpatialOSServer.hpp"
#include "Server/ServerApp.hpp"
#include "Server/ServerCommon.hpp"

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Strings/NamedStrings.hpp"

#include <iostream>
#include <chrono>

//-----------------------------------------------------------------------------------------------
void RunFrame()
{
	g_theServerApp->RunFrame();
}

//-----------------------------------------------------------------------------------------------
void Startup()
{
	std::cout << "Init Startup, world sim" << std::endl;
	tinyxml2::XMLDocument config;
	config.LoadFile("Data/GameConfig.xml");
	XmlElement* root = config.RootElement();
	if( root )
	{
		g_gameConfigBlackboard.PopulateFromXmlElementAttributes(*root);
	}
	g_theServerApp = new ServerApp();
	g_theServerApp->Startup();
	std::cout << "After ServerApp Startup" << std::endl;
}

//-----------------------------------------------------------------------------------------------
void Shutdown()
{
	g_theServerApp->Shutdown();

	// Destroy the global App instance
	delete g_theServerApp;
	g_theServerApp = nullptr;

}

// Entry point
int main( int argc, char** argv ) 
{
	std::cout << "MAIN BEGIN" << std::endl;
	std::vector<std::string> arguments;

	// if no arguments are supplied, use the defaults for a local deployment
	if (argc == 1) {
		arguments = { "receptionist", "localhost", "7777" };
		std::cout << "No args" << std::endl;
	}
	else {
		arguments = std::vector<std::string>( argv + 1, argv + argc );
		std::cout << "Get args" << std::endl;
		for( std::string arg : arguments )
		{
			std::cout << arg << " ";
		}
		std::cout << std::endl;
	}

	std::cout << "STARTUP BEGIN" << std::endl;

	
	SpatialOSServer::Startup( arguments );
	Startup();
	std::cout << "STARTUP COMPLETE" << std::endl;


    while ( SpatialOSServer::IsRunning() ) {
		RunFrame();
		std::this_thread::yield();
    }

	std::cout << "SHUTDOWN INIT" << std::endl;

	Shutdown();

	SpatialOSServer::Shutdown();

	std::cout << "SHUTDOWN COMPLETE" << std::endl;

    return 0;
}
