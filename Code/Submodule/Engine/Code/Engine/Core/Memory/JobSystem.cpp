#include "Engine/Core/Memory/JobSystem.hpp"
#include "Engine/Core/Memory/AsyncQueue.hpp"

#include "Engine/Core/Time/StopWatch.hpp"
#include "Engine/Core/Time/Time.hpp"

//--------------------------------------------------------------------------

static std::vector<JobCategory*> categories;
static std::vector<std::thread> genericThreads;

static bool isRunning = false;


//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
class JobCategory
{
public:
	void Enqueue( Job* job );

	// return nullptr if not job is ready
	Job* TryDequeue(); 

	// returns nullptr only if system is shutdown, otherwise block until a job is ready
	// Job* Dequeue(); 

	void EnqueueFinished( Job* job );
	Job* TryDequeueFinished();

private:
	// int m_queueId; 
	AsyncQueue<Job*> m_pendingQueue;
	AsyncQueue<Job*> m_finishQueue;

};

//--------------------------------------------------------------------------
/**
* Enqueue
*/
void JobCategory::Enqueue(Job* job)
{
	m_pendingQueue.Enqueue(job);
}

//--------------------------------------------------------------------------
/**
* TryDequeue
*/
Job* JobCategory::TryDequeue()
{
	if( m_pendingQueue.IsEmpty() )
	{
		return nullptr;
	}
	Job* job = nullptr;
	m_pendingQueue.Dequeue( &job );
	return job;
}
// 
// //--------------------------------------------------------------------------
// /**
// * Dequeue
// */
// Job* JobCategory::Dequeue()
// {
// 	if( !isRunning )
// 	{
// 		return nullptr;
// 	}
// 	Job* job = nullptr;
// 	while ( ( job = TryDequeue() ) == nullptr )
// 	{
// 		std::this_thread::yield();
// 	}
// 	return job;
// }

//--------------------------------------------------------------------------
/**
* EnqueueFinished
*/
void JobCategory::EnqueueFinished(Job* job)
{
	m_finishQueue.Enqueue(job);
}

//--------------------------------------------------------------------------
/**
* TryDequeueFinished
*/
Job* JobCategory::TryDequeueFinished()
{
	if( m_finishQueue.IsEmpty() )
	{
		return nullptr;
	}
	Job* job = nullptr;
	m_finishQueue.Dequeue( &job );
	return job;
}

//--------------------------------------------------------------------------

static void JobThread()
{
	while ( isRunning )
	{
		while(JobSystem::ProcessCategory(JOB_GENERIC)){}
		std::this_thread::yield();
	}
}


//--------------------------------------------------------------------------
/**
* Startup
*/
bool JobSystem::Startup( int num_threads /*= -1*/, uint num_categories /*= JOB_CORE_CATEGORY_COUNT */ )
{
	if( isRunning )
	{
		return false;
	}
	uint max_threads = std::thread::hardware_concurrency();

	int num_threads_to_spin = max_threads;
	if( num_threads < 0 )
	{
		num_threads_to_spin += num_threads;
		if( num_threads_to_spin < 1 )
		{
			num_threads_to_spin = 1;
		}
	}
	else
	{
		num_threads_to_spin = num_threads;
	}

	if( num_threads_to_spin < 0 )
	{
		return false;
	}

	isRunning = true;

	// Make sure you do this before starting threads
	for (uint catIdx = 0; catIdx < num_categories; ++catIdx)
	{
		categories.push_back(new JobCategory());
	}

	for( uint threadIdx = 0; threadIdx < (uint) num_threads_to_spin; ++threadIdx )
	{
		genericThreads.emplace_back( std::thread( JobThread ) );
	}

	return isRunning;
}

//--------------------------------------------------------------------------
/**
* Shutdown
*/
void JobSystem::Shutdown()
{
	isRunning = false;
	FinishJobs();
	for( uint threadIdx = 0; threadIdx < genericThreads.size(); ++threadIdx )
	{
		genericThreads[threadIdx].join();
	}
	genericThreads.clear();
	categories.clear();
}

//--------------------------------------------------------------------------
/**
* Run
*/
void JobSystem::Run( Job* job )
{
	if( !isRunning )
	{
		return;
	}
	job->TryStart();
}


//--------------------------------------------------------------------------
/**
* FinishJobs
*/
void JobSystem::FinishJobs()
{
	if (!isRunning)
	{
		return;
	}
	for( uint categoryIdx = 0; categoryIdx < categories.size(); ++categoryIdx )
	{
		FinishJobsForCategory(categoryIdx);
	}
}

//--------------------------------------------------------------------------
/**
* FinishJobsForCategory
*/
void JobSystem::FinishJobsForCategory(uint category)
{
	if (!isRunning)
	{
		return;
	}
	while (JobSystem::ProcessCategory(category)) {}
}

//--------------------------------------------------------------------------
/**
* ProcessCategory
*/
bool JobSystem::ProcessCategory(uint category)
{
	if (!isRunning || category >= categories.size() )
	{
		return false;
	}

	Job* job = categories[category]->TryDequeue();
	if( job )
	{
		job->Execute();
		job->FinishJob();
		if( !job->m_finishedCallback )
		{
			delete job;
		}
		return true;
	}
	return false;
}

//--------------------------------------------------------------------------
/**
* ProcessCategoryForMillisec
*/
void JobSystem::ProcessCategoryForMillisec( uint category, uint millisec )
{
	if (!isRunning)
	{
		return;
	}
	StopWatch timer( &Clock::Master );
	timer.SetAndReset( (float) GetMillisecToSec( millisec ) );
	while ( !timer.HasElapsed() )
	{
		if( !JobSystem::ProcessCategory( category ) )
		{
			return; // return if there's no work
		}
		std::this_thread::yield();
	}
}

//--------------------------------------------------------------------------
/**
* ProcessFinishedQueue
*/
void JobSystem::ProcessFinishedQueue()
{
	if (!isRunning)
	{
		return;
	}
	for (uint categoryIdx = 0; categoryIdx < categories.size(); ++categoryIdx)
	{
		JobSystem::ProcessFinishedQueue( categoryIdx );
	}
}

//--------------------------------------------------------------------------
/**
* ProcessFinishedQueue
*/
void JobSystem::ProcessFinishedQueue( uint category )
{
	if (!isRunning)
	{
		return;
	}
	Job* job = nullptr;
	while ( ( job = categories[category]->TryDequeueFinished() ) != nullptr )
	{
		job->m_finishedCallback( job );
		delete job;
	}
}

//--------------------------------------------------------------------------
/**
* AddSuccessor
*/
void Job::AddSuccessor( Job* job )
{
	m_successors.push_back( job );
	++(job->m_predecessorCount);
}

//--------------------------------------------------------------------------
/**
* AddPredecessor
*/
void Job::AddPredecessor( Job* job )
{
	job->AddSuccessor( this );
}

//--------------------------------------------------------------------------
/**
* SetFinishCallback
*/
void Job::SetFinishCallback( finish_cb cb )
{
	m_finishedCallback = cb;
}

//--------------------------------------------------------------------------
/**
* TryStart
*/
bool Job::TryStart()
{
	--m_predecessorCount;
	if( m_predecessorCount <= 0 )
	{
		categories[m_category]->Enqueue( this );
		return true;
	}
	return false;
}

//--------------------------------------------------------------------------
/**
* FinishJob
*/
void Job::FinishJob()
{
	for( Job* successor : m_successors )
	{
		successor->TryStart();
	}

	if ( m_finishedCallback )
	{
		categories[m_category]->EnqueueFinished( this );
	}
}
