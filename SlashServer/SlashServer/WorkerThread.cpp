#include "WorkerThread.h"
#include "SendManager.h"

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
		unsigned long dataSize{};
		ULONG_PTR compKey{};
		GameObject* object = nullptr; // 64��Ʈ ��忡���� long long���� 32��Ʈ������ long����
		WSAOVERLAPPED *pOver;

		BOOL isSuccess = GetQueuedCompletionStatus(Thread::GetIocp(),
			&dataSize, reinterpret_cast<ULONG_PTR*>(&object), &pOver, INFINITE);

		//printf("GQCS from client [ %d ] with size [ %d ]\n", object->ID_, dataSize); // ������ �ڵ�

		// �������� ó��
		if (0 == dataSize) {
			objectManager_->DisconnectPlayer(object);
			continue;
		}
		// ���� ó��
		if (0 == isSuccess) {
			printf("Error in GQCS client[ %d ]\n", object->ID_);
			objectManager_->DisconnectPlayer(object);
			continue;
		}

		// Send/Recv ó��
		EXOver *o = reinterpret_cast<EXOver *>(pOver);
		if (EVT_RECV == o->eventType) {

			Player* player = static_cast<Player*>(object);

			int rSize = dataSize;
			char *ptr = o->ioBuf;
			while (0 < rSize) {
				if (0 == player->packetSize_)
					player->packetSize_ = ptr[0];
				int remain = player->packetSize_ - player->prevSize_;
				// ��Ŷ�� ���� �� ���� ��
				if (remain <= rSize) {
					memcpy(player->prevPacket_ + player->prevSize_,
						ptr, remain);
					objectManager_->ProcessPacket(object, player->prevPacket_);
					rSize -= remain;
					ptr += remain;
					player->packetSize_ = 0;
					player->prevSize_ = 0;
				}
				// ��Ŷ�� ���� �� ���� ��
				else {
					memcpy(player->prevPacket_ + player->prevSize_,
						ptr,
						rSize);
					player->prevSize_ += rSize;
					rSize -= rSize;
					ptr += rSize;
				}
			}
			unsigned long rflag = 0;
			ZeroMemory(&o->wsaOver, sizeof(WSAOVERLAPPED)); // wsaOver ����
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
		else if (EVT_WARRIOR_ATTACK1 == o->eventType)
		{
			EXOver *o = reinterpret_cast<EXOver *>(pOver);
			objectManager_->ProcessWarriorAttack1(object);
			delete o;
		}
		else if (EVT_WARRIOR_ATTACK2 == o->eventType)
		{
			EXOver *o = reinterpret_cast<EXOver *>(pOver);
			objectManager_->ProcessWarriorAttack2(object);
			delete o;
		}
		else if (EVT_WARRIOR_ATTACK3 == o->eventType)
		{
			EXOver *o = reinterpret_cast<EXOver *>(pOver);
			objectManager_->ProcessWarriorAttack3(object);
			delete o;
		}
		else if (EVT_WIZARD_ATTACK1 == o->eventType)
		{
			EXOver *o = reinterpret_cast<EXOver *>(pOver);
			objectManager_->ProcessWizardAttack1(object);
			delete o;
		}
		else if (EVT_WIZARD_ATTACK2 == o->eventType)
		{
			EXOver *o = reinterpret_cast<EXOver *>(pOver);
			objectManager_->ProcessWizardAttack2(object);
			delete o;
		}
		else if (EVT_WIZARD_ATTACK3 == o->eventType)
		{
			EXOver *o = reinterpret_cast<EXOver *>(pOver);
			objectManager_->ProcessWizardAttack3(object);
			delete o;
		}
		else if (EVT_MONSTER_DAMAGED == o->eventType)
		{
			EXOver *o = reinterpret_cast<EXOver *>(pOver);
			GameObject* target = o->eventTarget;

			if (dynamic_cast<Player*>(target)->isWarriorUltimateMode)
				target->dmg_ *= 2;

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
		else if (EVT_MONSTER_RESPAWN == o->eventType)
		{
			EXOver *o = reinterpret_cast<EXOver *>(pOver);
			objectManager_->MonsterRespown(object);
			delete o;
		}
		else if (EVT_PLAYER_RESPAWN == o->eventType)
		{
			EXOver *o = reinterpret_cast<EXOver *>(pOver);
			objectManager_->PlayerRespown(object);
			delete o;
		}
		else if (EVT_PLAYER_ROLL == o->eventType)
		{
			EXOver *o = reinterpret_cast<EXOver *>(pOver);
			objectManager_->ProcessMove(object, dynamic_cast<Player*>(object)->rollDir);
			delete o;
		}
		else if (EVT_PLAY_ENDING == o->eventType)
		{
			EXOver *o = reinterpret_cast<EXOver *>(pOver);
			SendManager::SendPlayEnding(object);
			delete o;
		}
		else {

		}
	}
}