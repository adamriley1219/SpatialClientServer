//-----------------------------------------------------------------------------------------------
// Time.hpp
//
#pragma once
#include <atomic>


//-----------------------------------------------------------------------------------------------
double GetCurrentTimeSeconds();

uint64_t GetCurrentTimeHPC();
double GetMillisecToSec( uint64_t millisec );
double HPCToSeconds( uint64_t hpc );