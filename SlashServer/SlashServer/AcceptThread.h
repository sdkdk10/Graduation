#pragma once
#include "Thread.h"
#include "GameObjectManager.h"

class GameObjectManager;
class AcceptThread : public Thread
{

public:
	explicit AcceptThread();
	virtual ~AcceptThread();

public:
	virtual void Run();

private:
	SOCKET socket_;
	SOCKADDR_IN bindAddr_;
	GameObjectManager* objectManager_;
};