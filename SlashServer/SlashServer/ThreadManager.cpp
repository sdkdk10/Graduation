#include "ThreadManager.h"

ThreadManager::ThreadManager()
{
}

ThreadManager::~ThreadManager()
{
}

void ThreadManager::CreateThreads()
{
	Thread* acceptThread = new AcceptThread;
	AddThread(acceptThread);
	for (int i = 0; i < MAX_WORKERTHREAD_NUM; ++i)
	{
		Thread* workerThread = new WorkerThread;
		AddThread(workerThread);
	}
	Thread* timerThread = new TimerThread;
	AddThread(timerThread);
}
