#pragma once


struct FloatRange
{
public:
	float min = 0.0f;
	float max = 0.0f;

public:
	FloatRange() {}
	explicit FloatRange( float min, float max );
	explicit FloatRange( float minMax );
	~FloatRange(){}

};
