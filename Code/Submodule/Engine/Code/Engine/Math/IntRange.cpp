#include "Engine/Math/IntRange.hpp"
#include "Engine/core/EngineCommon.hpp"
#include <vector>


//--------------------------------------------------------------------------
/**
* IntRange
*/
IntRange::IntRange( int min, int max )
{
	this->min = min; 
	this->max = max; 
}


//--------------------------------------------------------------------------
/**
* IntRange
*/
IntRange::IntRange( int minAndMax )
{
	this->min = minAndMax; 
	this->max = minAndMax; 
}