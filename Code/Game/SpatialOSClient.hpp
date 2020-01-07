#pragma once


#include <improbable/worker.h>
#include <improbable/standard_library.h>
#include <improbable/view.h>

#include <ClientServer.h>

#include <thread>

struct entity_info_t
{
	worker::Entity* entity;
	worker::EntityId id;
	bool created = false;
};

class SpatialOSClient
{
public:
	static void Startup( const std::vector<std::string>& args );
	static void Shutdown();

public:
	static bool IsRunning(); 

private:
	static void Run(std::vector<std::string> arguments);
	static void RegisterCallbacks( worker::View& view );

	static entity_info_t* GetInfoFromEnityId(const worker::EntityId& entity_id);

public:
	// Component Updating
	static void PositionUpdated(const worker::ComponentUpdateOp<improbable::Position>& op);

	static const std::vector<entity_info_t>& GetEntityList();
	static worker::View* GetView();

private:
	static SpatialOSClient* GetInstance();
	static 	const worker::ComponentRegistry& MyComponents();
	SpatialOSClient();
	~SpatialOSClient();

private:
	bool isRunning = false;
	std::thread client_thread;

	worker::View* view = nullptr;
	worker::Connection* connection = nullptr;

	std::vector<entity_info_t> entity_info_list;
};