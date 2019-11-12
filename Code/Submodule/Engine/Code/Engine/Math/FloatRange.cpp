#include "Engine/Math/FloatRange.hpp"
#include "Engine/core/EngineCommon.hpp"
#include <vector>


//--------------------------------------------------------------------------
/**
* FloatRange
*/
FloatRange::FloatRange( float min, float max )
{
	this->min = min; 
	this->max = max; 
}

//--------------

//--------------------------------------------------------------------------
/**
* FloatRange
*/
FloatRange::FloatRange( float minMax )
{
	this->min = minMax; 
	this->max = minMax; 
}

