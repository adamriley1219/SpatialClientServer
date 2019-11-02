#pragma once


#include <improbable/worker.h>
#include <improbable/standard_library.h>

#include <thread>


struct entity_info_t
{
	worker::Entity* entity;
	worker::EntityId id;
	worker::RequestId<worker::CreateEntityRequest> entity_creation_request_id;
	worker::RequestId<worker::DeleteEntityRequest> entity_deletion_request_id;
	worker::RequestId<worker::ReserveEntityIdsRequest> entity_id_reservation_request_id;
};

class SpatialOSServer
{
public:
	static void Startup( const std::vector<std::string>& args );
	static void Shutdown();

public:
	static void RequestEntityCreation( worker::Entity* entity );
	static bool IsRunning();

private:
	static void Run( std::vector<std::string> arguments );

	static uint64_t DeleteEntityResponse( const worker::DeleteEntityResponseOp& op );
	static uint64_t CreateEntityResponse( const worker::CreateEntityResponseOp& op );
	static uint64_t ReserveEntityIdsResponse( const worker::ReserveEntityIdsResponseOp& op );

	static bool GetInfoFromCreateEnityRequest( const worker::RequestId<worker::CreateEntityRequest>& entity_creation_request_id, entity_info_t& entity_info_to_fill );
	static bool GetInfoFromDeleteEnityRequest( const worker::RequestId<worker::DeleteEntityRequest>& entity_deletion_request_id, entity_info_t& entity_info_to_fill );
	static bool GetInfoFromReserveEnityIdsRequest( const worker::RequestId<worker::ReserveEntityIdsRequest>& entity_id_reservation_request_id, entity_info_t& entity_info_to_fill );
	static bool GetInfoFromEnityId( const worker::EntityId& entity_id, entity_info_t& entity_info_to_fill );

private:
	static SpatialOSServer* GetInstance();
	SpatialOSServer();
	~SpatialOSServer();

private:
	bool isRunning = false;
	std::thread server_thread;

	worker::Dispatcher* dispatcher;
	worker::Connection* connection;

	std::vector<entity_info_t> entity_info_list;
};