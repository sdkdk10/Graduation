#pragma once
#include <vector>
#include "SingleTone.h"
#include "TimerThread.h"
#include "WorkerThread.h"
#include "AcceptThread.h"

class ThreadManager : public SingleTone<ThreadManager>
{
public:
	explicit ThreadManager();
	virtual ~ThreadManager();
public:
	void CreateThreads();

	void AddThread(Thread* th) {
		th->Init();
		allThreads_.push_back(th);
	}
	void JoinAllThreads() {
		for (auto& th : allThreads_) th->Join();
	}

	Thread* FindThread(int threadNum) {
		return allThreads_[threadNum];
	}

private:
	std::vector <Thread*> allThreads_;
};