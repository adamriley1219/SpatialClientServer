#include "Game/SpatialOSClient.hpp"

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Memory/Debug/Log.hpp"

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <iostream>

#include <cassert>
#include <crtdbg.h>

//--------------------------------------------------------------------------
//SpatialOS


// Use this to make a worker::ComponentRegistry. This worker doesn't use any components yet
// For example use worker::Components<improbable::Position, improbable::Metadata> to track these common components
using EmptyRegistry = worker::Components<>;

// Constants and parameters
const int ErrorExitStatus = 1;
const std::string kLoggerName = "client";
const std::uint32_t kGetOpListTimeoutInMilliseconds = 100;

// Connection helpers
worker::Connection ConnectWithLocator(const std::string hostname,
	const std::string project_name,
	const std::string deployment_id,
	const std::string login_token,
	const worker::ConnectionParameters& connection_parameters) {
	worker::LocatorParameters locator_parameters;
	locator_parameters.ProjectName = project_name;
	locator_parameters.CredentialsType = worker::LocatorCredentialsType::kLoginToken;
	locator_parameters.LoginToken.Token = login_token;

	worker::Locator locator{ hostname, locator_parameters };

	auto queue_status_callback = [&](const worker::QueueStatus& queue_status) {
		if (!queue_status.Error.empty()) {
			std::cerr << "Error while queueing: " << *queue_status.Error << std::endl;
			return false;
		}
		std::cout << "Worker of type '" << connection_parameters.WorkerType
			<< "' connecting through locator: queueing." << std::endl;
		Logf( "ClientLog", "Worker of type '%s' connecting through locator: queueing.", connection_parameters.WorkerType.c_str() );
		return true;
	};

	auto future = locator.ConnectAsync(EmptyRegistry{}, deployment_id, connection_parameters, queue_status_callback);
	return future.Get();
}


worker::Connection ConnectWithReceptionist(const std::string hostname,
	const std::uint16_t port,
	const std::string& worker_id,
	const worker::ConnectionParameters& connection_parameters) {
	auto future = worker::Connection::ConnectAsync(EmptyRegistry{}, hostname, port, worker_id, connection_parameters);
	return future.Get();
}

static std::string get_random_characters(size_t count) {
	const auto randchar = []() -> char {
		const char charset[] =
			"0123456789"
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			"abcdefghijklmnopqrstuvwxyz";
		const auto max_index = sizeof(charset) - 1;
		return charset[std::rand() % max_index];
	};
	std::string str(count, 0);
	std::generate_n(str.begin(), count, randchar);
	return str;
}

//-----------------------------------------------------------------------------------------------


//--------------------------------------------------------------------------
/**
* Startup
*/
void SpatialOSClient::Startup(const std::vector<std::string>& args)
{
	GetInstance()->isRunning = true;
	GetInstance()->client_thread = std::thread( Run, args );
}

//--------------------------------------------------------------------------
/**
* Shutdown
*/
void SpatialOSClient::Shutdown()
{
	GetInstance()->isRunning = false;
	GetInstance()->client_thread.join();
}

//--------------------------------------------------------------------------
/**
* IsRunning
*/
bool SpatialOSClient::IsRunning()
{
	return GetInstance()->isRunning;
}

//--------------------------------------------------------------------------
/**
* Run
*/
void SpatialOSClient::Run( std::vector<std::string> arguments )
{
	auto now = std::chrono::high_resolution_clock::now();
	std::srand((uint)std::chrono::time_point_cast<std::chrono::nanoseconds>(now).time_since_epoch().count());


	auto print_usage = [&]() {
		std::cout << "Usage: External receptionist <hostname> <port> <worker_id>" << std::endl;
		std::cout << "       External locator <hostname> <project_name> <deployment_id> <login_token>";
		std::cout << std::endl;
		std::cout << "Connects to SpatialOS" << std::endl;
		std::cout << "    <hostname>       - hostname of the receptionist or locator to connect to.";
		std::cout << std::endl;
		std::cout << "    <port>           - port to use if connecting through the receptionist.";
		std::cout << std::endl;
		std::cout << "    <worker_id>      - name of the worker assigned by SpatialOS." << std::endl;
		std::cout << "    <project_name>   - name of the project to run." << std::endl;
		std::cout << "    <deployment_id>  - name of the cloud deployment to run." << std::endl;
		std::cout << "    <login_token>   - token to use when connecting through the locator.";
		std::cout << std::endl;
	};


	worker::ConnectionParameters parameters;
	parameters.WorkerType = "External";
	parameters.Network.ConnectionType = worker::NetworkConnectionType::kTcp;
	parameters.Network.UseExternalIp = true;

	const std::string connection_type = arguments[0];
	if (connection_type != "receptionist" && connection_type != "locator") {
		print_usage();
		return;
	}

	const bool use_locator = connection_type == "locator";

	if ((use_locator && arguments.size() != 5) || (!use_locator && arguments.size() != 4)) {
		print_usage();
		return;
	}

	// Connect with locator or receptionist
	worker::Connection connection = use_locator
		? ConnectWithLocator(arguments[1], arguments[2], arguments[3], arguments[4], parameters)
		: ConnectWithReceptionist(arguments[1], (uint16_t)atoi(arguments[2].c_str()), arguments[3], parameters);
	GetInstance()->connection = &connection;
	connection.SendLogMessage(worker::LogLevel::kInfo, kLoggerName, "Connected successfully");

	// Register callbacks and run the worker main loop.
	worker::View view{ EmptyRegistry{} };
	GetInstance()->view = &view;
	bool is_connected = connection.IsConnected();

	view.OnDisconnect([&](const worker::DisconnectOp& op) {
		std::cerr << "[disconnect] " << op.Reason << std::endl;
		Logf("ClientLog", "[disconnect] %s", op.Reason.c_str() );

		is_connected = false;
		});


	RegisterCallbacks( view );
	
	constexpr unsigned kFramesPerSecond = 60;
	constexpr std::chrono::duration<double> kFramePeriodSeconds{
		1. / static_cast<double>(kFramesPerSecond) };
	Logf( "ClientLog", "Connected and running" );

	while ( GetInstance()->connection->IsConnected() && IsRunning() )
	{
		auto start_time = std::chrono::steady_clock::now();

		auto opList = GetInstance()->connection->GetOpList(0);
		GetInstance()->view->Process(opList);

		for( auto map_ID_entity : view.Entities )
		{
			worker::EntityId id = map_ID_entity.first;
			Logf( "ClientLog", "Client can see %u", id );
			std::cout << "Client can see " << map_ID_entity.first << std::endl;
		}

		auto end_time = std::chrono::steady_clock::now();
		auto wait_for = kFramePeriodSeconds - (end_time - start_time);
		std::this_thread::sleep_for(wait_for);
	}

	GetInstance()->isRunning = false;
	GetInstance()->connection = nullptr;
	GetInstance()->view = nullptr;
}


//--------------------------------------------------------------------------
/**
* RegisterCallbacks
*/
void SpatialOSClient::RegisterCallbacks( worker::View& view )
{
	// Print messages received from SpatialOS
	view.OnLogMessage([&](const worker::LogMessageOp& op) {
		if (op.Level == worker::LogLevel::kFatal) {
			std::cerr << "Fatal error: " << op.Message << std::endl;
			Logf( "ClientLog", "Fetal error: %s", op.Message.c_str() );
			std::terminate();
		}
		std::cout << "Connection: " << op.Message << std::endl;
		Logf("ClientLog", "Connection: %s", op.Message.c_str());
		});
}

//--------------------------------------------------------------------------
/**
* GetInfoFromEnityId
*/
entity_info_t* SpatialOSClient::GetInfoFromEnityId( const worker::EntityId& entity_id )
{
	for (entity_info_t& entity : GetInstance()->entity_info_list)
	{
		if (entity.id == entity_id)
		{
			return &entity;
		}
	}

	return nullptr;
}

//--------------------------------------------------------------------------
/**
* PositionUpdated
*/
void SpatialOSClient::PositionUpdated( const worker::ComponentUpdateOp<improbable::Position>& op )
{
	entity_info_t* info = GetInfoFromEnityId( op.EntityId );
	if ( info && info->created )
	{
		worker::Option<improbable::PositionData&> curPos = info->entity->Get<improbable::Position>();
		std::cout << "PosUpdate: entCords for " << info->id << ":" << curPos->coords().x() << "," << curPos->coords().y() << "," << curPos->coords().z() << std::endl;
		Logf("ClientLog", "PosUpdate: entCords for %u:%.03f,%.03f,%.03f %s", info->id, curPos->coords().x(), curPos->coords().y(), curPos->coords().z() );

	}
}

//--------------------------------------------------------------------------
/**
* GetEntityList
*/
const std::vector<entity_info_t>& SpatialOSClient::GetEntityList()
{
	return GetInstance()->entity_info_list;
}

//--------------------------------------------------------------------------
/**
* GetView
*/
worker::View* SpatialOSClient::GetView()
{
	return GetInstance()->view;
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
* MyComponents
*/
const worker::ComponentRegistry& SpatialOSClient::MyComponents()
{
	static const worker::Components<
		improbable::EntityAcl,
		improbable::Position > components;
	return components;
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
