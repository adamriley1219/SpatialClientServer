#include "Server/SpatialOSServer.hpp"
#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <iostream>


#include "Engine/Core/EngineCommon.hpp"



// Use this to make a worker::ComponentRegistry.
// For example use worker::Components<improbable::Position, improbable::Metadata> to track these common components
using ComponentRegistry = worker::Components<improbable::Position, improbable::Metadata>;

// Constants and parameters
const int ErrorExitStatus = 1;
const std::string kLoggerName = "startup.cc";
const std::uint32_t kGetOpListTimeoutInMilliseconds = 500;

worker::Connection ConnectWithReceptionist(const std::string hostname,
	const std::uint16_t port,
	const std::string& worker_id,
	const worker::ConnectionParameters& connection_parameters) {
	auto future = worker::Connection::ConnectAsync(ComponentRegistry{}, hostname, port, worker_id, connection_parameters);
	return future.Get();
}

std::string get_random_characters(size_t count) {
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


//--------------------------------------------------------------------------
/**
* Startup
*/
void SpatialOSServer::Startup( const std::vector<std::string>& arguments )
{
	GetInstance()->isRunning = true;
	GetInstance()->server_thread = std::thread( Run, arguments );
}

//--------------------------------------------------------------------------
/**
* Shutdown
*/
void SpatialOSServer::Shutdown()
{
	GetInstance()->isRunning = false;
	GetInstance()->server_thread.join();
}

//--------------------------------------------------------------------------
/**
* RequestEntityCreation
*/
void SpatialOSServer::RequestEntityCreation( worker::Entity* entity )
{
	if( !IsRunning() )
	{
		return;
	}

	entity_info_t info;
	// Reserve an entity ID.
	info.entity_id_reservation_request_id =
		GetInstance()->connection->SendReserveEntityIdsRequest(1, kGetOpListTimeoutInMilliseconds);

	info.entity = entity;

	GetInstance()->entity_info_list.push_back( info );
}

//--------------------------------------------------------------------------
/**
* IsRunning
*/
bool SpatialOSServer::IsRunning()
{
	return GetInstance()->isRunning;
}


//--------------------------------------------------------------------------
/**
* Run
*/
void SpatialOSServer::Run( const std::vector<std::string> arguments )
{

	auto now = std::chrono::high_resolution_clock::now();
	std::srand((unsigned int)std::chrono::time_point_cast<std::chrono::nanoseconds>(now).time_since_epoch().count());

	std::cout << "[local] Worker started " << std::endl << std::flush;



	auto print_usage = [&]() {
		std::cout << "Usage: Managed receptionist <hostname> <port> <worker_id>" << std::endl;
		std::cout << std::endl;
		std::cout << "Connects to SpatialOS" << std::endl;
		std::cout << "    <hostname>      - hostname of the receptionist or locator to connect to.";
		std::cout << std::endl;
		std::cout << "    <port>          - port to use if connecting through the receptionist.";
		std::cout << std::endl;
		std::cout << "    <worker_id>     - (optional) name of the worker assigned by SpatialOS." << std::endl;
		std::cout << std::endl;
	};

	if (arguments.size() != 3 && arguments.size() != 4) {
		print_usage();
		return;
	}

	worker::ConnectionParameters parameters;
	parameters.WorkerType = "Managed";
	parameters.Network.ConnectionType = worker::NetworkConnectionType::kTcp;
	parameters.Network.UseExternalIp = false;

	std::string workerId;

	// When running as an external worker using 'spatial local worker launch'
	// The WorkerId isn't passed, so we generate a random one
	if (arguments.size() == 4) {
		workerId = arguments[3];
	}
	else {
		workerId = parameters.WorkerType + "_" + get_random_characters(4);
	}

	std::cout << "[local] Connecting to SpatialOS as " << workerId << "..." << std::endl;
	std::cout << "Printing: ";

	for (uint argIdx = 0; argIdx < arguments.size(); ++argIdx)
	{
		std::cout << arguments[argIdx].c_str() << ", ";
	}

	std::cout << std::endl;


	// Connect with receptionist
	worker::Connection connection = ConnectWithReceptionist(arguments[1], atoi(arguments[2].c_str()), workerId, parameters);
	GetInstance()->connection = &connection;
	connection.SendLogMessage(worker::LogLevel::kInfo, kLoggerName, "Connected successfully");

	std::cout << "ConnectWithReceptionist done" << std::endl;

	// Register callbacks and run the worker main loop.
	worker::Dispatcher dispatcher = worker::Dispatcher{ ComponentRegistry{} };
	GetInstance()->dispatcher = &dispatcher;
	bool is_connected = connection.IsConnected();

	std::cout << "Dispatcher done" << std::endl;

	dispatcher.OnDisconnect([&](const worker::DisconnectOp& op) {
		std::cerr << "[disconnect] " << op.Reason << std::endl;
		is_connected = false;
		});
	
	dispatcher.OnRemoveEntity([&]( const worker::RemoveEntityOp& op )
		{
			
		});
	dispatcher.OnAddEntity([&]( const worker::AddEntityOp& op )
		{

		});


	// When the reservation succeeds, create an entity with the reserved ID.
	dispatcher.OnReserveEntityIdsResponse( ReserveEntityIdsResponse );

	// When the creation succeeds, delete the entity.
	dispatcher.OnCreateEntityResponse( CreateEntityResponse );

	// When the deletion succeeds, we're done.
	dispatcher.OnDeleteEntityResponse( DeleteEntityResponse );
	
	std::cout << "OnDisconnect done" << std::endl;


	// Print log messages received from SpatialOS
	dispatcher.OnLogMessage([&](const worker::LogMessageOp& op) {
		if (op.Level == worker::LogLevel::kFatal) {
			std::cerr << "Fatal error: " << op.Message << std::endl;
			std::terminate();
		}
		std::cout << "[remote] " << op.Message << std::endl;
		});

	std::cout << "OnLogMessage done" << std::endl;

	if (is_connected) {
		std::cout << "[local] Connected successfully to SpatialOS, listening to ops... " << std::endl;
		GetInstance()->isRunning = true;
	}

	std::cout << "Is Connected done" << std::endl;

	std::cout << "thread running" << std::endl;

	worker::Entity entity;

	RequestEntityCreation( &entity );

	while (GetInstance()->connection->IsConnected() && IsRunning())
	{
		GetInstance()->dispatcher->Process(GetInstance()->connection->GetOpList(kGetOpListTimeoutInMilliseconds));
	}

	
	std::cout << "Finish Running" << std::endl;


	GetInstance()->isRunning = false;
	GetInstance()->connection = nullptr;
	GetInstance()->dispatcher = nullptr;
}

//--------------------------------------------------------------------------
/**
* DeleteEntityResponse
*/
uint64_t SpatialOSServer::DeleteEntityResponse( const worker::DeleteEntityResponseOp& op )
{
	entity_info_t entity_info;
	if ( GetInfoFromDeleteEnityRequest( op.RequestId, entity_info ) &&
		op.StatusCode == worker::StatusCode::kSuccess ) 
	{
		// Test successful!
		SAFE_DELETE( entity_info.entity );
	}
	return 0;
}

//--------------------------------------------------------------------------
/**
* CreateEntityResponse
*/
uint64_t SpatialOSServer::CreateEntityResponse( const worker::CreateEntityResponseOp& op )
{
	entity_info_t entity_info;
	if ( GetInfoFromCreateEnityRequest( op.RequestId, entity_info ) &&
		op.StatusCode == worker::StatusCode::kSuccess) 
	{
		entity_info.id = *op.EntityId;
		std::cout << "Entity creation successful" <<std::endl;
	}
	return 0;
}

//--------------------------------------------------------------------------
/**
* ReserveEntityIdsResponse
*/
uint64_t SpatialOSServer::ReserveEntityIdsResponse( const worker::ReserveEntityIdsResponseOp& op )
{
	entity_info_t entity_info;

	if ( GetInfoFromReserveEnityIdsRequest( op.RequestId, entity_info ) &&
		op.StatusCode == worker::StatusCode::kSuccess ) {
		// ID reservation was successful - create an entity with the reserved ID.
		entity_info.entity->Add<improbable::Position>({ {1, 2, 3} });
		auto result = GetInstance()->connection->SendCreateEntityRequest( *(entity_info.entity), op.FirstEntityId, kGetOpListTimeoutInMilliseconds );
		// Check no errors occurred.
		if (result) {
			entity_info.entity_creation_request_id = *result;
		}
		else {
			GetInstance()->connection->SendLogMessage(worker::LogLevel::kError, "CreateDeleteEntity",
				result.GetErrorMessage());
			std::terminate();
		}
	}
	return 0;
}

//--------------------------------------------------------------------------
/**
* GetInfoFromCreateEnityRequest
*/
bool SpatialOSServer::GetInfoFromCreateEnityRequest(const worker::RequestId<worker::CreateEntityRequest>& entity_creation_request_id, entity_info_t& entity_info_to_fill )
{
	for( entity_info_t& entity : GetInstance()->entity_info_list )
	{
		if( entity.entity_creation_request_id == entity_creation_request_id )
		{
			entity_info_to_fill = entity;
			return true;
		}
	}

	return false;
}

//--------------------------------------------------------------------------
/**
* GetInfoFromDeleteEnityRequest
*/
bool SpatialOSServer::GetInfoFromDeleteEnityRequest(const worker::RequestId<worker::DeleteEntityRequest>& entity_deletion_request_id, entity_info_t& entity_info_to_fill )
{
	for (entity_info_t& entity : GetInstance()->entity_info_list)
	{
		if  ( entity.entity_deletion_request_id == entity_deletion_request_id )
		{
			entity_info_to_fill = entity;
			return true;
		}
	}

	return false;
}

//--------------------------------------------------------------------------
/**
* GetInfoFromReserveEnityIdsRequest
*/
bool SpatialOSServer::GetInfoFromReserveEnityIdsRequest( const worker::RequestId<worker::ReserveEntityIdsRequest>& entity_id_reservation_request_id, entity_info_t& entity_info_to_fill )
{
	for (entity_info_t& entity : GetInstance()->entity_info_list)
	{
		if ( entity.entity_id_reservation_request_id == entity_id_reservation_request_id )
		{
			entity_info_to_fill = entity;
			return true;
		}
	}

	return false;
}

//--------------------------------------------------------------------------
/**
* GetInfoFromEnityId
*/
bool SpatialOSServer::GetInfoFromEnityId(const worker::EntityId& entity_id, entity_info_t& entity_info_to_fill)
{
	for (entity_info_t& entity : GetInstance()->entity_info_list)
	{
		if ( entity.id == entity_id )
		{
			entity_info_to_fill = entity;
			return true;
		}
	}

	return false;
}

//--------------------------------------------------------------------------
/**
* GetInstance
*/
SpatialOSServer* SpatialOSServer::GetInstance()
{
	static SpatialOSServer* s_system = new SpatialOSServer(); 
	return s_system;  
}

//--------------------------------------------------------------------------
/**
* SpatialOSSystem
*/
SpatialOSServer::SpatialOSServer()
{

}

//--------------------------------------------------------------------------
/**
* ~SpatialOSSystem
*/
SpatialOSServer::~SpatialOSServer()
{

}
