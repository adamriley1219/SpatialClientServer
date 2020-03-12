#include "Server/SpatialOSServer.hpp"

#include "Engine/Core/Strings/StringUtils.hpp"

#include "Shared/EntityBase.hpp"
#include "Shared/ActorBase.hpp"
#include "Shared/Zone.hpp"
#include "Shared/SimController.hpp"

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <iostream>

#include "Server/WorldSim.hpp"

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Time/Time.hpp"

#include "Server/View.hpp"
#include "Server/ServerCommon.hpp"

typedef unsigned int uint;



using ComponentRegistry 
	= worker::Components<
	improbable::Position,
	improbable::EntityAcl,
	improbable::Metadata,
	improbable::Persistence,
	improbable::Interest,
	siren::PlayerControls,
	siren::ServerAPI
	>;

// Constants and parameters
const int ErrorExitStatus = 1;
const std::string kLoggerName = "SpatialOSServer.cpp";
const std::uint32_t kGetOpListTimeoutInMilliseconds = 100;

worker::Connection ConnectWithReceptionist(const std::string hostname,
	const std::uint16_t port,
	const std::string& worker_id,
	const worker::ConnectionParameters& connection_parameters) {
	std::cout << "Connecting to host: " << hostname <<  " port: " << port << " id: " << worker_id << std::endl;
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
	GetInstance()->server_thread = std::thread( Run, arguments );
	while (!GetInstance()->IsRunning())
	{
		std::this_thread::yield();
	}
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
* Process
*/
void SpatialOSServer::Process()
{
	auto op_list = GetInstance()->connection->GetOpList(0);
	GetInstance()->view->Process( op_list );

	GetInstance()->Update();
}

//--------------------------------------------------------------------------
/**
* RequestEntityCreation
*/
void SpatialOSServer::RequestEntityCreation( EntityBase* entity_to_create )
{
	std::cout << "Inside RequestEntityCreation" << std::endl;
	if( !IsRunning() )
	{
		std::cout << "Server not running, (SpatialOSServer::RequestEntityCreation)" << std::endl;
		return;
	}
	std::cout << "Server running, (SpatialOSServer::RequestEntityCreation)" << std::endl;
	GetInstance()->entity_info_list_lock.lock();

	entity_info_t info;
	// Reserve an entity ID.
	std::cout << "Requesting (Entity Creation)" << std::endl;
	info.entity_id_reservation_request_id = GetInstance()->connection->SendReserveEntityIdsRequest(1, {}).Id;

	GetInstance()->connection->SendLogMessage(worker::LogLevel::kInfo, kLoggerName, Stringf( "RequestEntityCreation successfully with ID: %u", info.entity_id_reservation_request_id ) );

	std::cout << "Request Sent (Entity Creation) With ResponseID: " << info.entity_id_reservation_request_id << std::endl;
	info.game_entity = entity_to_create;

	GetInstance()->entity_info_list.push_back( info );
	GetInstance()->entity_info_list_lock.unlock();
}

//--------------------------------------------------------------------------
/**
* RequestEntityDeletion
*/
void SpatialOSServer::RequestEntityDeletion( const worker::EntityId entity )
{
	GetInstance()->connection->SendDeleteEntityRequest( entity, 5000 );
}

//--------------------------------------------------------------------------
/**
* UpdatePosition
*/
void SpatialOSServer::UpdatePosition( EntityBase* entity )
{
//	std::cout << "SpatialOSServer::UpdatePosition" << std::endl;
	entity_info_t* info = GetInfoWithEnity( entity );

	if( info && info->created )
	{
		improbable::Position::Update posUpdate;
		Vec2 position = entity->GetPosition();
		improbable::Coordinates coords( position.x, 0.0f,  position.y );
		posUpdate.set_coords( coords );
		worker::UpdateParameters params;
//		std::cout << "sending update with: " << position.x << ", " << position.y << "with entityID: " << info->id << std::endl;  
		GetInstance()->connection->SendComponentUpdate<improbable::Position>( info->id, posUpdate, params );
	}
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

	worker::alpha::PlayerIdentityTokenResponse player_response;


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

	// Register callbacks and run the worker main loop.
	View view( ComponentRegistry{} );
	GetInstance()->view = &view;
	bool is_connected = connection.IsConnected();

	std::cout << "Dispatcher done" << std::endl;

	view.OnDisconnect([&](const worker::DisconnectOp& op) {
		std::cerr << "[disconnect] " << op.Reason << std::endl;
		is_connected = false;
		GetInstance()->isRunning = false;
		});
	

	RegisterCallbacks( view );

	if (is_connected) {
		std::cout << "[local] Connected successfully to SpatialOS, listening to ops... " << std::endl;
		GetInstance()->isRunning = true;
	}

	constexpr unsigned kFramesPerSecond = 60;
	constexpr std::chrono::duration<double> kFramePeriodSeconds{
		1. / static_cast<double>(kFramesPerSecond) };

	std::cout << "Begin running loop" << std::endl;
	GetInstance()->isRunning = true;
	while (is_connected && IsRunning())
	{
		std::this_thread::sleep_for( kFramePeriodSeconds );
	}

	
	std::cout << "Finish Running" << std::endl;


	GetInstance()->isRunning = false;
	GetInstance()->connection = nullptr;
	GetInstance()->view = nullptr;
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
 
	// When the reservation succeeds, create an entity with the reserved ID.
	dispatcher.OnReserveEntityIdsResponse( ReserveEntityIdsResponse );

	// When the creation succeeds, delete the entity.
	dispatcher.OnCreateEntityResponse( CreateEntityResponse );

	// When the deletion succeeds, we're done.
	dispatcher.OnDeleteEntityResponse( DeleteEntityResponse );

	// For requesting an entity to be created, used for client to enter.
	dispatcher.OnCommandRequest<CreateClientEntity>( PlayerCreation );
	dispatcher.OnCommandRequest<DeleteClientEntity>( PlayerDeletion );
}

//--------------------------------------------------------------------------
/**
* Update
*/
void SpatialOSServer::Update()
{
	for( auto& ent_pair : view->m_entities )
	{
		View::entity_tracker_t& tracker = ent_pair.second;
		if (!tracker.updated)
		{
			// Entity wasn't updated so there's nothing to tell the client
			continue;
		}

		entity_info_t* info = GetInfoWithEnityId(ent_pair.first);
		if (info)
		{
			if (info->game_entity && info->created)
			{
				UpdateEntityWithWorkerEntity(*(info->game_entity), tracker.worker_entity, view->m_component_authority[ent_pair.first]);
			}
			else
			{
				if (info->created)
				{
					std::cout << "Waiting: SpatialOSServer::Update - info found but not created yet| entity ID:" << ent_pair.first << std::endl;
				}
				else
				{
					// should never get here.
					std::cout << "ERROR: SpatialOSServer::Update - info and worker entity found but no game entity to update| entity ID:" << ent_pair.first << std::endl;
				}
			}
		}
		else
		{
			// Game doesn't know about the instance from the server yet.

			//	Working on getting an entity into the game from the server
			std::cout << "SpatialOSServer::Update can't find info for id: " << ent_pair.first << std::endl;
			std::cout << "	Thread: " << std::this_thread::get_id() << std::endl;
			worker::Option<improbable::MetadataData&> data = tracker.worker_entity.Get<improbable::Metadata>();
			if (data)
			{
				std::string name = data->entity_type();
				std::cout << "	SpatialOSServer::Update Making from data: " << name.c_str() << std::endl;
				entity_info_t new_info;
				new_info.game_entity = g_theSim->CreateSimulatedEntity(name);
				if (new_info.game_entity)
				{
					InitEntityWithWorkerEntity(*(new_info.game_entity), tracker.worker_entity );
					new_info.id = ent_pair.first;
					new_info.created = true;
					GetInstance()->entity_info_list.push_back( new_info );
					std::cout << "	SpatialOSServer::Update Success in creation of entity| ID: " << ent_pair.first << std::endl;
				}
			}
		}
		tracker.updated = false;
	}

	// Check for deleted entities
	for ( uint idx = 0; idx < entity_info_list.size(); )
	{
		entity_info_t& info = entity_info_list[idx];
		// See if we can no longer see an entity that has been created
		auto itr = view->m_entities.find(info.id);
		bool entity_in_view_list = itr != view->m_entities.end();

		if( entity_in_view_list )
		{
			if( itr->second.garbage )
			{
				// Tell the entity to die and then erase knowledge of entity
				std::cout << "Killing entity with ID: " << info.id << std::endl;
				info.game_entity->Die();
				entity_info_list.erase( entity_info_list.begin() + idx );
				continue;
			}
		}
		++idx;
	}

	view->CleanupGarbage();
}

//--------------------------------------------------------------------------
/**
* UpdateEntityWithWorkerEntity
*/
void SpatialOSServer::UpdateEntityWithWorkerEntity( EntityBase& entity, worker::Entity& worker_entity, worker::Map<worker::ComponentId, worker::Authority>& entity_auth )
{
	worker::Option<improbable::PositionData&> pos = worker_entity.Get<improbable::Position>();
	
	if ( pos )
	{
		// Only update the position if I don't have authority over it's position.
		const auto& pos_auth_itr = entity_auth.find( improbable::Position::ComponentId );
		worker::Authority& pos_auth = pos_auth_itr->second;
		if( pos_auth == worker::Authority::kNotAuthoritative )
		{
			entity.SetPosition((float)pos->coords().x(), (float)pos->coords().z());
		}
	}
	worker::Option<siren::PlayerControlsData&> input_from_player = worker_entity.Get<siren::PlayerControls>();
	if( input_from_player )
	{
		// If I have authority over the position of the entity, update it's movement.
		const auto& pos_auth_itr = entity_auth.find(improbable::Position::ComponentId);
		worker::Authority& pos_auth = pos_auth_itr->second;
		if (pos_auth == worker::Authority::kAuthoritative || pos_auth == worker::Authority::kAuthorityLossImminent )
		{
			( (SimController*)( (ActorBase*) &entity )->GetController() )->SetMoveDirection( 
					Vec2( input_from_player->x_move(), input_from_player->y_move() )
				);
		}
	}
}

//--------------------------------------------------------------------------
/**
* InitEntityWithWorkerEntity
*/
void SpatialOSServer::InitEntityWithWorkerEntity( EntityBase& entity, worker::Entity& worker_entity )
{
	worker::Option<improbable::PositionData&> pos = worker_entity.Get<improbable::Position>();

	if (pos)
	{
		entity.SetPosition((float)pos->coords().x(), (float)pos->coords().z());
	}
	worker::Option<siren::PlayerControlsData&> input_from_player = worker_entity.Get<siren::PlayerControls>();
	if (input_from_player)
	{
		((SimController*)((ActorBase*)&entity)->GetController())->SetMoveDirection(
			Vec2(input_from_player->x_move(), input_from_player->y_move())
		);
	}
}

//--------------------------------------------------------------------------
/**
* DeleteEntityResponse
*/
uint64_t SpatialOSServer::DeleteEntityResponse( const worker::DeleteEntityResponseOp& op )
{
	entity_info_t* entity_info;
	if ( ( entity_info = GetInfoWithDeleteEnityRequest( op.RequestId.Id ) ) != nullptr  &&
		op.StatusCode == worker::StatusCode::kSuccess ) 
	{
		entity_info->created = false;
		entity_info->game_entity->Die();
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
	std::cout << "SpatialOSServer::CreateEntityResponse" << std::endl;
	entity_info = GetInfoWithCreateEnityRequest( op.RequestId.Id );
	bool success = op.StatusCode == worker::StatusCode::kSuccess;

	if ( entity_info && success ) 
	{
		if( entity_info->id != 0 )
		{
			if( entity_info->id != *(op.EntityId) )
			{
				std::cout << "	ID: " << entity_info->id << " assigned to entity was not the ID: " << *(op.EntityId) <<" returned in the callback." << std::endl;
			}
		}
		entity_info->id = *(op.EntityId); 
		entity_info->created = true;
		std::cout << "	Entity creation successful with id " << entity_info->id << std::endl;
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
	std::cout << "    Additionally: " << op.Message << " with id: " << op.RequestId.Id << std::endl;

	GetInstance()->connection->SendLogMessage(worker::LogLevel::kInfo, kLoggerName, Stringf("Connected %s", op.StatusCode == worker::StatusCode::kSuccess ? "successfully" : "with fault" ) );
	
	if ( ( entity_info = GetInfoWithReserveEnityIdsRequest( op.RequestId.Id ) ) != nullptr &&
		op.StatusCode == worker::StatusCode::kSuccess ) 
	{
		// Send response back to however sent the command if triggered by a command.
		if( entity_info->command_response_id != (uint64_t)-1 )
		{
			std::cout << "		Sending back response with ID: " << entity_info->command_response_id << std::endl;
			worker::RequestId<worker::IncomingCommandRequest< CreateClientEntity > > command_response;
			CreateClientEntity::Response response;
			worker::EntityId id = *(op.FirstEntityId); // Optional but can assume there is an id because the status was successful.
			response.set_id_created( id );
			command_response.Id = entity_info->command_response_id;
			GetInstance()->connection->SendCommandResponse<CreateClientEntity>( command_response, response );
		}

		worker::Entity clientEntity;

		Vec2 position = entity_info->game_entity->GetPosition();
		clientEntity.Add<improbable::Position>({ { position.x,  0.0f, position.y } });


		worker::List<std::string> callerWorkerAttributeSet{ "workerId:" + entity_info->owner_id };
		worker::List<std::string> simulationWorkerAttributeSet{ "simulation" };
		worker::List<std::string> clientWorkerAttributeSet{ "client" };

		improbable::WorkerRequirementSet clientWorkerRequirementSet{ worker::List<improbable::WorkerAttributeSet>{ {clientWorkerAttributeSet} } };
		improbable::WorkerRequirementSet simulationWorkerRequirementSet{ worker::List<improbable::WorkerAttributeSet>{ {simulationWorkerAttributeSet} } };
		improbable::WorkerRequirementSet callerWorkerRequirementSet{ worker::List<improbable::WorkerAttributeSet>{ {callerWorkerAttributeSet} } };
		improbable::WorkerRequirementSet clientOrSimRequirementSet
		{
			worker::List<improbable::WorkerAttributeSet>
		{
			simulationWorkerAttributeSet,
				clientWorkerAttributeSet,
				callerWorkerAttributeSet
		}
		};

		worker::Map<worker::ComponentId, improbable::WorkerRequirementSet> componentAcl;
		componentAcl[improbable::Position::ComponentId] = simulationWorkerRequirementSet;
		componentAcl[improbable::EntityAcl::ComponentId] = simulationWorkerRequirementSet;
		componentAcl[improbable::Metadata::ComponentId] = simulationWorkerRequirementSet;
		componentAcl[siren::PlayerControls::ComponentId] = callerWorkerRequirementSet;

		clientEntity.Add<improbable::EntityAcl>(
			improbable::EntityAcl::Data{/* read */ clientOrSimRequirementSet, /* write */ componentAcl });

		improbable::Metadata::Data metadata;
		metadata.set_entity_type( entity_info->game_entity->GetName() );
		clientEntity.Add<improbable::Metadata>(metadata);

		siren::PlayerControls::Data client_data;
		clientEntity.Add<siren::PlayerControls>(client_data);

		improbable::ComponentInterest::QueryConstraint relativeConstraint;
		relativeConstraint.set_relative_box_constraint({ {{20.5, 9999, 20.5}} });
		improbable::ComponentInterest::Query relativeQuery;
		relativeQuery.set_constraint(relativeConstraint);
		relativeQuery.set_full_snapshot_result({ true });
		improbable::ComponentInterest interest{ {relativeQuery} };
		clientEntity.Add<improbable::Interest>({ {{siren::Client::ComponentId, interest}} });
		
		entity_info->id = *op.FirstEntityId;
		auto result = GetInstance()->connection->SendCreateEntityRequest( clientEntity, op.FirstEntityId, kGetOpListTimeoutInMilliseconds );
		// Check no errors occurred.
		if (result) {
			entity_info->entity_creation_request_id = (*result).Id;
			std::cout << "		Creating entity with id: " << entity_info->id << " with requestID: " << entity_info->entity_creation_request_id << std::endl;
			std::cout << "		ReserveEntity Success with id: " << entity_info->entity_creation_request_id << std::endl;
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
entity_info_t* SpatialOSServer::GetInfoWithCreateEnityRequest( uint64_t entity_creation_request_id )
{
	std::lock_guard<std::mutex> lg(  GetInstance()->entity_info_list_lock );
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
entity_info_t* SpatialOSServer::GetInfoWithDeleteEnityRequest( uint64_t entity_deletion_request_id )
{
	std::lock_guard<std::mutex> lg(GetInstance()->entity_info_list_lock);
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
entity_info_t* SpatialOSServer::GetInfoWithReserveEnityIdsRequest( uint64_t entity_id_reservation_request_id )
{
	std::lock_guard<std::mutex> lg(GetInstance()->entity_info_list_lock);
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
entity_info_t* SpatialOSServer::GetInfoWithEnityId(const worker::EntityId& entity_id )
{
	std::lock_guard<std::mutex> lg(GetInstance()->entity_info_list_lock);
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
* GetInfoFromEnity
*/
entity_info_t* SpatialOSServer::GetInfoWithEnity( EntityBase* entity_id )
{
	std::lock_guard<std::mutex> lg(GetInstance()->entity_info_list_lock);
	for ( entity_info_t& entity : GetInstance()->entity_info_list )
	{
		if ( entity.game_entity == entity_id )
		{
			return &entity;
		}
	}

	return nullptr;
}

//--------------------------------------------------------------------------
/**
* PrintAllEntityinfos
*/
void SpatialOSServer::PrintAllEntityinfos()
{
	std::lock_guard<std::mutex> lg(GetInstance()->entity_info_list_lock);
	std::cout << "Print all Entities in spatial server" << std::endl;
	std::cout << "	Thread: " << std::this_thread::get_id() << std::endl;

	for( entity_info_t& info : GetInstance()->entity_info_list )
	{
		std::cout << "	Info id: " << info.id << std::endl;
		std::cout << "		created: " << ( info.created ? "Yes" : "No" ) << std::endl;
		std::cout << "		updated: " << ( info.updated ? "Yes" : "No" ) << std::endl;
	}
}

//--------------------------------------------------------------------------
/**
* EntityCreation
*/
void SpatialOSServer::PlayerCreation( const worker::CommandRequestOp<CreateClientEntity>& op )
{
	// ID reservation was successful - create an entity with the reserved ID.
	//--------------------------------------------------------------------------
	std::cout << "Received a command request from: " << op.CallerWorkerId << std::endl;
	
	EntityBase* base = g_theSim->CreateSimulatedEntity( "player" );
	base->SetPosition( Vec2( -1.0f, 0.0f ) );
	SpatialOSServer::RequestEntityCreation( base );

	entity_info_t* info = GetInfoWithEnity( base );

	if( info )
	{
		info->owner_id = op.CallerWorkerId;
		info->id = op.Request.id_to_create();
		info->game_entity->SetPosition( 1.0f, 1.0f );

		std::cout << "entity sent successfully" << std::endl;

		// For responding to the command when an ID has been obtained.
		info->command_response_id = op.RequestId.Id;
		std::cout << "player creation command response id: " << op.RequestId.Id << std::endl;
	}
	else
	{
		std::cout << "Warning: SpatialOSServer::PlayerCreation failed to create the info for the player." << std::endl;
	}
}

//--------------------------------------------------------------------------
/**
* PlayerDeletion
*/
void SpatialOSServer::PlayerDeletion(const worker::CommandRequestOp<DeleteClientEntity>& op)
{
	std::cout << "SpatialOSServer::PlayerDeletion | Deleting ID: " << op.Request.id_to_delete() << std::endl;
	SpatialOSServer::RequestEntityDeletion( op.Request.id_to_delete() );
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
