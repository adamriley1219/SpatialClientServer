#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include "Shared/SharedCommon.hpp"

#include <map>

class Zone;
typedef std::map<uint,Zone*> Zones;
class EntityBase;
class PhysicsSystem;
class ControllerBase;

class Zone
{
	friend class EntityBase;
private:
	Zone();
	~Zone();

public:
	void Update( float deltaTime );
	void AddEntityWithController( EntityBase* entity, ControllerBase* controller = nullptr );
	void RemoveEntityWithController( EntityBase* entity, ControllerBase* controller = nullptr );

public:
	static void BeginFrame();
	static void UpdateZones( float deltaTime );
	static void EndFrame();

	static Zone* GetZone( uint zone_id );
	static Zones& GetZones(); 
	
	static Zone* AddZone( uint id, Zone* zone = nullptr );
	
	static void ClearAllZones();

public:
	void AddEntity( EntityBase* entity );
	void RemoveEntity( EntityBase* entity );
	void AddController( ControllerBase* controller );
	void RemoveController( ControllerBase* controller );

public:
	std::vector<EntityBase*> m_entities;
	std::vector<ControllerBase*> m_controllers;
	PhysicsSystem* m_physics_system;
	
};