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

#include "Game/View.hpp"
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

using DeleteClientEntity = siren::ServerAPI::Commands::DeleteClientEntity;

// Connection helpers
worker::Connection ConnectWithLocator(const std::string hostname,
	const std::string project_name,
	const std::string deployment_name,
	const std::string login_token,
	const worker::ConnectionParameters& connection_parameters) {
	worker::LocatorParameters locator_parameters;
	locator_parameters.ProjectName = project_name;
	locator_parameters.CredentialsType = worker::LocatorCredentialsType::kLoginToken;
	locator_parameters.LoginToken.Token = login_token;

	worker::Locator locator{ hostname, locator_parameters };

	auto queue_status_callback = [&](const worker::QueueStatus& queue_status) {
		if (!queue_status.Error.empty()) {
			std::cerr << "Error while queuing: " << *queue_status.Error << std::endl;
			ERROR_RECOVERABLE( Stringf("Error while queueing: %s", queue_status.Error->c_str()).c_str() );
			return false;
		}
		std::cout << "Worker of type '" << connection_parameters.WorkerType
			<< "' connecting through locator: queuing." << std::endl;
		Logf( "ClientLog", "Worker of type '%s' connecting through locator: queuing.", connection_parameters.WorkerType.c_str() );
		return true;
	};

	auto future = locator.ConnectAsync(ComponentRegistry, deployment_name, connection_parameters, queue_status_callback);
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
	ClientContext& context = GetInstance()->context;
	::siren::DeleteClientEntityRequest request;
	request.set_id_to_delete( context.clientEntityId );
	worker::Result<worker::RequestId<worker::OutgoingCommandRequest<DeleteClientEntity>>> createEntityRequestId = context.connection->SendCommandRequest<DeleteClientEntity>( GetContext()->APIEntityId, request, 30000, {} );
	for( entity_info_t*& info : GetInstance()->entity_info_list )
	{
		SAFE_DELETE(info);
	}
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
		GetInstance()->context.view->Process(opList);

		GetInstance()->Update();
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

	entity_info_t* info = new entity_info_t();
	info->game_entity = entity;

	worker::Result<worker::RequestId<worker::OutgoingCommandRequest<CreateClientEntity>>> createEntityRequestId = GetContext()->connection->SendCommandRequest<CreateClientEntity>(GetContext()->APIEntityId, 30000, {});
	if (!createEntityRequestId) {
		std::cout << "Failed to send create client entity request: " << createEntityRequestId.GetErrorMessage().c_str() << std::endl;
		ERROR_RECOVERABLE( "Failed to send create client entity" );
		SAFE_DELETE( info );
		return;
	}

	std::cout << "Sent create client entity request with id " << createEntityRequestId->Id << std::endl;
	info->createEntityCommandRequestId = createEntityRequestId->Id;
	AddEntityInfo( info );
}

//--------------------------------------------------------------------------
/**
* UpdatePlayerControls
*/
void SpatialOSClient::UpdatePlayerControls( EntityBase* player, const Vec2& force_vec )
{
	entity_info_t* info = GetInfoWithEntity( player );

	if( force_vec.GetLengthSquared() > 500.0f )
	{
		LogFlush();
		ERROR_RECOVERABLE( "Yeah, not meant to be going this far." );
	}

	if( info && info->created )
	{
		siren::PlayerControls::Update ctrUpdate;
		ctrUpdate.set_x_move( force_vec.x );
		ctrUpdate.set_y_move( force_vec.y );
		worker::UpdateParameters params;
		Logf( "", "	Sending intent x: %.04f y:%.04f with entity ID: %i", force_vec.x, force_vec.y, info->id );
		GetContext()->connection->SendComponentUpdate<siren::PlayerControls>( info->id, ctrUpdate, params );
	}
	else
	{
		Logf("WARNING", "Failed to send input", force_vec.x, force_vec.y);
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


	constexpr unsigned kFramesPerSecond = 30;
	constexpr std::chrono::duration<double> kFramePeriodSeconds{
		1. / static_cast<double>(kFramesPerSecond) };


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


	const std::string connection_type = arguments[0];
	if (connection_type != "receptionist" && connection_type != "locator") {
		print_usage();
		return;
	}

	const bool use_locator = connection_type == "locator";

	worker::alpha::LoginTokenDetails login_details;
	if( use_locator )
	{
		worker::alpha::PlayerIdentityTokenRequest player_identok_request;
		player_identok_request.DevelopmentAuthenticationToken = arguments[5];
		player_identok_request.DisplayName = "player_login_display_name";
		player_identok_request.PlayerId = "useless";
		player_identok_request.UseInsecureConnection = false;
		player_identok_request.Metadata = "{username:bob, password:password}";
		auto future_player_ident = worker::alpha::CreateDevelopmentPlayerIdentityTokenAsync(  arguments[1], (uint16_t)atoi(arguments[2].c_str()), player_identok_request );
		worker::alpha::PlayerIdentityTokenResponse player_iden_response = future_player_ident.Get();

		if(  player_iden_response.Status.Code == worker::ConnectionStatusCode::kSuccess )
		{
			worker::alpha::LoginTokensRequest request;
			request.WorkerType = "Managed";
			request.UseInsecureConnection = false;
			request.PlayerIdentityToken = player_iden_response.PlayerIdentityToken;
			auto future = worker::alpha::CreateDevelopmentLoginTokensAsync( arguments[1], (uint16_t)atoi(arguments[2].c_str()), request );
			worker::alpha::LoginTokensResponse response = future.Get();
			if( response.Status.Code == worker::ConnectionStatusCode::kSuccess && response.LoginTokens.size() > 0 )
			{
				login_details = response.LoginTokens[0];
			}
			else
			{
				if( response.Status.Code == worker::ConnectionStatusCode::kSuccess )
				{
					Logf("ClientLog", "[FAILED LOGIN] Success but no login tokens given. Make sure you have dev_login tagged in the deployment.");
					ERROR_AND_DIE("Failed to log into the server : Success but no login tokens given");
				}
				else
				{
					Logf("ClientLog", "[FAILED LOGIN] %s", response.Status.Detail.c_str());
					ERROR_AND_DIE(Stringf("Failed to log into the server : %s", response.Status.Detail.c_str()));
				}
			}
		}
		else
		{
			Logf("ClientLog", "[FAILED LOGIN] %s", player_iden_response.Status.Detail.c_str() );
			ERROR_AND_DIE(Stringf("Failed to log into the server : %s", player_iden_response.Status.Detail.c_str() ) );
		}

	}

	worker::ConnectionParameters parameters;
	parameters.WorkerType = "External";
	parameters.Network.ConnectionType = worker::NetworkConnectionType::kTcp;
	parameters.Network.UseExternalIp = true;

	// Connect with locator or receptionist
	worker::Connection connection = use_locator
		? ConnectWithLocator(arguments[1], arguments[3], login_details.DeploymentName, login_details.LoginToken, parameters )
		: ConnectWithReceptionist(arguments[1], (uint16_t)atoi(arguments[2].c_str()), arguments[3], parameters);
	GetInstance()->context.connection = &connection;
	connection.SendLogMessage( worker::LogLevel::kInfo, kLoggerName, "Connected successfully" );

	// Register callbacks and run the worker main loop.
	View view{ ComponentRegistry };
	GetInstance()->context.view = &view;
	bool is_connected = connection.IsConnected();

	view.OnDisconnect([&](const worker::DisconnectOp& op) {
		Logf("ClientLog", "[disconnect] %s", op.Reason.c_str());
		ERROR_RECOVERABLE( Stringf( "Disconnected from the server : %s", op.Reason.c_str() ).c_str() );
		is_connected = false;
		});


	RegisterCallbacks( view );
	
	// Look for API
	worker::query::EntityQuery APIEntityQuery
	{
		worker::query::ComponentConstraint{siren::ServerAPI::ComponentId},
		worker::query::SnapshotResultType{{{siren::ServerAPI::ComponentId}}}
	};
	APIQueryRequestId = connection.SendEntityQueryRequest(APIEntityQuery, {});
	Logf( "ClientLog", "Connected and running" );



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
	GetInstance()->context.view = nullptr;
}


//--------------------------------------------------------------------------
/**
* RegisterCallbacks
*/
void SpatialOSClient::RegisterCallbacks( View& view )
{
	// Print messages received from SpatialOS
	view.OnLogMessage([&](const worker::LogMessageOp& op) {
		if (op.Level == worker::LogLevel::kFatal) {
			std::cerr << "Fatal error: " << op.Message << std::endl;
			Logf( "ClientLog", "Fetal error: %s", op.Message.c_str() );
		}
		std::cout << "Connection: " << op.Message << std::endl;
		Logf("ClientLog", "Connection: %s", op.Message.c_str());
		});

	view.OnEntityQueryResponse( EntityQueryResponse );

	view.OnCommandResponse<CreateClientEntity>( ClientCreationResponse );

	view.OnCommandResponse<DeleteClientEntity>( []( const worker::CommandResponseOp<DeleteClientEntity>& op )
		{
			Logf( "%s", op.Message.c_str() );
		}
	);


}


//--------------------------------------------------------------------------
/**
* Update
*/
void SpatialOSClient::Update()
{
	for( auto& ent_pair : context.view->m_entities )
	{
		View::entity_tracker_t& tracker = ent_pair.second;
		if( !tracker.updated )
		{
			// Entity wasn't updated so there's nothing to tell the client
			continue;
		}
		
		entity_info_t* info = GetInfoWithEntityId( ent_pair.first );
		if( info )
		{
			if( info->game_entity )
			{
				Logf("SpatialOSClient::Update", "Updating ID: %i", ent_pair.first);
				UpdateEntityWithWorkerEntity( *(info->game_entity), tracker.worker_entity );
			}
			else
			{
				// should never get here.
				Logf("ERROR", "SpatialOSClient::Update - info and worker entity found but no game entity to update");
			}
		}
		else
		{
			// Game doesn't know about the instance from the server yet.
			
			//	Working on getting an entity into the game from the server
			Logf("SpatialOSClient::Update", "can't find info with ID: %i", ent_pair.first);
			worker::Option<improbable::MetadataData&> data = tracker.worker_entity.Get<improbable::Metadata>();
			worker::Option<improbable::InterestData&> intrest = tracker.worker_entity.Get<improbable::Interest>();
			if (data)
			{
				Logf("SpatialOSClient::Update", "Making from data");
				entity_info_t* new_info = new entity_info_t();
				std::string name = data->entity_type();
				new_info->game_entity = g_theGame->CreateSimulatedEntity( name );
				if( new_info->game_entity )
				{
					UpdateEntityWithWorkerEntity( *(new_info->game_entity), tracker.worker_entity );
					new_info->id = ent_pair.first;
					new_info->created = true;
					AddEntityInfo( new_info );
					Logf("SpatialOSClient::Update", "Success in creation of entity");
				}
				else
				{
					SAFE_DELETE( new_info );
				}
			}
		}
		tracker.updated = false;
	}

	// Check for deleted entities
	for( entity_info_t*& info : entity_info_list )
	{
		// See if we can no longer see an entity that has been created
		if( info && context.view->m_entities.find( info->id ) == context.view->m_entities.end() && info->created && (EntityBase*)g_theGame->GetPlayerEntity() != info->game_entity )
		{
			// Tell the entity to die and then erase knowledge of entity
			info->game_entity->Die();
			SAFE_DELETE(info);
		}
	}
}

//--------------------------------------------------------------------------
/**
* UpdateEntityWithWorkerEntity
*/
void SpatialOSClient::UpdateEntityWithWorkerEntity( EntityBase& entity, worker::Entity& worker_entity )
{
	worker::Option<improbable::PositionData&> pos = worker_entity.Get<improbable::Position>();
	if (pos)
	{
		entity.SetPosition((float)pos->coords().x(), (float)pos->coords().z());
	}	
}

//--------------------------------------------------------------------------
/**
* GetInfoFromEnityId
*/
entity_info_t* SpatialOSClient::GetInfoWithEntityId( const worker::EntityId& entity_id )
{
	for (entity_info_t*& info : GetInstance()->entity_info_list)
	{
		if ( info && info->id == entity_id )
		{
			return info;
		}
	}

	return nullptr;
}


//--------------------------------------------------------------------------
/**
* GetInfoFromEnity
*/
entity_info_t* SpatialOSClient::GetInfoWithEntity( EntityBase* entity_id )
{
	for (entity_info_t*& info : GetInstance()->entity_info_list)
	{
		if ( info && info->game_entity && info->game_entity == entity_id )
		{
			return info;
		}
	}

	return nullptr;
}


//--------------------------------------------------------------------------
/**
* EntityQueryResponse
*/
void SpatialOSClient::EntityQueryResponse( const worker::EntityQueryResponseOp& op )
{
	std::cout << "Received entity query response for request " << op.RequestId.Id << " with status code " << (int)(op.StatusCode) << std::endl;


	if (op.RequestId == APIQueryRequestId && !op.Result.empty()) {
		GetInstance()->context.APIEntityId = op.Result.begin()->first;
		g_theEventSystem->FireEvent("API_connection_made");
	}
}

//--------------------------------------------------------------------------
/**
* ClientCreationResponse
*/
void SpatialOSClient::ClientCreationResponse( const worker::CommandResponseOp<CreateClientEntity>& op )
{
	Logf( "SpatialOSClient::ClientCreationResponse", "Response with Request ID: %i and Entity ID: %i", op.RequestId.Id, op.EntityId );
	if( op.StatusCode == worker::StatusCode::kSuccess )
	{
		Logf( "SpatialOSClient::ClientCreationResponse", "		Response from response received: %i", op.Response->id_created() );
		entity_info_t* info = GetInfoWithCreateEntityCommandRequestId( op.RequestId.Id );

		if( info )
		{
			Logf( "SpatialOSClient::ClientCreationResponse", "		Info found with entity ID: %i", info->id );
			if( !info->game_entity )
			{
				Logf( "WARNING", "Improper creation given within SpatialOSClient::ClientCreationResponse" );
			}
			
			info->created = true;
			info->id = op.Response->id_created();
			GetInstance()->context.clientEntityId = op.Response->id_created();
		}
		else
		{
			Logf( "WARNING", "Couldn't find the thing you were trying to create." );
		}
	}
}

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
/**
* GetInfoWithCreateEntityCommandRequestId
*/
entity_info_t* SpatialOSClient::GetInfoWithCreateEntityCommandRequestId( uint64_t request_id )
{
	for (entity_info_t*& entity : GetInstance()->entity_info_list)
	{
		if ( entity && entity->createEntityCommandRequestId == request_id )
		{
			return entity;
		}
	}
	return nullptr;
}

//--------------------------------------------------------------------------
/**
* GetInfoList
*/
const std::vector<entity_info_t*>& SpatialOSClient::GetInfoList()
{
	return GetInstance()->entity_info_list;
}

//--------------------------------------------------------------------------
/**
* AddEntityInfo
*/
void SpatialOSClient::AddEntityInfo( entity_info_t* to_add )
{
	for( entity_info_t*& info_old : GetInstance()->entity_info_list )
	{
		if ( !info_old )
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
