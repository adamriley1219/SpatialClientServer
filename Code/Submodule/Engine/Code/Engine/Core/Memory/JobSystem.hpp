#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include <thread>
#include <atomic>

class Job;
class JobCategory;

enum eJobCategories
{
	JOB_GENERIC = 0,
	JOB_RENDER,
	JOB_MAIN,

	JOB_CORE_CATEGORY_COUNT,
};


class JobSystem
{
public:
	/**
	* If num_threads is negative, minus that number from the max threads available.
	* Min of 1 thread unless specifically given 0.
	*/
	static bool Startup( int num_threads = -1, uint num_categories = JOB_CORE_CATEGORY_COUNT );
	static void Shutdown();

	static void Run( Job* job );

	static void FinishJobs();
	static void FinishJobsForCategory( uint category = JOB_GENERIC );

	// Process until all jobs are complete for this category
	static bool ProcessCategory( uint category );
	// Process until all jobs are complete or time runs out
	static void ProcessCategoryForMillisec( uint category, uint millisec );

	static void ProcessFinishedQueue();
	static void ProcessFinishedQueue( uint category );

};



class Job
{
	friend class JobSystem;
public:
	typedef void (*finish_cb)(Job*);

public:
	virtual void Execute() = 0;

	void AddSuccessor( Job* job );
	void AddPredecessor( Job* job );

	void SetFinishCallback( finish_cb cb );

	void SetCategoty( uint category ) { m_category = category; }

private:
	bool TryStart();
	void FinishJob();

private:
	uint m_category = JOB_GENERIC;
	finish_cb m_finishedCallback = nullptr;

	std::vector<Job*> 	m_successors;
	std::atomic<int> 	m_predecessorCount = 1;

};