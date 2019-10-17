#pragma once


class ActorBase;

class ControllerBase
{
	friend class ActorBase;
public:
	ControllerBase();
	virtual ~ControllerBase();

	virtual void Update( float deltaTime ) = 0;

protected:
	void SetControlled( ActorBase* to_control );
	ActorBase* m_controlled = nullptr;
	bool m_player_interface = false;

};