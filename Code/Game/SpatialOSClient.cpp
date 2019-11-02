#include "Game/SpatialOSClient.hpp"

#include "Engine/Core/EngineCommon.hpp"

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
* RequestEntityCreation
*/
void SpatialOSClient::RequestEntityCreation( worker::Entity* entity )
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

	GetInstance()->entity_info_list.push_back(info);
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
	worker::Dispatcher dispatcher{ EmptyRegistry{} };
	GetInstance()->dispatcher = &dispatcher;
	bool is_connected = connection.IsConnected();

	dispatcher.OnDisconnect([&](const worker::DisconnectOp& op) {
		std::cerr << "[disconnect] " << op.Reason << std::endl;
		is_connected = false;
		});

	// Print messages received from SpatialOS
	dispatcher.OnLogMessage([&](const worker::LogMessageOp& op) {
		if (op.Level == worker::LogLevel::kFatal) {
			std::cerr << "Fatal error: " << op.Message << std::endl;
			std::terminate();
		}
		std::cout << "Connection: " << op.Message << std::endl;
		});

	// When the reservation succeeds, create an entity with the reserved ID.
	dispatcher.OnReserveEntityIdsResponse( ReserveEntityIdsResponse );

	// When the creation succeeds, delete the entity.
	dispatcher.OnCreateEntityResponse( CreateEntityResponse );

	// When the deletion succeeds, we're done.
	dispatcher.OnDeleteEntityResponse( DeleteEntityResponse );

	dispatcher.OnRemoveEntity( [&](const worker::RemoveEntityOp& op )
		{

		});
	dispatcher.OnAddEntity( [&](const worker::AddEntityOp& op )
		{
			op.EntityId;
			
		});

	dispatcher.OnCreateEntityResponse([&]( const worker::CreateEntityResponseOp& op )
		{

		});

	

	//dispatcher.OnComponentUpdate();
	//dispatcher.OnDeleteEntityResponse();

	worker::Entity entity;
	RequestEntityCreation( &entity );

	while ( GetInstance()->connection->IsConnected() && IsRunning() )
	{
		dispatcher.Process(connection.GetOpList(kGetOpListTimeoutInMilliseconds));
	}

	if( GetInstance()->connection->IsConnected() )
	{
		entity_info_t info;
		if( GetInfoFromEnity( &entity, info ) )
		{
			connection.SendDeleteEntityRequest( info.id, kGetOpListTimeoutInMilliseconds );
		}
	}

	GetInstance()->isRunning = false;
	GetInstance()->connection = nullptr;
	GetInstance()->dispatcher = nullptr;
}


//--------------------------------------------------------------------------
/**
* DeleteEntityResponse
*/
uint64_t SpatialOSClient::DeleteEntityResponse(const worker::DeleteEntityResponseOp& op)
{
	entity_info_t entity_info;
	if (GetInfoFromEnityId( op.EntityId, entity_info ) &&
		op.StatusCode == worker::StatusCode::kSuccess)
	{
		// Test successful!
		SAFE_DELETE(entity_info.entity);
	}
	return 0;
}

//--------------------------------------------------------------------------
/**
* CreateEntityResponse
*/
uint64_t SpatialOSClient::CreateEntityResponse(const worker::CreateEntityResponseOp& op)
{
	entity_info_t entity_info;
	if (GetInfoFromCreateEnityRequest(op.RequestId, entity_info) &&
		op.StatusCode == worker::StatusCode::kSuccess)
	{
		entity_info.id = *op.EntityId;
	}
	return 0;
}

//--------------------------------------------------------------------------
/**
* ReserveEntityIdsResponse
*/
uint64_t SpatialOSClient::ReserveEntityIdsResponse(const worker::ReserveEntityIdsResponseOp& op)
{
	entity_info_t entity_info;

	if (GetInfoFromReserveEnityIdsRequest(op.RequestId, entity_info) &&
		op.StatusCode == worker::StatusCode::kSuccess) {
		// ID reservation was successful - create an entity with the reserved ID.
		entity_info.entity->Add<improbable::Position>({ {1, 2, 3} });
		auto result = GetInstance()->connection->SendCreateEntityRequest(*(entity_info.entity), op.FirstEntityId, kGetOpListTimeoutInMilliseconds);
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
bool SpatialOSClient::GetInfoFromCreateEnityRequest(const worker::RequestId<worker::CreateEntityRequest>& entity_creation_request_id, entity_info_t& entity_info_to_fill)
{
	for (entity_info_t& entity : GetInstance()->entity_info_list)
	{
		if (entity.entity_creation_request_id == entity_creation_request_id)
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
bool SpatialOSClient::GetInfoFromDeleteEnityRequest(const worker::RequestId<worker::DeleteEntityRequest>& entity_deletion_request_id, entity_info_t& entity_info_to_fill)
{
	for (entity_info_t& entity : GetInstance()->entity_info_list)
	{
		if (entity.entity_deletion_request_id == entity_deletion_request_id)
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
bool SpatialOSClient::GetInfoFromReserveEnityIdsRequest(const worker::RequestId<worker::ReserveEntityIdsRequest>& entity_id_reservation_request_id, entity_info_t& entity_info_to_fill)
{
	for (entity_info_t& entity : GetInstance()->entity_info_list)
	{
		if (entity.entity_id_reservation_request_id == entity_id_reservation_request_id)
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
bool SpatialOSClient::GetInfoFromEnityId(const worker::EntityId& entity_id, entity_info_t& entity_info_to_fill)
{
	for (entity_info_t& entity : GetInstance()->entity_info_list)
	{
		if (entity.id == entity_id)
		{
			entity_info_to_fill = entity;
			return true;
		}
	}

	return false;
}


//--------------------------------------------------------------------------
/**
* GetInfoFromEnity
*/
bool SpatialOSClient::GetInfoFromEnity(const worker::Entity* entity_to_find, entity_info_t& entity_info_to_fill)
{
	for ( entity_info_t& entity : GetInstance()->entity_info_list )
	{
		if ( entity.entity == entity_to_find )
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
