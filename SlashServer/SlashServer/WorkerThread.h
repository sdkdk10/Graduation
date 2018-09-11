#pragma once
#include "Thread.h"
#include "GameObjectManager.h"

class GameObjectManager;
class WorkerThread : public Thread
{

public:
	explicit WorkerThread();
	virtual ~WorkerThread();

public:
	virtual void Init() {
		thread_ = std::thread([&]() { WorkerThread::Run(); });
	}
	virtual void Run();

private:
	GameObjectManager* objectManager_;
};