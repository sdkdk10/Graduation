#include "GameObjectManager.h"
#include "SendManager.h"
#include "CollisionUtil.h"

GameObjectManager::GameObjectManager()
{
	threadManager_ = ThreadManager::GET_INSTANCE();
}

GameObjectManager::~GameObjectManager()
{
}

void GameObjectManager::InitGameObjects()
{
	for (auto& d : playerArray_)
	{
		d = new Player;
		d->Initialize();
	}
	for (int i = 0; i < NUM_OF_NPC_TOTAL; ++i)
	{
		npcArray_[i] = new NPC;
		npcArray_[i]->ID_ = i;

		if (i < NAGAGUARD_ID_START)
			dynamic_cast<NPC*>(npcArray_[i])->npcType_ = NPC_SPIDER;
		else if (i < ROCKWARRIOR_ID_START)
			dynamic_cast<NPC*>(npcArray_[i])->npcType_ = NPC_NAGA_GUARD;
		else if (i < TREEGUARD_ID_START)
			dynamic_cast<NPC*>(npcArray_[i])->npcType_ = NPC_ROCK_WARRIOR;
		else if (i < TURTLE_ID_START)
			dynamic_cast<NPC*>(npcArray_[i])->npcType_ = NPC_TREE_GUARD;
		else
			dynamic_cast<NPC*>(npcArray_[i])->npcType_ = NPC_TURTLE;

		npcArray_[i]->Initialize();
	}
	for (auto& d : mapObjectArray_)
	{
		d = new MapObject;
		d->Initialize();
	}
}

void GameObjectManager::PutNewPlayer(GameObject* player)
{
	auto pPlayer = dynamic_cast<Player*>(player);

	pPlayer->hp_ = 200;
	pPlayer->world_._41 = 15;
	pPlayer->world_._42 = 0;
	pPlayer->world_._43 = 0;

	pPlayer->vlm_.lock();
	pPlayer->viewList_.clear();
	pPlayer->vlm_.unlock();

	SendManager::SendPutPlayer(player, player);

	// ���� ������ ��� �÷��̾�� �˸�
	for (int i = 0; i < NUM_OF_PLAYER; ++i)
	{
		if (false == playerArray_[i]->isActive_) continue;
		if (false == playerArray_[i]->CanSee(player)) continue;
		if (i == player->ID_) continue;

		SendManager::SendPutPlayer(playerArray_[i], player);

		dynamic_cast<Player*>(playerArray_[i])->vlm_.lock();
		dynamic_cast<Player*>(playerArray_[i])->viewList_.insert(player);
		dynamic_cast<Player*>(playerArray_[i])->vlm_.unlock();
	}
	// �������� �ٸ� �÷��̾� ������ ����
	for (int i = 0; i < NUM_OF_PLAYER; ++i) {
		if (false == playerArray_[i]->isActive_) continue;
		if (false == playerArray_[i]->CanSee(player)) continue;
		if (i == player->ID_) continue;

		SendManager::SendPutPlayer(player, playerArray_[i]);

		pPlayer->vlm_.lock();
		pPlayer->viewList_.insert(playerArray_[i]);
		pPlayer->vlm_.unlock();
	}
	for (int i = 0; i < NUM_OF_NPC_TOTAL; i++)
	{
		if (false == pPlayer->CanSee(npcArray_[i])) continue; // �̰� ������ Ȱ��ȭ�Ǿ� �ִ°��� Ȯ���ؾ���
		if (pPlayer->IsInAgroRange(npcArray_[i]))
			WakeUpNPC(npcArray_[i], player); // ���� 2�� ���� �����ؾ���

		SendManager::SendPutMonster(player, npcArray_[i]);

		pPlayer->vlm_.lock();
		pPlayer->viewList_.insert(npcArray_[i]);
		pPlayer->vlm_.unlock();

		//cout << "���� : " << i << " ��° ���� ����" << endl;
	}
}

void GameObjectManager::DisconnectPlayer(GameObject* player) {

	auto pPlayer = dynamic_cast<Player*>(player);

	closesocket(pPlayer->s_); // closesocket ���� �ؾ��� // ��Ƽ ������� 
	pPlayer->isActive_ = false;
	printf("%d�� �÷��̾� ��������.\n", player->ID_);

	sc_packet_remove_object p;
	p.id = player->ID_;
	p.size = sizeof(p);
	p.type = SC_REMOVE_OBJECT;

	pPlayer->vlm_.lock();
	unordered_set <GameObject*> vlCopy = pPlayer->viewList_;
	pPlayer->vlm_.unlock();
	pPlayer->viewList_.clear();

	for (auto object : vlCopy) {
		if (TYPE_PLAYER != object->objectType_) continue;
		if (false == object->isActive_) continue;

		auto playerObject = dynamic_cast<Player*>(object);
		playerObject->vlm_.lock();
		if (0 != playerObject->viewList_.count(player)) {
			playerObject->viewList_.erase(player);
			playerObject->vlm_.unlock();
			SendManager::SendPacket(object, &p);
		}
	
		else
			playerObject->vlm_.unlock();
	}
}


void GameObjectManager::WakeUpNPC(GameObject* npc, GameObject* target)
{
	if (npc->isActive_) return;
	if (STATE_DEAD == npc->state_) return;
	npc->isActive_ = true;
	npc->state_ = STATE_WALK;

	for (int i = 0; i < NUM_OF_PLAYER; ++i)
	{
		if (false == playerArray_[i]->isActive_) continue;
		if (false == npc->CanSee(playerArray_[i])) continue;

		SendManager::SendObjectState(playerArray_[i], npc);
	}

	dynamic_cast<TimerThread*>(threadManager_->FindThread(TIMER_THREAD))->AddTimer(npc, EVT_CHASE, GetTickCount(), target);
}

void GameObjectManager::ChasingPlayer(GameObject* npc, GameObject* player) {

	if (npc->state_ == STATE_DEAD)
		return;

	XMFLOAT3 monsterLook{};
	
	auto npcType = dynamic_cast<NPC*>(npc)->npcType_;

	if (NPC_SPIDER == npcType)
		monsterLook = XMFLOAT3(npc->world_._31, npc->world_._32, npc->world_._33);
	else
		monsterLook = XMFLOAT3(npc->world_._21, npc->world_._22, npc->world_._23);

	XMFLOAT3 monsterPos = XMFLOAT3(npc->world_._41, npc->world_._42, npc->world_._43);

	XMFLOAT3 playerPos = XMFLOAT3(player->world_._41, player->world_._42, player->world_._43);
	XMFLOAT3 shift = Vector3::Normalize(monsterLook);

	XMFLOAT3 dirVector = Vector3::Subtract(playerPos, monsterPos);   // ��ü���� �÷��̾�� ���� ����

	//auto n_dirVector = Vector3::Normalize(dirVector);
	dirVector = Vector3::Normalize(dirVector);
	XMFLOAT3 crossVector = Vector3::CrossProduct(shift, dirVector, true);

	float dotproduct = Vector3::DotProduct(shift, dirVector);
	float shifttLength = Vector3::Length(shift);
	float dirVectorLength = Vector3::Length(dirVector);

	float cosCeta = (dotproduct / shifttLength * dirVectorLength);

	float ceta = acos(cosCeta);

	ceta = ceta * RADIAN_TO_DEGREE;

	XMFLOAT3 movingReflectVector = XMFLOAT3(0, 0, 0); // ���߿� �浹�ϸ� ����������� �̵��ϱ�����

	bool IsRotated = false;

	if (ceta > 5.6f)
	{
		XMMATRIX mtxRotate{};

		if (crossVector.y > 0)
		{	
			if(NPC_SPIDER == npcType)
			{
				mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(0.f), XMConvertToRadians(MONSTER_ROTATE_DEGREE), XMConvertToRadians(0.f));
				npc->lookDegree_ += MONSTER_ROTATE_DEGREE;
			}
			else
			{
				mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(0.f), XMConvertToRadians(0.f), XMConvertToRadians(-MONSTER_ROTATE_DEGREE));
				npc->lookDegree_ += (360.0 - MONSTER_ROTATE_DEGREE);
			}
		}
		else if (crossVector.y < 0)
		{
			if (NPC_SPIDER == npcType)
			{
				mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(0.f), XMConvertToRadians(-MONSTER_ROTATE_DEGREE), XMConvertToRadians(0.f));
				npc->lookDegree_ += (360.0 - MONSTER_ROTATE_DEGREE);
			}
			else
			{
				mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(0.f), XMConvertToRadians(0.f), XMConvertToRadians(MONSTER_ROTATE_DEGREE));
				npc->lookDegree_ += MONSTER_ROTATE_DEGREE;
			}
		}
		npc->world_ = Matrix4x4::Multiply(mtxRotate, npc->world_);
		IsRotated = true;
	}

	if (npc->lookDegree_ >= 360.f)
		npc->lookDegree_ -= 360.f;

	XMFLOAT3 movingVector = XMFLOAT3(dirVector.x * MONSTER_MOVE_SPEED, dirVector.y * MONSTER_MOVE_SPEED, dirVector.z * MONSTER_MOVE_SPEED); 

	unordered_set <GameObject*> oldVL;
	for (int id = 0; id < NUM_OF_PLAYER; ++id)
		if (true == playerArray_[id]->isActive_)
			if (true == playerArray_[id]->CanSee(npc)) {
				oldVL.insert(playerArray_[id]);
			}

	bool IsNPCColl = false;
	bool IsMapObjectColl = false;

	for (int i = 0; i < NUM_OF_NPC_TOTAL; ++i)
	{
		if (false == npcArray_[i]->isActive_) continue;
		if (false == npcArray_[i]->IsClose(npc)) continue;
		if (i == npc->ID_) continue;

		IsNPCColl = CollisionUtil::ProcessObjectColl(npcArray_[i], npc, movingVector);
		if (IsNPCColl)
			break;
	}

	for (auto& mapObject : mapObjectArray_)
	{
		if (false == mapObject->isActive_) continue;
		if (false == npc->CanSeeMapObject(mapObject)) continue;

		IsMapObjectColl = CollisionUtil::ProcessMapObjectColl(mapObject, npc, movingVector);
		if (IsMapObjectColl)
			break;
	}

	if (!IsNPCColl && !IsMapObjectColl)
		npc->world_._41 += movingVector.x, npc->world_._42 += movingVector.y, npc->world_._43 += movingVector.z;


	npc->xmOOBBTransformed_.Transform(npc->xmOOBB_, XMLoadFloat4x4(&npc->world_));
	XMStoreFloat4(&npc->xmOOBBTransformed_.Orientation, XMQuaternionNormalize(XMLoadFloat4(&npc->xmOOBBTransformed_.Orientation)));

	unordered_set<GameObject*> newVL;
	for (int i = 0; i < NUM_OF_PLAYER; ++i)
	{
		if (false == playerArray_[i]->isActive_) continue;
		if (false == playerArray_[i]->CanSee(npc)) continue;

		newVL.insert(playerArray_[i]);
	}

	// PutObject
	for (auto object : newVL)
	{
		auto newPlayer = dynamic_cast<Player*>(object);

		newPlayer->vlm_.lock();
		if (0 == newPlayer->viewList_.count(npc))
		{
			newPlayer->viewList_.insert(npc);
			newPlayer->vlm_.unlock();
			SendManager::SendPutMonster(object, npc);
		}
		else
		{
			newPlayer->vlm_.unlock();
			SendManager::SendObjectWalk(object, npc);
			if (IsRotated)
				SendManager::SendObjectLook(object, npc);
		}
	}
	// RemoveObject
	for (auto object : oldVL)
	{
		if (0 == newVL.count(object))
		{
			auto oldPlayer = dynamic_cast<Player*>(object);
			oldPlayer->vlm_.lock();
			if (0 != oldPlayer->viewList_.count(npc)) {
				oldPlayer->viewList_.erase(npc);
				oldPlayer->vlm_.unlock();
				SendManager::SendRemoveObject(object, npc);
			}
			else {
				oldPlayer->vlm_.unlock();
			}
		}
	}

	if (false == npc->CanSee(player))
	{
		for (int i = 0; i < NUM_OF_PLAYER; ++i)
		{
			if (false == playerArray_[i]->isActive_) continue;
			if (false == playerArray_[i]->CanSee(npc)) continue;
			if (playerArray_[i]->IsInAgroRange(npc))
			{
				dynamic_cast<TimerThread*>(threadManager_->FindThread(TIMER_THREAD))->AddTimer(npc, EVT_CHASE, GetTickCount() + 50, playerArray_[i]);
				return;
			}
		}

		npc->state_ = STATE_IDLE;
		npc->isActive_ = false;

		for (int i = 0; i < NUM_OF_PLAYER; ++i)
		{
			if (false == playerArray_[i]->isActive_) continue;
			if (false == playerArray_[i]->CanSee(npc)) continue;
			SendManager::SendObjectState(playerArray_[i], npc);
		}
	}
	else if (player->IsClose(npc))	 //�浹�� ������ ����� ������
	{
		if (STATE_ATTACK1 == npc->state_) return;

		dynamic_cast<TimerThread*>(threadManager_->FindThread(TIMER_THREAD))->AddTimer(npc, EVT_MONSTER_ATTACK, GetTickCount(), player);

		// ���� ������ ���·� �����ϰ� Ŭ��� ������ (�׳� Ŭ�󿡼� ��� ����ϴٰ� �Ǵ��ұ�?)
		// �׸��� ���� �� �÷��̾� ü�� ��´�.
	}
	else
	{
		dynamic_cast<TimerThread*>(threadManager_->FindThread(TIMER_THREAD))->AddTimer(npc, EVT_CHASE, GetTickCount() + 50, player);
	}
}

void GameObjectManager::MonsterAttack(GameObject* monster, GameObject* player) {

	if (player->isActive_ == false)
		return;
	if (monster->state_ == STATE_DEAD)
		return;

	if (false == player->IsClose(monster))
	{
		monster->state_ = STATE_WALK;

		for (int i = 0; i < NUM_OF_PLAYER; ++i)
		{
			if (false == playerArray_[i]->isActive_) continue;
			if (false == playerArray_[i]->CanSee(monster)) continue;
			SendManager::SendObjectState(playerArray_[i], monster);
		}
		dynamic_cast<TimerThread*>(threadManager_->FindThread(TIMER_THREAD))->AddTimer(monster, EVT_CHASE, GetTickCount(), player);
		return;
	}
	else
	{
		monster->state_ = STATE_ATTACK1;

		for (int i = 0; i < NUM_OF_PLAYER; ++i)
		{
			if (false == playerArray_[i]->isActive_) continue;
			if (false == playerArray_[i]->CanSee(monster)) continue;
			SendManager::SendObjectState(playerArray_[i], monster);
		}
		dynamic_cast<TimerThread*>(threadManager_->FindThread(TIMER_THREAD))->AddTimer(monster, EVT_PLAYER_DAMAGED, GetTickCount() + 300, player);

		if ((player->hp_ - monster->dmg_) > 0)
		{
			dynamic_cast<TimerThread*>(threadManager_->FindThread(TIMER_THREAD))->AddTimer(monster, EVT_MONSTER_ATTACK, GetTickCount() + 1200, player);
		}
		else
		{
			monster->state_ = STATE_IDLE;
			monster->isActive_ = false;

			for (int i = 0; i < NUM_OF_PLAYER; ++i)
			{
				if (false == playerArray_[i]->isActive_) continue;
				if (false == playerArray_[i]->CanSee(monster)) continue;
				SendManager::SendObjectState(playerArray_[i], monster);
			}
		}
	}
}

void GameObjectManager::PlayerAttack(GameObject* player) {

	for (int i = 0; i < NUM_OF_NPC_TOTAL; ++i) // �丮��Ʈ�� �ִ� �ֵ� ���°� �� ��������..
	{
		if (false == npcArray_[i]->isActive_) continue;
		if (false == npcArray_[i]->IsAttackRange(player)) continue;

		// ���Ͱ� �տ� �ִ°� ���⿡�� �Ǵ�

		dynamic_cast<TimerThread*>(threadManager_->FindThread(TIMER_THREAD))->AddTimer(player, EVT_MONSTER_DAMAGED, GetTickCount() + 50, npcArray_[i]);
	}

}

void GameObjectManager::PlayerDamaged(GameObject* monster, GameObject* player) {

	if (player->state_ == STATE_DEAD)
		return;

	player->hp_ -= monster->dmg_;

	if ((player->hp_ - monster->dmg_) < 0)
		player->hp_ = 0;

	SendManager::SendObjectHp(player, player);

	if (player->hp_ <= 0)
	{
		player->state_ = STATE_DEAD;

		for (int i = 0; i < NUM_OF_PLAYER; ++i)
		{
			if (false == playerArray_[i]->isActive_) continue;
			if (false == playerArray_[i]->CanSee(player)) continue;
			SendManager::SendObjectState(playerArray_[i], player);
		}

		dynamic_cast<TimerThread*>(threadManager_->FindThread(TIMER_THREAD))->AddTimer(player, EVT_PLAYER_RESPOWN, GetTickCount() + 5000, nullptr);
	}
}

void GameObjectManager::MonsterDamaged(GameObject* player, GameObject* monster) {

	if (monster->state_ == STATE_DEAD)
		return;

	monster->hp_ -= player->dmg_;

	if ((monster->hp_ - player->dmg_) < 0)
		monster->hp_ = 0;

	if (monster->hp_ <= 0)
	{
		monster->state_ = STATE_DEAD;
		monster->isActive_ = false;

		for (int i = 0; i < NUM_OF_PLAYER; ++i)
		{
			if (false == playerArray_[i]->isActive_) continue;
			if (false == playerArray_[i]->CanSee(monster)) continue;
			SendManager::SendObjectState(playerArray_[i], monster);
		}

		dynamic_cast<TimerThread*>(threadManager_->FindThread(TIMER_THREAD))->AddTimer(monster, EVT_MONSTER_RESPOWN, GetTickCount() + 50000, nullptr);
	}
}

void GameObjectManager::PlayerRespown(GameObject* player)
{
	if (false == player->isActive_)
		return;

	player->state_ = STATE_IDLE;

	PutNewPlayer(player);
}

void GameObjectManager::MonsterRespown(GameObject* monster)
{

	monster->state_ = STATE_IDLE;

	monster->hp_ = 5;

	for (int i = 0; i < NUM_OF_PLAYER; ++i)
	{
		if (false == playerArray_[i]->isActive_) continue;
		if (false == playerArray_[i]->CanSee(monster)) continue;

		SendManager::SendObjectState(playerArray_[i], monster);

		if (false == playerArray_[i]->IsInAgroRange(monster)) continue;
		ChasingPlayer(monster, playerArray_[i]);
		break;
	}
}

void GameObjectManager::ProcessMove(GameObject* player, unsigned char dirType, unsigned char moveType)
{
	auto pPlayer = dynamic_cast<Player*>(player);

	float moveSpeed = 0;

	if (SC_WALK_MOVE == moveType)
	{
		player->state_ = STATE_WALK;
		moveSpeed = PLAYER_MOVE_SPEED;
	}
	else
	{
		player->state_ = STATE_ROLL;
		moveSpeed = PLAYER_ROLL_SPEED;
		pPlayer->rollDir = dirType;
	}

	XMFLOAT3 xmf3Shift{ 0.0f, 0.0f, 0.0f };

	if (CS_DIR_FORWARD & dirType) xmf3Shift = Vector3::Add(xmf3Shift, const_cast<XMFLOAT3&>(DIR_VECTOR_Z), moveSpeed);
	if (CS_DIR_BACKWARD & dirType) xmf3Shift = Vector3::Add(xmf3Shift, const_cast<XMFLOAT3&>(DIR_VECTOR_Z), -moveSpeed);
	if (CS_DIR_RIGHT & dirType) xmf3Shift = Vector3::Add(xmf3Shift, const_cast<XMFLOAT3&>(DIR_VECTOR_X), moveSpeed);
	if (CS_DIR_LEFT & dirType) xmf3Shift = Vector3::Add(xmf3Shift, const_cast<XMFLOAT3&>(DIR_VECTOR_X), -moveSpeed);

	XMFLOAT3 SlideVector{};

	bool IsRotated = false;

	XMFLOAT3 playerLook = XMFLOAT3(-player->world_._21, -player->world_._22, -player->world_._23);
	playerLook = Vector3::Normalize(playerLook);
	XMFLOAT3 n_xmf3Shift = Vector3::Normalize(xmf3Shift);
	XMFLOAT3 crossVector = Vector3::CrossProduct(n_xmf3Shift, playerLook, true);

	float dotproduct = Vector3::DotProduct(n_xmf3Shift, playerLook);
	float xmf3ShiftLength = Vector3::Length(n_xmf3Shift);
	float xmf3PlayerLooklength = Vector3::Length(playerLook);

	float cosCeta = dotproduct / xmf3ShiftLength * xmf3PlayerLooklength;

	float ceta = acos(cosCeta); // ���� ����

	ceta = ceta * RADIAN_TO_DEGREE;

	//cout << ceta << endl;

	if (ceta > 8.0f)
	{
		XMMATRIX mtxRotate{};

		if (crossVector.y > 0)
		{
			mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(0.f), XMConvertToRadians(0.f), XMConvertToRadians(-PLAYER_ROTATE_DEGREE));
			player->lookDegree_ += (360 - PLAYER_ROTATE_DEGREE);
		}
		else
		{
			mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(0.f), XMConvertToRadians(0.f), XMConvertToRadians(PLAYER_ROTATE_DEGREE));
			player->lookDegree_ += PLAYER_ROTATE_DEGREE;
		}

		if (player->lookDegree_ >= 360.f)
			player->lookDegree_ -= 360.f;

		player->world_ = Matrix4x4::Multiply(mtxRotate, player->world_);
		IsRotated = true;
	}

	player->xmOOBBTransformed_.Transform(player->xmOOBB_, XMLoadFloat4x4(&(player->world_)));
	XMStoreFloat4(&player->xmOOBBTransformed_.Orientation, XMQuaternionNormalize(XMLoadFloat4(&player->xmOOBBTransformed_.Orientation)));

	bool IsPlayerColl = false;
	bool IsNPCColl = false;
	bool IsMapObjectColl = false;

	// �÷��̾� �̵��� �浹üũ

	for (int i = 0; i < NUM_OF_PLAYER; ++i)
	{
		if (false == playerArray_[i]->isActive_) continue;
		if (false == playerArray_[i]->IsClose(player)) continue;
		if (i == player->ID_) continue;

		IsPlayerColl = CollisionUtil::ProcessObjectColl(playerArray_[i], player, xmf3Shift);
		if (IsPlayerColl)
			break;
	}

	for (int i = 0; i < NUM_OF_NPC_TOTAL; ++i)
	{
		if (false == npcArray_[i]->isActive_) continue;
		if (false == npcArray_[i]->IsClose(player)) continue;

		IsNPCColl = CollisionUtil::ProcessObjectColl(npcArray_[i], player, xmf3Shift);
		if (IsNPCColl)
			break;
	}

	for (auto& mapObject : mapObjectArray_)
	{
		if (false == mapObject->isActive_) continue;
		if (false == player->CanSeeMapObject(mapObject)) continue;

		IsMapObjectColl = CollisionUtil::ProcessMapObjectColl(mapObject, player, xmf3Shift);
		if (IsMapObjectColl)
			break;
	}

	if (!IsPlayerColl && !IsNPCColl && !IsMapObjectColl)
		player->world_._41 += xmf3Shift.x, player->world_._42 += xmf3Shift.y, player->world_._43 += xmf3Shift.z;

	sc_packet_move sp_pos;
	sp_pos.id = player->ID_;
	sp_pos.size = sizeof(sc_packet_move);
	sp_pos.type = moveType;
	sp_pos.posX = player->world_._41;
	sp_pos.posY = player->world_._42;
	sp_pos.posZ = player->world_._43;

	sc_packet_look_degree sp_rotate;
	sp_rotate.id = player->ID_;
	sp_rotate.size = sizeof(sc_packet_look_degree);
	sp_rotate.type = SC_ROTATE;
	sp_rotate.lookDegree = player->lookDegree_;

	//////////////

	unordered_set <GameObject*> new_view_list; // ���ο� �丮��Ʈ ����
	for (int i = 0; i < NUM_OF_PLAYER; ++i) {
		if (i == player->ID_) continue;
		if (false == playerArray_[i]->isActive_) continue;
		if (false == playerArray_[i]->CanSee(player)) continue;
		// ���ο� �丮��Ʈ (���� ����) // ���� �̵��ϰ� ���̴� ��� �ֵ�
		new_view_list.insert(playerArray_[i]);
	}
	for (int i = 0; i < NUM_OF_NPC_TOTAL; ++i) {
		if (false == npcArray_[i]->CanSee(player)) continue;
		new_view_list.insert(npcArray_[i]);
	}

	SendManager::SendPacket(player, &sp_pos);
	if (IsRotated)
		SendManager::SendPacket(player, &sp_rotate);

	for (auto& object : new_view_list) {

		if (TYPE_MONSTER == object->objectType_)
			WakeUpNPC(object, player);

		pPlayer->vlm_.lock();
		// ���� ���� �丮��Ʈ���� ������ // �� ���� ���Դ�
		if (0 == pPlayer->viewList_.count(object))
			pPlayer->viewList_.insert(object);

		pPlayer->vlm_.unlock();

		if (TYPE_MONSTER == object->objectType_)
			SendManager::SendPutMonster(player, object);
		else
			SendManager::SendPutPlayer(player, object);

		if (TYPE_MONSTER == object->objectType_) continue;

		auto objectPlayer = dynamic_cast<Player*>(object);

		objectPlayer->vlm_.lock();
		// �������� ���� ������? // �߰�
		if (0 == objectPlayer->viewList_.count(player)) {
			objectPlayer->viewList_.insert(player);
			objectPlayer->vlm_.unlock();
			SendManager::SendPutPlayer(object, player);
		}
		// �������� ���� �־���? // ��ġ����
		else
		{
			objectPlayer->vlm_.unlock();
			SendManager::SendPacket(object, &sp_pos);
			if (IsRotated)
				SendManager::SendPacket(object, &sp_rotate);
		}
	}
	// ���� ���� �丮��Ʈ�� �ִ� �ֵ�
	pPlayer->vlm_.lock();
	unordered_set <GameObject*> old_v = pPlayer->viewList_;
	pPlayer->vlm_.unlock();
	for (auto& object : old_v) {
		if (0 == new_view_list.count(object)) {
			if (player->ID_ == object->ID_) continue;
			pPlayer->vlm_.lock();
			pPlayer->viewList_.erase(object); // ��� �� ������� ���� ���� �����긮��Ʈ����� �ѹ��� ����°� ����
			pPlayer->vlm_.unlock();
			SendManager::SendRemoveObject(player, object);

			if (TYPE_PLAYER != object->objectType_) continue;

			auto objectPlayer = dynamic_cast<Player*>(object);

			objectPlayer->vlm_.lock();
			// ���� ��쿡�� ����� ����.
			if (0 != objectPlayer->viewList_.count(player)) {
				objectPlayer->viewList_.erase(player);
				objectPlayer->vlm_.unlock();
				SendManager::SendRemoveObject(object, player);
			}
			else {
				objectPlayer->vlm_.unlock();
			}
		}
	}

	if (SC_ROLL_MOVE == moveType)
	{
		if (++(pPlayer->rollCurFrame) >= ROLL_MAX_FRAME)
		{
			pPlayer->rollCurFrame = 0;
			return;
		}
		else
		{ 
			dynamic_cast<TimerThread*>(threadManager_->FindThread(TIMER_THREAD))->AddTimer(player, EVT_PLAYER_ROLL, GetTickCount() + ROLL_DELAY, nullptr);
		}
	}

	/////////////
}

void GameObjectManager::ProcessPacket(GameObject* player, char *packet)
{
	if (packet[1] <= MOVE_PACKET_END && packet[1] >= MOVE_PACKET_START)
	{
		unsigned char moveType = 0;
		unsigned char dirType = 0;

		cs_packet_dir *p = reinterpret_cast<cs_packet_dir *>(packet);

		if (p->type & CS_DIR_FORWARD) dirType |= CS_DIR_FORWARD;
		if (p->type & CS_DIR_BACKWARD) dirType |= CS_DIR_BACKWARD;
		if (p->type & CS_DIR_RIGHT) dirType |= CS_DIR_RIGHT;
		if (p->type & CS_DIR_LEFT) dirType |= CS_DIR_LEFT;

		if (p->type & CS_ROLL)
			moveType = SC_ROLL_MOVE;
		else
			moveType = SC_WALK_MOVE;

		GameObjectManager::ProcessMove(player, dirType, moveType);
	}
	else if (packet[1] == CS_STOP)
	{

		player->state_ = STATE_IDLE;
		sc_packet_state sp;
		sp.id = player->ID_;
		sp.size = sizeof(sc_packet_state);
		sp.type = SC_STATE;
		sp.state = player->state_;
		for (int i = 0; i < NUM_OF_PLAYER; ++i)
		{
			if (false == playerArray_[i]->isActive_) continue;
			if (false == playerArray_[i]->CanSee(player)) continue;
			if (player == playerArray_[i]) continue;
			SendManager::SendPacket(playerArray_[i], &sp);
		}

		return;
	}
	else if ((packet[1] == CS_ATTACK1) || (packet[1] == CS_ATTACK2) || (packet[1] == CS_ATTACK3))
	{
		if (packet[1] == CS_ATTACK1)
			player->state_ = STATE_ATTACK1;
		else if (packet[1] == CS_ATTACK2)
			player->state_ = STATE_ATTACK2;
		else if (packet[1] == CS_ATTACK3)
			player->state_ = STATE_ATTACK3;

		for (int i = 0; i < NUM_OF_PLAYER; ++i)
		{
			if (false == playerArray_[i]->isActive_) continue;
			if (false == playerArray_[i]->CanSee(player)) continue;

			SendManager::SendObjectState(playerArray_[i], player);

		}

		dynamic_cast<TimerThread*>(threadManager_->FindThread(TIMER_THREAD))->AddTimer(player, EVT_PLAYER_ATTACK, GetTickCount() + ATTACK_DELAY, nullptr);

		return;
	}
	else if (packet[1] == CS_MAP_INIT_DATA)
	{
		static int first_client = player->ID_;

		if (first_client != player->ID_)
			return;

		cs_packet_mapinitdata *p = reinterpret_cast<cs_packet_mapinitdata *>(packet);

		static int id = 0;

		mapObjectArray_[id] = new MapObject;
		mapObjectArray_[id]->isActive_ = true;
		mapObjectArray_[id]->ID_ = id;
		mapObjectArray_[id]->world_ = p->world;
		dynamic_cast<MapObject*>(mapObjectArray_[id])->Bounds = p->bounds;

		++id;

		return;
	}
	else if (packet[1] == CS_PLAYER_TYPE)
	{
		cs_packet_player_type *p = reinterpret_cast<cs_packet_player_type *>(packet);

		dynamic_cast<Player*>(player)->playerType = p->playerType;

		PutNewPlayer(player);

		return;
	}
	else
	{
		//cout << cl << " ProcessPacket Error" << endl;
		return;
	}
}