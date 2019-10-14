#pragma once
#include "Engine/Physics/PhysicsSystem.hpp"

#include "Engine/Math/Vec2.hpp"



class EntityBase
{
public:
	EntityBase( const std::string& name );
	virtual ~EntityBase();

	virtual void Die();
	bool IsAlive() const;
	bool IsGarbage() const;
	virtual void Update(float deltaSeconds);

	void ApplyForce( const Vec2& force );

	// Getters
	Vec2 GetPosition() const;

	// Game play
	void TakeDamage(float damage);

private:
	std::string m_name = "NONE";

	Rigidbody2D* m_rigidbody;
	Collider2D* m_collider;
	Transform2D m_transform;

	bool m_isAccelerating = false;
	int m_rotateDirection = 0; // 1 for counter clockwise, -1 for clockwise, 0 for no movement.

	bool m_isDead = false;
	bool m_isGarbage = false;

	// Gameplay
	float m_health = 0.0f;
	float m_collisionDamage = 1.0f;

	

};
