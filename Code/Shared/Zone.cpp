#include "Shared/Zone.hpp"
#include "Shared/EntityBase.hpp"
#include "Shared/ControllerBase.hpp"

#include "Engine/Core/EngineCommon.hpp"

#include "Engine/Physics/PhysicsSystem.hpp"

//--------------------------------------------------------------------------
/**
* Zone
*/
Zone::Zone()
{
	m_physics_system = new PhysicsSystem();
	m_physics_system->BeginFrame();
}

//--------------------------------------------------------------------------
/**
* ~Zone
*/
Zone::~Zone()
{
	m_physics_system->Shutdown();
	SAFE_DELETE( m_physics_system );
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
* BeginFrame
*/
void Zone::BeginFrame()
{
	Zones& zones = GetZones();
	for (auto& zone : zones)
	{
		zone.second->m_physics_system->BeginFrame();
	}
}

//--------------------------------------------------------------------------
/**
* AddZone
*/
Zone* Zone::AddZone( uint id, Zone* zone /*= nullptr*/ )
{
	Zones& zones = GetZones();

	if (zones.find(id) != zones.end())
	{
		delete zones[id];
	}
	zones[id] = zone ? zone : new Zone();
	
	return zones[id];
}

//--------------------------------------------------------------------------
/**
* ClearAllZones
*/
void Zone::ClearAllZones()
{
	GetZones().clear();
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
			delete entity;
			entity = nullptr;
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
Zone* Zone::GetZone( uint zone_id )
{
	Zones& zones = GetZones(); 
	if (zones.find(zone_id) != zones.end())
	{
		return	zones[zone_id];
	}
	return nullptr;
}

//--------------------------------------------------------------------------
/**
* GetZones
*/
Zones& Zone::GetZones()
{
	static Zones s_zones;
	return s_zones;
}

//--------------------------------------------------------------------------
/**
* UpdateZones
*/
void Zone::UpdateZones(float deltaTime)
{
	Zones& zones = GetZones();
	for( auto& zone : zones )
	{
		zone.second->Update( deltaTime );
	}
}

//--------------------------------------------------------------------------
/**
* EndFrame
*/
void Zone::EndFrame()
{
	Zones& zones = GetZones();
	for (auto& zone : zones)
	{
		zone.second->m_physics_system->EndFrame();
		for( EntityBase*& entity : zone.second->m_entities )
		{
			if( entity && entity->IsGarbage() )
			{
				zone.second->RemoveEntityWithController(entity);
			}
		}
	}
}
