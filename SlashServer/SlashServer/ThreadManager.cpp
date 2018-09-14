#include "ThreadManager.h"

ThreadManager::ThreadManager()
{
}

ThreadManager::~ThreadManager()
{
}

void ThreadManager::CreateThreads()
{
	AddThread(new AcceptThread);
	for (int i = 0; i < MAX_WORKERTHREAD_NUM; ++i)
		AddThread(new WorkerThread);
	AddThread(new TimerThread);
}
