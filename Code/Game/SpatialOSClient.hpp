#pragma once
#include "Engine/Math/Vec2.hpp"

#include <improbable/worker.h>
#include <improbable/standard_library.h>
#include <improbable/view.h>

#include <ClientServer.h>

#include <thread>
#include <mutex>

class EntityBase;

struct ClientContext {
	worker::Connection* connection = nullptr;
	worker::Dispatcher* dispatcher = nullptr;
	worker::EntityId APIEntityId;
	worker::EntityId clientEntityId;
	bool absoluteInterest;
	bool restrictController;
	bool viewDependenciesUpdated;
	double lastInterestUpdatePositionY;
	double edgeLength;
};

struct entity_info_t
{
	EntityBase* entity = nullptr;
	worker::EntityId id = 0;
	uint64_t createEntityCommandRequestId = 0;
	int64_t newEntityQueryId = -1;
	bool created = false;
	bool updated = false;
};

using CreateClientEntity = siren::ServerAPI::Commands::CreateClientEntity;


class SpatialOSClient
{
public:
	static void Startup( const std::vector<std::string>& args );
	static void Shutdown();

	static void Process();

public:
	static bool IsRunning(); 
	static bool IsConnected();
	static void RequestEntityCreation( EntityBase* entity );
	static void UpdatePlayerControls( EntityBase* player, const Vec2& direction );

private:
	static void Run( std::vector<std::string> arguments );
	static void RegisterCallbacks( worker::Dispatcher& dispatcher );

public:
	// Component Updating
	static void PositionUpdated( const worker::ComponentUpdateOp<improbable::Position>& op );
	static void EntityQueryResponse( const worker::EntityQueryResponseOp& op );
	static void ClientCreationResponse( const worker::CommandResponseOp<CreateClientEntity>& op );

	static void AddEntity( const worker::AddEntityOp op );
	static void RemoveEntity( const worker::RemoveEntityOp op );

	static entity_info_t* GetInfoFromCreateEntityCommandRequestId( uint64_t request_id );
	static entity_info_t* GetInfoFromEntityId( const worker::EntityId& entity_id );
	static entity_info_t* GetInfoFromEntityQueryId( int64_t id );
	static entity_info_t* GetInfoFromEntity( EntityBase* entity_id );

	static void RemoveInfoFromEnityId( const worker::EntityId& entity_id );

	static const std::vector<entity_info_t>& GetEntityList();
	static worker::Dispatcher* GetDispatcher();

	static void AddEntityInfo( const entity_info_t& to_add );

private:
	static SpatialOSClient* GetInstance();
	static ClientContext* GetContext();
	SpatialOSClient();
	~SpatialOSClient();

private:
	bool isRunning = false;
	std::thread client_thread;

	std::mutex entity_info_list_lock;

	ClientContext context;
	std::vector<entity_info_t> entity_info_list;	

};