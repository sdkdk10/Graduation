#include "ThreadManager.h"
#include "GameObjectManager.h"
#include "MiniDump.h"

int main()
{
	MiniDump::Begin();

	GameObjectManager::GET_INSTANCE()->InitGameObjects();
	ThreadManager::GET_INSTANCE()->CreateThreads();
	ThreadManager::GET_INSTANCE()->JoinAllThreads();

	WSACleanup();
	MiniDump::End();
}