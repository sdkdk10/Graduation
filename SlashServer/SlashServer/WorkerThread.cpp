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
		GameObject* object = nullptr; // 64��Ʈ ��忡���� long long���� 32��Ʈ������ long����
		WSAOVERLAPPED *pOver;

		BOOL isSuccess = GetQueuedCompletionStatus(Thread::GetIocp(),
			&dataSize, reinterpret_cast<PULONG_PTR>(&object), &pOver, INFINITE);

		//printf("GQCS from client [ %d ] with size [ %d ]\n", key, data_size);

		// �������� ó��
		if (0 == dataSize) {
			objectManager_->DisconnectPlayer(object);
			continue;
		}
		// ���� ó��
		if (0 == isSuccess) {
			//printf("Error in GQCS key[ %d ]\n", key);
			objectManager_->DisconnectPlayer(object);
			continue;
		}

		// Send/Recv ó��
		EXOver *o = reinterpret_cast<EXOver *>(pOver);
		if (EVT_RECV == o->eventType) {

			auto player = dynamic_cast<Player*>(object);

			int rSize = dataSize;
			char *ptr = o->ioBuf;
			while (0 < rSize) {
				if (0 == player->packetSize_)
					player->packetSize_ = ptr[0];
				int remain = player->packetSize_ - player->prevSize_; // �޾ƾ��ϴ� ��Ŷ �� ������ - ������ ���� ��
				if (remain <= rSize) { // ��Ŷ�� ���� �� ���� ��
					memcpy(player->prevPacket_ + player->prevSize_,
						ptr, remain);
					objectManager_->ProcessPacket(object, player->prevPacket_);
					rSize -= remain;
					ptr += remain;
					player->packetSize_ = 0;
					player->prevSize_ = 0;
				}
				else {// ��Ŷ�� ���� �� ���� ��
					memcpy(player->prevPacket_ + player->prevSize_,
						ptr,
						rSize);
					player->prevSize_ += rSize;
					rSize -= rSize;
					ptr += rSize;
				}
			}
			unsigned long rflag = 0;
			ZeroMemory(&o->wsaOver, sizeof(WSAOVERLAPPED)); // �����ҰŴϱ� �ʱ�ȭ����� �Ѵ�.
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