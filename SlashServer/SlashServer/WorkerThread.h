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
	virtual void Run();

private:
	GameObjectManager* objectManager_;
};