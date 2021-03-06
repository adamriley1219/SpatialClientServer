#include "Game/PlayerController.hpp"
#include "Game/ActorRenderable.hpp"
#include "Game/Game.hpp"

#include "Engine/Input/InputSystem.hpp"

#include "Engine/Math/Ray3.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Core/WindowContext.hpp"

#include "Engine/Memory/Debug/Log.hpp"

//--------------------------------------------------------------------------
/**
* PlayerController
*/
PlayerController::PlayerController()  
	: ControllerBase()
{ 
	m_player_interface = true;
	Logf( "playerContr", "construction" );
}

//--------------------------------------------------------------------------
/**
* ~PlayerController
*/
PlayerController::~PlayerController()
{
	Logf( "playerContr", "deconstruction" );

}

//--------------------------------------------------------------------------
/**
* Update
*/
void PlayerController::Update( float deltaTime )
{
	UNUSED( deltaTime );
	m_moveDir = Vec2::ZERO;

	if( g_theInputSystem->KeyIsDown( KEY_W ) )
	{
		m_moveDir += Vec2::UP;
	}
	if ( g_theInputSystem->KeyIsDown(KEY_D) )
	{
		m_moveDir += Vec2::RIGHT;
	}
	if ( g_theInputSystem->KeyIsDown(KEY_A) )
	{
		m_moveDir += Vec2::LEFT;
	}
	if ( g_theInputSystem->KeyIsDown(KEY_S) )
	{
		m_moveDir += Vec2::DOWN;
	}

	bool basic_attack_pressed = false;
	for( uint id = 0; id < MAX_XBOX_CONTROLLERS; ++id )
	{
		if( m_moveDir == Vec2::ZERO && g_theInputSystem->GetControllerByID(id).IsConnected() )
		{
			m_moveDir = g_theInputSystem->GetControllerByID(id).m_leftJoystick.GetPosition();
			if( g_theInputSystem->GetControllerByID(id).GetButtonState( XBOX_BUTTON_ID_A ).WasJustPressed() )
			{
				basic_attack_pressed = true;
			}
		}
	}

	if( g_theInputSystem->KeyWasPressed( MOUSE_L ) || basic_attack_pressed )
	{
		m_controlled->PreformAbility( "basic_attack", GetScreenMousePos() );
	}

	m_moveDir.Normalize();
	//m_controlled->ApplyForce( m_moveDir * m_controlled->GetSpeed() * deltaTime );
}

//--------------------------------------------------------------------------
/**
* GetMoveDirection
*/
const Vec2& PlayerController::GetMoveDirection() const
{
	return m_moveDir;
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
