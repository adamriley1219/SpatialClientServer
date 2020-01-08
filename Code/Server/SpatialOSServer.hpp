#pragma once


#include <improbable/worker.h>
#include <improbable/standard_library.h>

#include "ClientServer.h"

#include <thread>
#include <mutex>

class EntityBase;

using CreateClientEntity = siren::ServerAPI::Commands::CreateClientEntity;


struct entity_info_t
{
	EntityBase* entity = nullptr;
	worker::EntityId id = 0;
	uint64_t entity_creation_request_id = 0;
	uint64_t entity_deletion_request_id = 0;
	uint64_t entity_id_reservation_request_id = 0;
	uint64_t command_response = 0;
	std::string owner_id = "";
	bool created = false;
	bool updated = false;
};

class SpatialOSServer
{
public:
	static void Startup( const std::vector<std::string>& args );
	static void Shutdown();

public:
	static void RequestEntityCreation( EntityBase* entity );
	static bool IsRunning();

private:
	static void Run( const std::vector<std::string> arguments );
	static void RegisterCallbacks( worker::Dispatcher& dispatcher );

	static uint64_t DeleteEntityResponse( const worker::DeleteEntityResponseOp& op );
	static uint64_t CreateEntityResponse( const worker::CreateEntityResponseOp& op );
	static uint64_t ReserveEntityIdsResponse( const worker::ReserveEntityIdsResponseOp& op );

	static entity_info_t* GetInfoFromCreateEnityRequest( uint64_t entity_creation_request_id );
	static entity_info_t* GetInfoFromDeleteEnityRequest( uint64_t entity_deletion_request_id );
	static entity_info_t* GetInfoFromReserveEnityIdsRequest( uint64_t entity_id_reservation_request_id );
	static entity_info_t* GetInfoFromEnityId( const worker::EntityId& entity_id );
	static entity_info_t* GetInfoFromEnity( EntityBase* entity_id );

private:
	// Component Updating
	static void PositionUpdated( const worker::ComponentUpdateOp<improbable::Position>& op );
	static void PlayerCreation( const worker::CommandRequestOp<CreateClientEntity>& op ); 

private:
	static SpatialOSServer* GetInstance();
	SpatialOSServer();
	~SpatialOSServer();

private:
	bool isRunning = false;
	std::thread server_thread;

	worker::Dispatcher* dispatcher;
	worker::Connection* connection;

	std::mutex entity_info_list_lock;

	std::vector<entity_info_t> entity_info_list;
	std::vector<EntityBase*> unclaimed_game_entities;
};