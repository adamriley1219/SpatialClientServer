#include "Shared/Zone.hpp"
#include "Shared/EntityBase.hpp"
#include "Shared/ActorBase.hpp"
#include "Shared/ControllerBase.hpp"

#include "Engine/Core/EngineCommon.hpp"

#include "Engine/Physics/PhysicsSystem.hpp"

//--------------------------------------------------------------------------
/**
* Zone
*/
Zone::Zone()
{
	
}

//--------------------------------------------------------------------------
/**
* ~Zone
*/
Zone::~Zone()
{

}

//--------------------------------------------------------------------------
/**
* Update
*/
void Zone::Update(float deltaTime)
{
	for ( ControllerBase* contr : m_controllers )
	{
		if (contr)
		{
			contr->Update(deltaTime);
		}
	}

	for( EntityBase* entity : m_entities )
	{
		if( entity )
		{
			entity->Update(deltaTime);
		}
	}

	m_physics_system->Update(deltaTime);
}




//--------------------------------------------------------------------------
/**
* AddEntityWithController
*/
void Zone::AddEntityWithController(EntityBase* entity_to_add, ControllerBase* controller /*= nullptr*/)
{
	AddController(controller);
	AddEntity( entity_to_add );
}

//--------------------------------------------------------------------------
/**
* RemoveEntityWithController
*/
void Zone::RemoveEntityWithController(EntityBase* entity_to_remove, ControllerBase* controller /*= nullptr*/)
{
	RemoveEntity(entity_to_remove);
	RemoveController(controller);
}

//--------------------------------------------------------------------------
/**
* Clear
*/
void Zone::Clear()
{
	Deinit();
	Init();
}

//--------------------------------------------------------------------------
/**
* Init
*/
void Zone::Init()
{
	m_physics_system = new PhysicsSystem();
	initialized = true;
}

//--------------------------------------------------------------------------
/**
* Deinit
*/
void Zone::Deinit()
{
	for (EntityBase*& entity : m_entities)
	{
		SAFE_DELETE(entity);
	}

	for (ControllerBase*& ctrl : m_controllers)
	{
		SAFE_DELETE(ctrl);
	}

	m_entities.clear();
	m_controllers.clear();


	m_physics_system->Shutdown();
	SAFE_DELETE(m_physics_system);
}

//--------------------------------------------------------------------------
/**
* BeginFrame
*/
void Zone::BeginFrame()
{
	Zone* zone = GetZone();
	zone->m_physics_system->BeginFrame();
}


//--------------------------------------------------------------------------
/**
* ClearAllZones
*/
void Zone::ClearAllZones()
{
	GetZone()->Clear();
}

//--------------------------------------------------------------------------
/**
* AddEntity
*/
void Zone::AddEntity( EntityBase* entity_to_add )
{
	if( !entity_to_add )
	{
		return;
	}

	for (EntityBase*& entity : m_entities)
	{
		if (!entity)
		{
			entity = entity_to_add;
			return;
		}
	}
	m_entities.push_back(entity_to_add);
}

//--------------------------------------------------------------------------
/**
* RemoveEntity
*/
void Zone::RemoveEntity( EntityBase* entity_to_remove )
{
	for (EntityBase*& entity : m_entities)
	{
		if (entity == entity_to_remove)
		{
			SAFE_DELETE( entity );
			return;
		}
	}
}

//--------------------------------------------------------------------------
/**
* AddController
*/
void Zone::AddController(ControllerBase* controller)
{
	if( !controller )
	{
		return;
	}

	for (ControllerBase*& contr : m_controllers)
	{
		if (!contr)
		{
			contr = controller;
			return;
		}
	}
	m_controllers.push_back(controller);
}

//--------------------------------------------------------------------------
/**
* RemoveController
*/
void Zone::RemoveController(ControllerBase* controller)
{
	for (ControllerBase*& contr : m_controllers)
	{
		if (contr == controller)
		{
			delete contr;
			contr = nullptr;
			return;
		}
	}
}

//--------------------------------------------------------------------------
/**
* GetZone
*/
Zone* Zone::GetZone()
{
	static Zone* zone = new Zone();
	return zone;
}

//--------------------------------------------------------------------------
/**
* UpdateZones
*/
void Zone::UpdateZones( float deltaTime )
{
	GetZone()->Update( deltaTime );
}

//--------------------------------------------------------------------------
/**
* EndFrame
*/
void Zone::EndFrame()
{
	Zone* zone = GetZone();
	
	zone->m_physics_system->EndFrame();
	for (EntityBase*& entity : zone->m_entities)
	{
		if (entity && entity->IsGarbage())
		{
			zone->RemoveEntityWithController(entity, ((ActorBase*)entity)->GetController() );
		}
	}
	
}

//--------------------------------------------------------------------------
/**
* Startup
*/
void Zone::Startup()
{
	GetZone()->Init();
}

//--------------------------------------------------------------------------
/**
* Shutdown
*/
void Zone::Shutdown()
{
	Zone* zone = GetZone();

	zone->Deinit();
}
