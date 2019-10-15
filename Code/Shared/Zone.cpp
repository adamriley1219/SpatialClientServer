#include "Shared/Zone.hpp"
#include "Shared/EntityBase.hpp"

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
	delete m_physics_system;
	m_physics_system = nullptr;
}

//--------------------------------------------------------------------------
/**
* Update
*/
void Zone::Update(float deltaTime)
{
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
* AddEntity
*/
void Zone::AddEntity(EntityBase* entity_to_add)
{
	for( EntityBase*& entity : m_entities )
	{
		if( !entity )
		{
			entity = entity_to_add;
			return;
		}
	}
	m_entities.push_back( entity_to_add );
}

//--------------------------------------------------------------------------
/**
* RemoveEntity
*/
void Zone::RemoveEntity(EntityBase* entity_to_remove)
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
				zone.second->RemoveEntity(entity);
			}
		}
	}
}
