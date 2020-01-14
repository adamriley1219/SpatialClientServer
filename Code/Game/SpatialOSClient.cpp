#include "Game/SpatialOSClient.hpp"

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Memory/Debug/Log.hpp"

#include "Engine/Core/EventSystem.hpp"

#include <algorithm>
#include <chrono>
#include <iostream>

#include "Shared/EntityBase.hpp"
#include "Shared/EntityBaseDefinition.hpp"
#include "Shared/AbilityBase.hpp"
#include "Shared/AbilityBaseDefinition.hpp"
#include "Shared/ActorBase.hpp"
#include "Shared/ActorBaseDefinition.hpp"
#include "Shared/AIController.hpp"
#include "Shared/SimController.hpp"

#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"

worker::Components<
	improbable::Position,
	improbable::EntityAcl,
	improbable::Metadata,
	improbable::Persistence,
	improbable::Interest,
	siren::PlayerControls,
	siren::ServerAPI
> ComponentRegistry;


// Constants and parameters
const std::string kLoggerName = "client";
const std::uint32_t kGetOpListTimeoutInMilliseconds = 100;
static worker::RequestId<worker::EntityQueryRequest> APIQueryRequestId;

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
		Logf( "ClientLog", "Worker of type '%s' connecting through locator: queueing.", connection_parameters.WorkerType.c_str() );
		return true;
	};

	auto future = locator.ConnectAsync(ComponentRegistry, deployment_id, connection_parameters, queue_status_callback);
	return future.Get();
}


worker::Connection ConnectWithReceptionist(const std::string hostname,
	const std::uint16_t port,
	const std::string& worker_id,
	const worker::ConnectionParameters& connection_parameters) {
	auto future = worker::Connection::ConnectAsync(ComponentRegistry, hostname, port, worker_id, connection_parameters);
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


int CreateCleanSnapshot()
{
	improbable::WorkerAttributeSet clientAttributeSet({ "client" });
	improbable::WorkerAttributeSet serverAttributeSet({ "simulation" });
	improbable::WorkerRequirementSet clientRequirementSet({ clientAttributeSet });
	improbable::WorkerRequirementSet serverRequirementSet({ serverAttributeSet });
	improbable::WorkerRequirementSet clientOrServerRequirementSet({ clientAttributeSet, serverAttributeSet });

	worker::Entity apiEntity;
	apiEntity.Add<improbable::Metadata>({ "API" });
	apiEntity.Add<improbable::Persistence>({});
	apiEntity.Add<improbable::Position>({ {-100, -100, -100} });
	apiEntity.Add<siren::ServerAPI>({});
	worker::Map<std::uint32_t, improbable::WorkerRequirementSet> bootstrapComponentAclMap;
	bootstrapComponentAclMap.insert({ {siren::ServerAPI::ComponentId, serverRequirementSet} });
	apiEntity.Add<improbable::EntityAcl>({ clientOrServerRequirementSet, bootstrapComponentAclMap });
	improbable::ComponentInterest::Query query;
	improbable::ComponentInterest::QueryConstraint queryConstraint;
	queryConstraint.set_component_constraint(siren::Client::ComponentId);
	query.set_constraint(queryConstraint);
	query.set_full_snapshot_result({ true });
	improbable::ComponentInterest componentInterest;
	componentInterest.set_queries({ query });
	improbable::InterestData interestData;
	interestData.component_interest()[siren::ServerAPI::ComponentId] = componentInterest;
	apiEntity.Add<improbable::Interest>(interestData);

	worker::Result<worker::SnapshotOutputStream, worker::StreamErrorCode> outputStream = worker::SnapshotOutputStream::Create(ComponentRegistry, "D:/GitHubRepos/SpatialClientServer/SpatialOS/snapshots/CleanServerAPI.snapshot");
	worker::Result<worker::None, worker::StreamErrorCode> entityWritten = outputStream->WriteEntity(1, apiEntity);
	if (!entityWritten) {
		return 1;
	}
	return 0;
}

//--------------------------------------------------------------------------
/**
* Startup
*/
void SpatialOSClient::Startup(const std::vector<std::string>& args)
{
	GetInstance()->client_thread = std::thread( Run, args );
	while (!GetInstance()->IsRunning())
	{
		std::this_thread::yield();
	}
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
* Process
*/
void SpatialOSClient::Process()
{
	if( IsRunning() )
	{
		auto opList = GetInstance()->context.connection->GetOpList(0);
		GetInstance()->context.dispatcher->Process(opList);
	}
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
* IsConnected
*/
bool SpatialOSClient::IsConnected()
{
	return GetInstance()->context.connection && GetInstance()->context.connection->IsConnected();
}

//--------------------------------------------------------------------------
/**
* RequestEntityCreation
*/
void SpatialOSClient::RequestEntityCreation( EntityBase* entity )
{
	std::cout << "Inside RequestEntityCreation" << std::endl;
	if (!IsRunning())
	{
		std::cout << "Server not running, (SpatialOSServer::RequestEntityCreation)" << std::endl;
		return;
	}

	entity_info_t info;
	info.entity = entity;

	worker::Result<worker::RequestId<worker::OutgoingCommandRequest<CreateClientEntity>>> createEntityRequestId = GetContext()->connection->SendCommandRequest<CreateClientEntity>(GetContext()->APIEntityId, 30000, {});
	if (!createEntityRequestId) {
		std::cout << "Failed to send create client entity request: " << createEntityRequestId.GetErrorMessage().c_str() << std::endl;
		return;
	}

	std::cout << "Sent create client entity request with id " << createEntityRequestId->Id << std::endl;
	info.createEntityCommandRequestId = createEntityRequestId->Id;

	AddEntityInfo( info );
}

//--------------------------------------------------------------------------
/**
* UpdatePlayerControls
*/
void SpatialOSClient::UpdatePlayerControls( EntityBase* player, const Vec2& direction )
{
	entity_info_t* info = GetInfoFromEntity( player );

	if( info && info->created )
	{
		siren::PlayerControls::Update ctrUpdate;
		ctrUpdate.set_x_move( direction.x );
		ctrUpdate.set_y_move( direction.y );
		worker::UpdateParameters params;
		GetContext()->connection->SendComponentUpdate<siren::PlayerControls>( info->id, ctrUpdate, params );
	}
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
	GetInstance()->context.connection = &connection;
	connection.SendLogMessage(worker::LogLevel::kInfo, kLoggerName, "Connected successfully");

	// Register callbacks and run the worker main loop.
	worker::Dispatcher dispatcher{ ComponentRegistry };
	GetInstance()->context.dispatcher = &dispatcher;
	bool is_connected = connection.IsConnected();

	dispatcher.OnDisconnect([&](const worker::DisconnectOp& op) {
		std::cerr << "[disconnect] " << op.Reason << std::endl;
		Logf("ClientLog", "[disconnect] %s", op.Reason.c_str());

		is_connected = false;
		});


	RegisterCallbacks( dispatcher );
	
	// Look for API
	worker::query::EntityQuery APIEntityQuery
	{
		worker::query::ComponentConstraint{siren::ServerAPI::ComponentId},
		worker::query::SnapshotResultType{{{siren::ServerAPI::ComponentId}}}
	};
	APIQueryRequestId = connection.SendEntityQueryRequest(APIEntityQuery, {});
	Logf( "ClientLog", "Connected and running" );

	constexpr unsigned kFramesPerSecond = 30;
	constexpr std::chrono::duration<double> kFramePeriodSeconds{
		1. / static_cast<double>(kFramesPerSecond) };

	GetInstance()->isRunning = true;
	while ( is_connected && IsRunning() )
	{
// 		auto start_time = std::chrono::steady_clock::now();
// 
// 		//Process();
// 
// 		auto end_time = std::chrono::steady_clock::now();
// 		auto wait_for = kFramePeriodSeconds - (end_time - start_time);
		std::this_thread::sleep_for(kFramePeriodSeconds);
	}

	GetInstance()->isRunning = false;
	GetInstance()->context.connection = nullptr;
	GetInstance()->context.dispatcher = nullptr;
}


//--------------------------------------------------------------------------
/**
* RegisterCallbacks
*/
void SpatialOSClient::RegisterCallbacks( worker::Dispatcher& dispatcher )
{
	// Print messages received from SpatialOS
	dispatcher.OnLogMessage([&](const worker::LogMessageOp& op) {
		if (op.Level == worker::LogLevel::kFatal) {
			std::cerr << "Fatal error: " << op.Message << std::endl;
			Logf( "ClientLog", "Fetal error: %s", op.Message.c_str() );
		}
		std::cout << "Connection: " << op.Message << std::endl;
		Logf("ClientLog", "Connection: %s", op.Message.c_str());
		});

	dispatcher.OnEntityQueryResponse( EntityQueryResponse );

	dispatcher.OnCommandResponse<CreateClientEntity>( ClientCreationResponse );

	dispatcher.OnComponentUpdate<improbable::Position>( PositionUpdated );

	dispatcher.OnAddEntity( AddEntity );

	dispatcher.OnRemoveEntity( RemoveEntity );
}


//--------------------------------------------------------------------------
/**
* GetInfoFromEnityId
*/
entity_info_t* SpatialOSClient::GetInfoFromEntityId( const worker::EntityId& entity_id )
{
	for (entity_info_t& info : GetInstance()->entity_info_list)
	{
		if ( info.id == entity_id )
		{
			return &info;
		}
	}

	return nullptr;
}

//--------------------------------------------------------------------------
/**
* GetInfoFromEnityQueryId
*/
entity_info_t* SpatialOSClient::GetInfoFromEntityQueryId(int64_t id)
{
	for (entity_info_t& info : GetInstance()->entity_info_list)
	{
		if ( info.newEntityQueryId == id )
		{
			return &info;
		}
	}

	return nullptr;
}

//--------------------------------------------------------------------------
/**
* GetInfoFromEnity
*/
entity_info_t* SpatialOSClient::GetInfoFromEntity( EntityBase* entity_id )
{
	for (entity_info_t& info : GetInstance()->entity_info_list)
	{
		if ( info.entity && info.entity == entity_id)
		{
			return &info;
		}
	}

	return nullptr;
}

//--------------------------------------------------------------------------
/**
* RemoveInfoFromEnityId
*/
void SpatialOSClient::RemoveInfoFromEnityId( const worker::EntityId& entity_id )
{
	for ( entity_info_t& info : GetInstance()->entity_info_list )
	{
		if ( info.id == entity_id)
		{
			// Clears the location for reuse later
			info = entity_info_t();
		}
	}
}

//--------------------------------------------------------------------------
/**
* PositionUpdated
*/
void SpatialOSClient::PositionUpdated( const worker::ComponentUpdateOp<improbable::Position>& op )
{
	entity_info_t* info = GetInfoFromEntityId( op.EntityId );
	if ( info && info->created )
	{
		auto curPos = op.Update.coords();
		//std::cout << "PosUpdate: entCords for " << info->id << ":" << curPos->x() << "," << curPos->y() << "," << curPos->y() << std::endl;
		//Logf("ClientLog", "PosUpdate: entCords for %u:%.03f,%.03f,%.03f ", info->id, curPos->x(), curPos->y(), curPos->z() );
		info->entity->SetPosition( (float)curPos->x(), (float)curPos->z() );
	}
	else
	{
		Logf("Warning", "Getting updates from %u and it doesn't exist in game.", op.EntityId );
	}
}

//--------------------------------------------------------------------------
/**
* EntityQueryResponse
*/
void SpatialOSClient::EntityQueryResponse( const worker::EntityQueryResponseOp& op )
{
	std::cout << "Received entity query response for request " << op.RequestId.Id << " with status code " << (int)(op.StatusCode) << std::endl;
	entity_info_t* info = GetInfoFromEntityQueryId( op.RequestId.Id ); 


	if (op.RequestId == APIQueryRequestId && !op.Result.empty()) {
		GetInstance()->context.APIEntityId = op.Result.begin()->first;
		g_theEventSystem->FireEvent("API_connection_made");
	}
	else if( info && op.RequestId.Id == info->newEntityQueryId )
	{
		// Working on getting an entity into the game from the server
		worker::Entity worker_entity = op.Result.begin()->second;
		worker::Option<improbable::MetadataData&> data = worker_entity.Get<improbable::Metadata>();
		if( data )
		{
			std::string name = data->entity_type();
			if( EntityBaseDefinition::IsGameType( name ) )
			{
				info->entity = nullptr;
				if( AbilityBaseDefinition::DoesDefExist( name ) )
				{
					info->entity = new AbilityBase( name );
				}
				else if( ActorBaseDefinition::DoesDefExist( name ) )
				{
					info->entity = new ActorBase( name );
					if( name == "player" )
					{
						( (ActorBase*)info->entity )->Possess( new SimController() );
					}
					else
					{
						( (ActorBase*)info->entity )->Possess( new AIController() );
					}
				}
				if( info->entity )
				{
					worker::Option<improbable::PositionData&> pos = worker_entity.Get<improbable::Position>();
					if( pos )
					{
						info->entity->SetPosition( pos->coords().x(), pos->coords().z() );
					}
				}
			}
		}
	}
}

//--------------------------------------------------------------------------
/**
* ClientCreationResponse
*/
void SpatialOSClient::ClientCreationResponse( const worker::CommandResponseOp<CreateClientEntity>& op )
{
	if( op.StatusCode == worker::StatusCode::kSuccess )
	{
		entity_info_t* info = GetInfoFromCreateEntityCommandRequestId( op.RequestId.Id );
		if( !info->entity )
		{
			Logf( "Warning", "Improper creation given within SpatialOSClient::ClientCreationResponse" );
		}
		if( info )
		{
			info->created = true;
			info->id = op.Response->id_created();
		}
	}
}

//--------------------------------------------------------------------------
/**
* AddEntity
*/
void SpatialOSClient::AddEntity( const worker::AddEntityOp op )
{
	entity_info_t* info = GetInfoFromEntityId( op.EntityId );
	if( !info )
	{
		// info not established yet
		worker::query::EntityQuery entityQuery
		{
			worker::query::ComponentConstraint{improbable::Metadata::ComponentId},
			worker::query::SnapshotResultType{{{improbable::Metadata::ComponentId}}}
		};
		
		// Place info to grab
		entity_info_t info;
		info.newEntityQueryId = GetContext()->connection->SendEntityQueryRequest( entityQuery, {} ).Id;
		info.id = op.EntityId;
		AddEntityInfo( info );
	}
	else
	{
		// Already established the entity. Possibly already created by this worker or bug occurred.
	}
}

//--------------------------------------------------------------------------
/**
* RemoveEntity
*/
void SpatialOSClient::RemoveEntity( const worker::RemoveEntityOp op )
{
	entity_info_t* info = GetInfoFromEntityId( op.EntityId );
	if( info )
	{
		g_theGame->RemoveEntity( info->entity );
		*info = entity_info_t();
	}
}

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
/**
* GetInfoFromCreateEntityCommandRequestId
*/
entity_info_t* SpatialOSClient::GetInfoFromCreateEntityCommandRequestId( uint64_t request_id )
{
	for (entity_info_t& entity : GetInstance()->entity_info_list)
	{
		if ( entity.createEntityCommandRequestId == request_id )
		{
			return &entity;
		}
	}
	return nullptr;
}

//--------------------------------------------------------------------------
/**
* GetEntityList
*/
const std::vector<entity_info_t>& SpatialOSClient::GetEntityList()
{
	return GetInstance()->entity_info_list;
}

//--------------------------------------------------------------------------
/**
* GetDispatcher
*/
worker::Dispatcher* SpatialOSClient::GetDispatcher()
{
	return GetInstance()->context.dispatcher;
}

//--------------------------------------------------------------------------
/**
* AddEntityInfo
*/
void SpatialOSClient::AddEntityInfo( const entity_info_t& to_add )
{
	for (entity_info_t& info_old : GetInstance()->entity_info_list)
	{
		if (info_old.entity == nullptr)
		{
			info_old = to_add;
			return;
		}
	}

	GetInstance()->entity_info_list.push_back( to_add );
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
* GetContext
*/
ClientContext* SpatialOSClient::GetContext()
{
	return &GetInstance()->context;
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
