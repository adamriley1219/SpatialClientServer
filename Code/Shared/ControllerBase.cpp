#include "Shared/ControllerBase.hpp"

//--------------------------------------------------------------------------
/**
* ControllerBase
*/
ControllerBase::ControllerBase()
{

}

//--------------------------------------------------------------------------
/**
* ~ControllerBase
*/
ControllerBase::~ControllerBase()
{

}

//--------------------------------------------------------------------------
/**
* SetControlled
*/
void ControllerBase::SetControlled(ActorBase* to_control)
{
	m_controlled = to_control;
}
