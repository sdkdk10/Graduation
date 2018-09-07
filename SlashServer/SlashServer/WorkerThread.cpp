#include "WorkerThread.h"

WorkerThread::WorkerThread()
{
	objectManager_ = GameObjectManager::GET_INSTANCE()->GetGameObjectManager();
}

WorkerThread::~WorkerThread()
{
}

void WorkerThread::Run()
{
	while (true) {
		unsigned long dataSize;
		GameObject* object = nullptr; // 64비트 모드에서는 long long으로 32비트에서는 long으로
		WSAOVERLAPPED *pOver;

		BOOL isSuccess = GetQueuedCompletionStatus(Thread::GetIocp(),
			&dataSize, reinterpret_cast<PULONG_PTR>(&object), &pOver, INFINITE);

		//printf("GQCS from client [ %d ] with size [ %d ]\n", key, data_size);

		// 접속종료 처리
		if (0 == dataSize) {
			objectManager_->DisconnectPlayer(object);
			continue;
		}
		// 에러 처리
		if (0 == isSuccess) {
			//printf("Error in GQCS key[ %d ]\n", key);
			objectManager_->DisconnectPlayer(object);
			continue;
		}

		// Send/Recv 처리
		EXOver *o = reinterpret_cast<EXOver *>(pOver);
		if (EVT_RECV == o->eventType) {

			auto player = dynamic_cast<Player*>(object);

			int rSize = dataSize;
			char *ptr = o->ioBuf;
			while (0 < rSize) {
				if (0 == player->packetSize_)
					player->packetSize_ = ptr[0];
				int remain = player->packetSize_ - player->prevSize_; // 받아야하는 패킷 총 사이즈 - 이전에 받은 값
				if (remain <= rSize) { // 패킷을 만들 수 있을 때
					memcpy(player->prevPacket_ + player->prevSize_,
						ptr, remain);
					objectManager_->ProcessPacket(object, player->prevPacket_);
					rSize -= remain;
					ptr += remain;
					player->packetSize_ = 0;
					player->prevSize_ = 0;
				}
				else {// 패킷을 만들 수 없을 때
					memcpy(player->prevPacket_ + player->prevSize_,
						ptr,
						rSize);
					player->prevSize_ += rSize;
					rSize -= rSize;
					ptr += rSize;
				}
			}
			unsigned long rflag = 0;
			ZeroMemory(&o->wsaOver, sizeof(WSAOVERLAPPED)); // 재사용할거니까 초기화해줘야 한다.
			WSARecv(player->s_, &o->wsaBuf, 1, NULL,
				&rflag, &o->wsaOver, NULL);
		}
		else if (EVT_SEND == o->eventType)
		{
			delete o;
		}
		else if (EVT_CHASE == o->eventType)
		{
			EXOver *o = reinterpret_cast<EXOver *>(pOver);
			GameObject* player = o->eventTarget;
			objectManager_->ChasingPlayer(object, player);
			delete o;
		}
		else if (EVT_MONSTER_ATTACK == o->eventType)
		{
			EXOver *o = reinterpret_cast<EXOver *>(pOver);
			GameObject* target = o->eventTarget;
			objectManager_->MonsterAttack(object, target);
			delete o;
		}
		else if (EVT_PLAYER_ATTACK == o->eventType)
		{
			EXOver *o = reinterpret_cast<EXOver *>(pOver);
			objectManager_->PlayerAttack(object);
			delete o;
		}
		else if (EVT_MONSTER_DAMAGED == o->eventType)
		{
			EXOver *o = reinterpret_cast<EXOver *>(pOver);
			GameObject* target = o->eventTarget;
			objectManager_->MonsterDamaged(object, target);
			delete o;
		}
		else if (EVT_PLAYER_DAMAGED == o->eventType)
		{
			EXOver *o = reinterpret_cast<EXOver *>(pOver);
			GameObject* target = o->eventTarget;
			objectManager_->PlayerDamaged(object, target);
			delete o;
		}
		else if (EVT_MONSTER_RESPOWN == o->eventType)
		{
			EXOver *o = reinterpret_cast<EXOver *>(pOver);
			objectManager_->MonsterRespown(object);
			delete o;
		}
		else if (EVT_PLAYER_RESPOWN == o->eventType)
		{
			EXOver *o = reinterpret_cast<EXOver *>(pOver);
			objectManager_->PlayerRespown(object);
			delete o;
		}
		else {

		}
	}
}