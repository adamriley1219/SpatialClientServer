#include "Game/PlayerController.hpp"
#include "Game/Entity.hpp"

#include "Engine/Input/InputSystem.hpp"

//--------------------------------------------------------------------------
/**
* PlayerController
*/
PlayerController::PlayerController( Entity* toControl )
{
	m_controlled = toControl;
}

//--------------------------------------------------------------------------
/**
* ~PlayerController
*/
PlayerController::~PlayerController()
{

}

//--------------------------------------------------------------------------
/**
* Update
*/
void PlayerController::Update( float deltaTime )
{
	UNUSED(deltaTime);
	Vec2 moveDir = Vec2::ZERO;

	if( g_theInputSystem->KeyIsDown( KEY_W ) )
	{
		moveDir += Vec2::UP;
	}
	if (g_theInputSystem->KeyIsDown(KEY_D))
	{
		moveDir += Vec2::RIGHT;
	}
	if (g_theInputSystem->KeyIsDown(KEY_A))
	{
		moveDir += Vec2::LEFT;
	}
	if (g_theInputSystem->KeyIsDown(KEY_S))
	{
		moveDir += Vec2::DOWN;
	}

	if( moveDir == Vec2::ZERO && g_theInputSystem->GetControllerByID(0).IsConnected() )
	{
		moveDir = g_theInputSystem->GetControllerByID(0).m_leftJoystick.GetPosition();
	}

	moveDir.Normalize();
	m_controlled->ApplyForce( moveDir );
}
