#pragma once


#include <improbable/worker.h>
#include <improbable/standard_library.h>

#include "ClientServer.h"

#include <thread>
#include <mutex>

class EntityBase;
class View;

using CreateClientEntity = siren::ServerAPI::Commands::CreateClientEntity;
using DeleteClientEntity = siren::ServerAPI::Commands::DeleteClientEntity;


struct entity_info_t
{
	EntityBase* game_entity = nullptr;
	worker::EntityId id = 0;
	uint64_t entity_creation_request_id = 0;
	uint64_t entity_deletion_request_id = 0;
	uint64_t entity_id_reservation_request_id = 0;
	uint64_t command_response_id = (uint64_t)-1;
	std::string owner_id = "";
	bool created = false;
	bool updated = false;
};

class SpatialOSServer
{
public:
	static void Startup( const std::vector<std::string>& args );
	static void Shutdown();

	static void Process();

public:
	static void RequestEntityCreation( EntityBase* entity );
	static void RequestEntityDeletion( const worker::EntityId entity );
	static void UpdatePosition( EntityBase *entity );
	static bool IsRunning();

private:
	static void Run( const std::vector<std::string> arguments );
	static void RegisterCallbacks( worker::Dispatcher& dispatcher );

private:
	void Update();
	static void UpdateEntityWithWorkerEntity( EntityBase& entity, worker::Entity& worker_entity, worker::Map<worker::ComponentId, worker::Authority>& auth );

private:
	static uint64_t DeleteEntityResponse( const worker::DeleteEntityResponseOp& op );
	static uint64_t CreateEntityResponse( const worker::CreateEntityResponseOp& op );
	static uint64_t ReserveEntityIdsResponse( const worker::ReserveEntityIdsResponseOp& op );

	static entity_info_t* GetInfoWithCreateEnityRequest( uint64_t entity_creation_request_id );
	static entity_info_t* GetInfoWithDeleteEnityRequest( uint64_t entity_deletion_request_id );
	static entity_info_t* GetInfoWithReserveEnityIdsRequest( uint64_t entity_id_reservation_request_id );
	static entity_info_t* GetInfoWithEnityId( const worker::EntityId& entity_id );
	static entity_info_t* GetInfoWithEnity( EntityBase* entity_id );

	static void PrintAllEntityinfos();

private:
	// Component Updating
	static void PlayerCreation( const worker::CommandRequestOp<CreateClientEntity>& op ); 
	static void PlayerDeletion( const worker::CommandRequestOp<DeleteClientEntity>& op ); 

private:
	static SpatialOSServer* GetInstance();
	SpatialOSServer();
	~SpatialOSServer();

private:
	bool isRunning = false;
	std::thread server_thread;

	View* view;

	worker::Connection* connection;

	std::mutex entity_info_list_lock;

	std::vector<entity_info_t> entity_info_list;
};