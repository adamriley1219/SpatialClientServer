#include "Server/SpatialOSServer.hpp"
#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <iostream>


//#include "Engine/Core/EngineCommon.hpp"
//#include "Engine/Core/Time/Time.hpp"

typedef unsigned int uint;

// Use this to make a worker::ComponentRegistry.
// For example use worker::Components<improbable::Position, improbable::Metadata> to track these common components
using ComponentRegistry = worker::Components<improbable::Position, improbable::Metadata>;

// Constants and parameters
const int ErrorExitStatus = 1;
const std::string kLoggerName = "SpatialOSServer.cpp";
const std::uint32_t kGetOpListTimeoutInMilliseconds = 100;

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
	Run( arguments );
	//GetInstance()->server_thread = std::thread( Run, arguments );
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
void SpatialOSServer::RequestEntityCreation( worker::Entity* entity )
{
	if( !IsRunning() )
	{
		return;
	}

	entity_info_t info;
	// Reserve an entity ID.
	std::cout << "Requesting (Entity Creation)" << std::endl;
	info.entity_id_reservation_request_id =
		GetInstance()->connection->SendReserveEntityIdsRequest(1, kGetOpListTimeoutInMilliseconds).Id;

	std::cout << "Request Sent (Entity Creation)" << std::endl;
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

	std::cout << "connection done" << std::endl;
	const worker::Components<
		improbable::EntityAcl,
		improbable::Position> MyComponents;
	// Register callbacks and run the worker main loop.
	worker::Dispatcher dispatcher = worker::Dispatcher{  ComponentRegistry{} };
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

// 	worker::Entity entity;
// 	entity.Add<improbable::Position>({ { 7,7,7 } });
// 
// 	RequestEntityCreation(&entity);

	constexpr unsigned kFramesPerSecond = 60;
	constexpr std::chrono::duration<double> kFramePeriodSeconds{
		1. / static_cast<double>(kFramesPerSecond) };

	while (connection.IsConnected() && IsRunning())
	{
		auto start_time = std::chrono::steady_clock::now();

		std::cout << "before getOPlist" << std::endl;
		auto opList = connection.GetOpList(0);
		std::cout << "after getOPlist" << std::endl;
		dispatcher.Process( opList );
		std::cout << "after Process" << std::endl;

 		// Temp code for testing
// 		entity_info_t* entity;
// 		if( ( entity = GetInfoFromEnityId( test_id ) ) && entity->created )
// 		{
// 			worker::Option<improbable::PositionData&> curPos = entity->entity->Get<improbable::Position>();
// 			float offset = SinDegrees( GetCurrentTimeSeconds() * 300.0 );
// 			std::cout << "Set     entCords      for " << entity->id << ":" << curPos->coords().x() << "," << curPos->coords().z() << "," << curPos->coords().y() << std::endl;
// 			improbable::Position::Update posUpdate;
// 			posUpdate.set_coords( curPos->coords() );
// 			posUpdate.coords()->set_x( 7 + offset );
// 			std::cout << "Set     Update Corrds for " << entity->id << ":" << posUpdate.coords()->x() << "," << posUpdate.coords()->z() << "," << posUpdate.coords()->y() << std::endl;
// 			connection.SendComponentUpdate<improbable::Position>( entity->id, posUpdate );
// 		}

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

// 	dispatcher.OnRemoveEntity([&](const worker::RemoveEntityOp& op)
// 		{
// 			std::cout << "Removed entity " << op.EntityId << std::endl;
// 		});
// 	dispatcher.OnAddEntity([&]( const worker::AddEntityOp& op )
// 		{
// 			std::cout << "Added entity " << op.EntityId << std::endl;
// 		});
// 
// 	// When the reservation succeeds, create an entity with the reserved ID.
// 	dispatcher.OnReserveEntityIdsResponse( ReserveEntityIdsResponse );
// 
// 	// When the creation succeeds, delete the entity.
// 	dispatcher.OnCreateEntityResponse( CreateEntityResponse );
// 
// 	// When the deletion succeeds, we're done.
// 	dispatcher.OnDeleteEntityResponse( DeleteEntityResponse );
// 
// 	// For updating components
// 	dispatcher.OnComponentUpdate<improbable::Position>( PositionUpdated );
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
		std::cout << "Entity creation successful with id" << test_id << std::endl;
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
	std::cout << "ReserveEntity begin" << std::endl;

	if ( ( entity_info = GetInfoFromReserveEnityIdsRequest( op.RequestId.Id ) ) != nullptr &&
		op.StatusCode == worker::StatusCode::kSuccess ) 
	{
		auto result = GetInstance()->connection->SendCreateEntityRequest( *(entity_info->entity), op.FirstEntityId, kGetOpListTimeoutInMilliseconds );
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
	entity_info_t* info = GetInfoFromEnityId( test_id );
	
	if( info && info->created )
	{
		worker::Option<improbable::PositionData&> curPos = info->entity->Get<improbable::Position>();
		std::cout << "PosUpdate: entCords for " << info->id << ":" << curPos->coords().x() << "," << curPos->coords().z() << "," << curPos->coords().y() << std::endl;
		curPos->coords().set_x( coords->x() );
		curPos->coords().set_y( coords->y() );
		curPos->coords().set_z( coords->z() );
		std::cout << "PosUpdate: entCords for " << info->id << ":" << curPos->coords().x() << "," << curPos->coords().z() << "," << curPos->coords().y() << std::endl;
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
