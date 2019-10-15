#include "Game/PlayerController.hpp"
#include "Game/ActorRenderable.hpp"
#include "Game/Game.hpp"

#include "Engine/Input/InputSystem.hpp"

#include "Engine/Math/Ray3.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Core/WindowContext.hpp"

//--------------------------------------------------------------------------
/**
* PlayerController
*/
PlayerController::PlayerController( ActorRenderable* toControl )
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

	if( g_theInputSystem->KeyWasPressed( MOUSE_L ) )
	{
		m_controlled->PreformAbility( "basic_attack", GetScreenMousePos() );
	}

	moveDir.Normalize();
	m_controlled->ApplyForce( moveDir );
}

//--------------------------------------------------------------------------
/**
* GetScreenMousePos
*/
Vec2 PlayerController::GetScreenMousePos()
{
	IntVec2 rawMouseMovement = GetRawScreenMousePos();

	Vec3 worldCamPos = g_theGame->GetCamera()->GetClientToWorld(rawMouseMovement);
	return Vec2( worldCamPos.x, worldCamPos.y );
}

//--------------------------------------------------------------------------
/**
* GetRawScreenMousePos
*/
IntVec2 PlayerController::GetRawScreenMousePos()
{
	return g_theWindowContext->GetClientMousePosition();
}
