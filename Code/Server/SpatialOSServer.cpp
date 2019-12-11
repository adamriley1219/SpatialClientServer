#include "Server/SpatialOSServer.hpp"

#include "Engine/Core/Strings/StringUtils.hpp"

#include "Shared/EntityBase.hpp"
#include "Shared/ActorBase.hpp"

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <iostream>


#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Time/Time.hpp"

typedef unsigned int uint;

// Use this to make a worker::ComponentRegistry.
// For example use worker::Components<improbable::Position, improbable::Metadata> to track these common components
using ComponentRegistry = worker::Components<improbable::Position, improbable::EntityAcl, improbable::Metadata>;

// Constants and parameters
const int ErrorExitStatus = 1;
const std::string kLoggerName = "SpatialOSServer.cpp";
const std::uint32_t kGetOpListTimeoutInMilliseconds = 100;

worker::Connection ConnectWithReceptionist(const std::string hostname,
	const std::uint16_t port,
	const std::string& worker_id,
	const worker::ConnectionParameters& connection_parameters) {
	std::cout << "Connectoing to host: " << hostname <<  " port: " << port << " id: " << worker_id;
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
	//Run( arguments );
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
uint64_t test_id;
//--------------------------------------------------------------------------
/**
* RequestEntityCreation
*/
void SpatialOSServer::RequestEntityCreation( EntityBase* entity_to_create )
{
	if( !IsRunning() )
	{
		return;
	}

	entity_info_t info;
	// Reserve an entity ID.
	std::cout << "Requesting (Entity Creation)" << std::endl;
	info.entity_id_reservation_request_id = GetInstance()->connection->SendReserveEntityIdsRequest(1, {}).Id;

	GetInstance()->connection->SendLogMessage(worker::LogLevel::kInfo, kLoggerName, Stringf( "RequestEntityCreation successfully with ID: %u", info.entity_id_reservation_request_id ) );

	std::cout << "Request Sent (Entity Creation) With ResponseID: " << info.entity_id_reservation_request_id << std::endl;
	info.entity = entity_to_create;

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

	if ( arguments.size() < 3 ) {
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
	if (arguments.size() >= 4) {
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

	std::cout << "connection done" << std::endl;
	const worker::Components<
		improbable::EntityAcl,
		improbable::Position> MyComponents;
	// Register callbacks and run the worker main loop.
	worker::Dispatcher dispatcher( ComponentRegistry{} );
	GetInstance()->dispatcher = &dispatcher;
	bool is_connected = connection.IsConnected();

	std::cout << "Dispatcher done" << std::endl;

	dispatcher.OnDisconnect([&](const worker::DisconnectOp& op) {
		std::cerr << "[disconnect] " << op.Reason << std::endl;
		is_connected = false;
		GetInstance()->isRunning = false;
		});
	

	RegisterCallbacks( dispatcher );

	if (is_connected) {
		std::cout << "[local] Connected successfully to SpatialOS, listening to ops... " << std::endl;
		GetInstance()->isRunning = true;
	}
	std::cout << "Attempting to create entity" << std::endl;
	EntityBase* entity = new ActorBase("turret", 0);
	std::cout << "Successfully newed off and entity" << std::endl;
	RequestEntityCreation( entity );

	constexpr unsigned kFramesPerSecond = 60;
	constexpr std::chrono::duration<double> kFramePeriodSeconds{
		1. / static_cast<double>(kFramesPerSecond) };

	std::cout << "Begin running loop" << std::endl;
	while (is_connected && IsRunning())
	{
		auto start_time = std::chrono::steady_clock::now();

		auto op_list = GetInstance()->connection->GetOpList(0);
		GetInstance()->dispatcher->Process( op_list );

 		// Temp code for testing
		entity_info_t* info;
		if( ( info = GetInfoFromEnityId( test_id ) ) && info->created )
		{
			Vec2 curPos = info->entity->GetPosition();
			float offset = 0.02f;
			std::cout << "Set     entCords      for " << info->id << ":" << curPos.x << "," << curPos.y << std::endl;
			improbable::Position::Update posUpdate;
			float pos_to_set = curPos.y + offset;
			if( pos_to_set > 8 )
			{
				pos_to_set = 0;
			}
			posUpdate.set_coords( improbable::Coordinates( curPos.x, 0.0f, pos_to_set + offset ) );
			std::cout << "Set     Update Corrds for " << info->id << " x: " << posUpdate.coords()->x() << ", y: " << posUpdate.coords()->y() << ", z: " << posUpdate.coords()->z() << std::endl;
			connection.SendComponentUpdate<improbable::Position>( info->id, posUpdate );
		}

		auto end_time = std::chrono::steady_clock::now();
		auto wait_for = kFramePeriodSeconds - ( end_time - start_time );
		std::this_thread::sleep_for( wait_for );
	}

	
	std::cout << "Finish Running" << std::endl;


	GetInstance()->isRunning = false;
	GetInstance()->connection = nullptr;
	GetInstance()->dispatcher = nullptr;
}

//--------------------------------------------------------------------------
/**
* RegisterCallbacks
*/
void SpatialOSServer::RegisterCallbacks( worker::Dispatcher& dispatcher )
{
	// Print log messages received from SpatialOS
	dispatcher.OnLogMessage([&](const worker::LogMessageOp& op) {
		if (op.Level == worker::LogLevel::kFatal) {
			std::cerr << "Fatal error: " << op.Message << std::endl;
			std::terminate();
		}
		std::cout << "[remote] " << op.Message << std::endl;
		});

	dispatcher.OnRemoveEntity([&](const worker::RemoveEntityOp& op)
		{
			std::cout << "Removed entity " << op.EntityId << std::endl;
		});
	dispatcher.OnAddEntity([&]( const worker::AddEntityOp& op )
		{
			std::cout << "Added entity " << op.EntityId << std::endl;
		});
 
	// When the reservation succeeds, create an entity with the reserved ID.
	dispatcher.OnReserveEntityIdsResponse( ReserveEntityIdsResponse );

	// When the creation succeeds, delete the entity.
	dispatcher.OnCreateEntityResponse( CreateEntityResponse );

	// When the deletion succeeds, we're done.
	dispatcher.OnDeleteEntityResponse( DeleteEntityResponse );

	// For updating components
	dispatcher.OnComponentUpdate<improbable::Position>( PositionUpdated );
}

//--------------------------------------------------------------------------
/**
* DeleteEntityResponse
*/
uint64_t SpatialOSServer::DeleteEntityResponse( const worker::DeleteEntityResponseOp& op )
{
	entity_info_t* entity_info;
	if ( ( entity_info = GetInfoFromDeleteEnityRequest( op.RequestId.Id ) ) != nullptr  &&
		op.StatusCode == worker::StatusCode::kSuccess ) 
	{
		entity_info->created = false;
		delete( entity_info->entity );
		entity_info->entity = nullptr;
	}
	return op.RequestId.Id;
}

//--------------------------------------------------------------------------
/**
* CreateEntityResponse
*/
uint64_t SpatialOSServer::CreateEntityResponse( const worker::CreateEntityResponseOp& op )
{
	entity_info_t* entity_info;
	std::cout << "Creating Entity" << std::endl;
	entity_info = GetInfoFromCreateEnityRequest( op.RequestId.Id );
	bool success = op.StatusCode == worker::StatusCode::kSuccess;
	
	if( entity_info )
	{
		std::cout << "infoGrab Success" << std::endl;
	}
	else
	{
		std::cout << "infoGrab failed with id: " << op.RequestId.Id << std::endl;
	}
	if( success )
	{
		std::cout << "success Success" << std::endl;
	}

	if ( entity_info && success ) 
	{
		entity_info->id = *(op.EntityId);
		entity_info->created = true;
		test_id = entity_info->id;
		std::cout << "Entity creation successful with id " << test_id << std::endl;
	}
	return op.RequestId.Id;
}

//--------------------------------------------------------------------------
/**
* ReserveEntityIdsResponse
*/
uint64_t SpatialOSServer::ReserveEntityIdsResponse( const worker::ReserveEntityIdsResponseOp& op )
{
	entity_info_t* entity_info;
	std::cout << "ReserveEntity begin with response ID: " << op.RequestId.Id << std::endl;
	std::cout << "    Additionally: " << op.Message << " weith id: " << op.RequestId.Id << std::endl;

	GetInstance()->connection->SendLogMessage(worker::LogLevel::kInfo, kLoggerName, Stringf("Connected %s", op.StatusCode == worker::StatusCode::kSuccess ? "successfully" : "with fault" ) );

	if ( ( entity_info = GetInfoFromReserveEnityIdsRequest( op.RequestId.Id ) ) != nullptr &&
		op.StatusCode == worker::StatusCode::kSuccess ) 
	{

		worker::Entity entity;
		Vec2 position = entity_info->entity->GetPosition();
		entity.Add<improbable::Position>({ { position.x,  0.0f, position.y } });

		worker::List<std::string> simulationWorkerAttributeSet{ "simulation" };

		// This requirement set matches any worker with the attribute "physics".
		improbable::WorkerRequirementSet simulationWorkerRequirementSet{
			worker::List<improbable::WorkerAttributeSet>{ {simulationWorkerAttributeSet}} };

		// This requirement set matches any worker with the attribute "client" or "physics".
		improbable::WorkerRequirementSet clientOrPhysicsRequirementSet
		{
			worker::List<improbable::WorkerAttributeSet>
		{
			improbable::WorkerAttributeSet{worker::List<std::string>{"client"}},
				simulationWorkerAttributeSet
		}
		};

		// Give authority over Position and EntityAcl to any physics worker, and over PlayerControls to
		// the caller worker.
		worker::Map<worker::ComponentId, improbable::WorkerRequirementSet> componentAcl
		{
			{
				improbable::Position::ComponentId, simulationWorkerRequirementSet
			},
		{
			improbable::EntityAcl::ComponentId, simulationWorkerRequirementSet
		}
		};

		entity.Add<improbable::EntityAcl>(
			improbable::EntityAcl::Data{/* read */ clientOrPhysicsRequirementSet, /* write */ componentAcl });


		auto result = GetInstance()->connection->SendCreateEntityRequest( entity, op.FirstEntityId, kGetOpListTimeoutInMilliseconds );
		// Check no errors occurred.
		if (result) {
			entity_info->entity_creation_request_id = (*result).Id;
			std::cout << "ReserveEntity Success with id: " << entity_info->entity_creation_request_id << std::endl;
		}
		else {
			GetInstance()->connection->SendLogMessage(worker::LogLevel::kError, "ReserveEntityIdsResponse Error",
				result.GetErrorMessage());
			std::terminate();
		}
	}
	return op.RequestId.Id;
}

//--------------------------------------------------------------------------
/**
* GetInfoFromCreateEnityRequest
*/
entity_info_t* SpatialOSServer::GetInfoFromCreateEnityRequest( uint64_t entity_creation_request_id )
{
	for( entity_info_t& entity : GetInstance()->entity_info_list )
	{
		std::cout << "GetInfoFromCreateEnityRequest: " << entity.entity_creation_request_id << " with " << entity_creation_request_id << std::endl;

		if( entity.entity_creation_request_id == entity_creation_request_id )
		{
			return &entity;
		}
	}

	return nullptr;
}

//--------------------------------------------------------------------------
/**
* GetInfoFromDeleteEnityRequest
*/
entity_info_t* SpatialOSServer::GetInfoFromDeleteEnityRequest( uint64_t entity_deletion_request_id )
{
	for (entity_info_t& entity : GetInstance()->entity_info_list)
	{
		if  ( entity.entity_deletion_request_id == entity_deletion_request_id )
		{
			return &entity;
		}
	}

	return nullptr;
}

//--------------------------------------------------------------------------
/**
* GetInfoFromReserveEnityIdsRequest
*/
entity_info_t* SpatialOSServer::GetInfoFromReserveEnityIdsRequest( uint64_t entity_id_reservation_request_id )
{
	for (entity_info_t& entity : GetInstance()->entity_info_list)
	{
		if ( entity.entity_id_reservation_request_id == entity_id_reservation_request_id )
		{
			return &entity;
		}
	}
	return nullptr;
}

//--------------------------------------------------------------------------
/**
* GetInfoFromEnityId
*/
entity_info_t* SpatialOSServer::GetInfoFromEnityId(const worker::EntityId& entity_id )
{
	for (entity_info_t& entity : GetInstance()->entity_info_list)
	{
		if ( entity.id == entity_id )
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
void SpatialOSServer::PositionUpdated (const worker::ComponentUpdateOp<improbable::Position>& op )
{
	std::cout << "SpatialOSServer::PositionUpdated" << std::endl;
	const auto coords = op.Update.coords().data();
	std::cout << "Updated position of entity " << op.EntityId << " to "
		<< " x: " << coords->x() << " y: " << coords->y() << " z: " << coords->z() << std::endl;
	entity_info_t* info = GetInfoFromEnityId( op.EntityId );
	
	if( info && info->created )
	{
		info->entity->SetPosition( coords->x(), coords->z() );

		std::cout << "PosUpdate: Success" << std::endl;
	}
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
