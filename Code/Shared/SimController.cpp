#include "Shared/SimController.hpp"

#include "Shared/ActorBase.hpp"

#include <iostream>

//--------------------------------------------------------------------------
/**
* PlayerController
*/
SimController::SimController()  
	: ControllerBase()
{ 
	m_player_interface = true;
}

//--------------------------------------------------------------------------
/**
* ~PlayerController
*/
SimController::~SimController()
{

}

//--------------------------------------------------------------------------
/**
* Update
*/
void SimController::Update( float deltaTime )
{
// 	if( g_theInputSystem->KeyWasPressed( MOUSE_L ) || basic_attack_pressed )
// 	{
// 		m_controlled->PreformAbility( "basic_attack", GetScreenMousePos() );
// 	}
//	std::cout << "applying force: " << m_moveDir.x << ", " << m_moveDir.y << "scaled by " << deltaTime << std::endl;
	m_controlled->ApplyForce( m_moveDir * deltaTime );
}

//--------------------------------------------------------------------------
/**
* SetMoveDirection
*/
void SimController::SetMoveDirection( const Vec2& direction )
{
	m_moveDir = direction;
}
