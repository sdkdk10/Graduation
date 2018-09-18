#include "GameObjectManager.h"
#include "SendManager.h"
#include "CollisionUtil.h"

GameObjectManager::GameObjectManager()
	:mapObjectNum_(0)
{
	threadManager_ = ThreadManager::GET_INSTANCE();
}

GameObjectManager::~GameObjectManager()
{
}

void GameObjectManager::InitGameObjects()
{
	for (GameObject*& d : playerArray_)
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
	for (GameObject*& d : mapObjectArray_)
	{
		d = new MapObject;
		d->Initialize();
	}
}

void GameObjectManager::PutNewPlayer(GameObject* player)
{
	auto pPlayer = dynamic_cast<Player*>(player);

	player->state_ = STATE_IDLE;
	pPlayer->hp_ = INIT_PLAYER_HP;
	pPlayer->world_._41 = INIT_PLAYER_POS.x;
	pPlayer->world_._42 = INIT_PLAYER_POS.y;
	pPlayer->world_._43 = INIT_PLAYER_POS.z;

	unordered_set <GameObject*> new_view_list; // 새로운 뷰리스트 생성
	for (int i = 0; i < NUM_OF_PLAYER; ++i) {
		if (false == playerArray_[i]->isActive_) continue;
		if (false == playerArray_[i]->CanSee(player)) continue;
		// 새로운 뷰리스트 (나를 기준) // 새로 이동하고 보이는 모든 애들
		new_view_list.insert(playerArray_[i]);
	}
	for (int i = 0; i < NUM_OF_NPC_TOTAL; ++i) {
		if (false == npcArray_[i]->CanSee(player)) continue;
		new_view_list.insert(npcArray_[i]);
		WakeUpNPC(npcArray_[i], player);
	}

	SendManager::SendPutPlayer(player, player);

	for (auto& object : new_view_list) {

		pPlayer->vlm_.lock();
		// 나의 기존 뷰리스트에는 없었다 // 즉 새로 들어왔다
		if (0 == pPlayer->viewList_.count(object))
			pPlayer->viewList_.insert(object);

		pPlayer->vlm_.unlock();

		if (TYPE_MONSTER == object->objectType_)
			SendManager::SendPutMonster(player, object);
		else
		{
			if(player != object)
				SendManager::SendPutPlayer(player, object);
		}

		if (TYPE_MONSTER == object->objectType_) continue;

		auto objectPlayer = dynamic_cast<Player*>(object);

		objectPlayer->vlm_.lock();
		// 상대방한테 내가 없었다? // 추가
		if (0 == objectPlayer->viewList_.count(player)) {
			objectPlayer->viewList_.insert(player);
		}
		objectPlayer->vlm_.unlock();
		SendManager::SendPutPlayer(object, player);
	}
	// 나의 이전 뷰리스트에 있는 애들
	pPlayer->vlm_.lock();
	unordered_set <GameObject*> old_v = pPlayer->viewList_;
	pPlayer->vlm_.unlock();
	for (auto& object : old_v) {
		if (0 == new_view_list.count(object)) {
			if (player->ID_ == object->ID_) continue;
			pPlayer->vlm_.lock();
			pPlayer->viewList_.erase(object); // 계속 락 언락하지 말고 따로 리무브리스트만들고 한번에 지우는게 좋음
			pPlayer->vlm_.unlock();
			SendManager::SendRemoveObject(player, object);

			if (TYPE_PLAYER != object->objectType_) continue;

			auto objectPlayer = dynamic_cast<Player*>(object);

			objectPlayer->vlm_.lock();
			// 내 시야에 없는 애들이니까 걔네 시야에도 내가 있으면 지우게 하자.
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
}

void GameObjectManager::DisconnectPlayer(GameObject* player) {

	Player* pPlayer = dynamic_cast<Player*>(player);

	closesocket(pPlayer->s_); // closesocket 먼저 해야함 // 멀티 쓰레드라서 
	pPlayer->isActive_ = false;
	printf("%d 번 플레이어 접속종료.\n", player->ID_);

	sc_packet_remove_object p;
	p.id = player->ID_;
	p.size = sizeof(p);
	p.type = SC_REMOVE_OBJECT;

	pPlayer->vlm_.lock();
	unordered_set <GameObject*> vlCopy = pPlayer->viewList_;
	pPlayer->vlm_.unlock();
	pPlayer->viewList_.clear();

	for (auto& object : vlCopy) {
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
	if (STATE_IDLE != npc->state_) return;

	if (false == npc->isActive_)
		npc->isActive_ = true;

	if (npc->IsInAgroRange(target))
	{
		npc->state_ = STATE_WALK;
		//cout << "WakeUpNPC에서 호출 npc id : " << npc->ID_ << endl;
		dynamic_cast<TimerThread*>(threadManager_->FindThread(TIMER_THREAD))->AddTimer(npc, EVT_CHASE, GetTickCount(), target);
	}
}

void GameObjectManager::ChasingPlayer(GameObject* npc, GameObject* player) {
	
	if (npc->state_ == STATE_DEAD)
		return;

	if (npc->state_ == STATE_HIT)
	{
		//cout << "ChasingPlayer(위)에서 호출 npc id : " << npc->ID_ << endl;
		npc->isActive_ = true;
		npc->state_ = STATE_WALK;
		dynamic_cast<TimerThread*>(threadManager_->FindThread(TIMER_THREAD))->AddTimer(npc, EVT_CHASE, GetTickCount() + 100, player);
		return;
	}

	npc->state_ = STATE_WALK;

	if (player->isActive_ == false || player->state_ == STATE_DEAD)
	{
		SearchNewTargetPlayer(npc);
		return;
	}

	XMFLOAT3 monsterLook{};
	
	auto npcType = dynamic_cast<NPC*>(npc)->npcType_;

	if (NPC_SPIDER == npcType)
		monsterLook = XMFLOAT3(npc->world_._31, npc->world_._32, npc->world_._33);
	else
		monsterLook = XMFLOAT3(npc->world_._21, npc->world_._22, npc->world_._23);

	XMFLOAT3 monsterPos = XMFLOAT3(npc->world_._41, npc->world_._42, npc->world_._43);

	XMFLOAT3 playerPos = XMFLOAT3(player->world_._41, player->world_._42, player->world_._43);
	XMFLOAT3 shift = Vector3::Normalize(monsterLook);

	XMFLOAT3 dirVector = Vector3::Subtract(playerPos, monsterPos);   // 객체에서 플레이어로 가는 벡터

	//auto n_dirVector = Vector3::Normalize(dirVector);
	dirVector = Vector3::Normalize(dirVector);
	XMFLOAT3 crossVector = Vector3::CrossProduct(shift, dirVector, true);

	float dotproduct = Vector3::DotProduct(shift, dirVector);
	float shifttLength = Vector3::Length(shift);
	float dirVectorLength = Vector3::Length(dirVector);

	float cosCeta = (dotproduct / shifttLength * dirVectorLength);

	float ceta = acos(cosCeta);

	ceta = ceta * RADIAN_TO_DEGREE;

	XMFLOAT3 movingReflectVector = XMFLOAT3(0, 0, 0); // 나중에 충돌하면 갱신해줘야함 이동하기전에

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
	for (auto& playerObject : newVL)
	{
		auto newPlayer = dynamic_cast<Player*>(playerObject);

		newPlayer->vlm_.lock();

		if (0 == newPlayer->viewList_.count(npc))
			newPlayer->viewList_.insert(npc);
		
		newPlayer->vlm_.unlock();
		SendManager::SendPutMonster(playerObject, npc);
	}
	// RemoveObject
	for (auto& object : oldVL)
	{
		if (0 != newVL.count(object)) continue;

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

	if (player->IsClose(npc))	 //충돌할 정도로 가까워 졌으면
	{
		dynamic_cast<TimerThread*>(threadManager_->FindThread(TIMER_THREAD))->AddTimer(npc, EVT_MONSTER_ATTACK, GetTickCount(), player);
	}
	else if(0 != player->viewList_.count(npc))
	{
		npc->isActive_ = true;
		npc->state_ = STATE_WALK;
		dynamic_cast<TimerThread*>(threadManager_->FindThread(TIMER_THREAD))->AddTimer(npc, EVT_CHASE, GetTickCount() + 50, player);
	}
	else
	{
		SearchNewTargetPlayer(npc);
	}
}

void GameObjectManager::MonsterAttack(GameObject* monster, GameObject* player) {

	if (monster->state_ == STATE_DEAD)
		return;

	if (monster->state_ == STATE_HIT)
	{
		monster->state_ = STATE_ATTACK1;
		dynamic_cast<TimerThread*>(threadManager_->FindThread(TIMER_THREAD))->AddTimer(monster, EVT_MONSTER_ATTACK, GetTickCount() + 100, player);
		return;
	}

	if (player->isActive_ == false || 
		player->state_ == STATE_DEAD)
	{
		SearchNewTargetPlayer(monster);
		return;
	}

	if (false == player->IsClose(monster))
	{
		//cout << "MonsterAttack에서 호출 npc id : " << monster->ID_ << endl;
		monster->isActive_ = true;
		monster->state_ = STATE_WALK;
		dynamic_cast<TimerThread*>(threadManager_->FindThread(TIMER_THREAD))->AddTimer(monster, EVT_CHASE, GetTickCount(), player);
		return;
	}
	else
	{
		monster->state_ = STATE_ATTACK1;

		for (int i = 0; i < NUM_OF_PLAYER; ++i)
		{
			if (false == playerArray_[i]->isActive_) continue;
			if (0 == playerArray_[i]->viewList_.count(monster)) continue;
			SendManager::SendObjectState(playerArray_[i], monster);
		}
		dynamic_cast<TimerThread*>(threadManager_->FindThread(TIMER_THREAD))->AddTimer(player, EVT_PLAYER_DAMAGED, GetTickCount() + 300, monster);
		dynamic_cast<TimerThread*>(threadManager_->FindThread(TIMER_THREAD))->AddTimer(monster, EVT_MONSTER_ATTACK, GetTickCount() + 1200, player);
	}
}

void GameObjectManager::ProcessWarriorAttack1(GameObject* player) {

	player->dmg_ = WARRIOR_SKILL1_DMG * (1 + player->level_ / 10.f);

	// -bound.center.y == playerlook방향 
	//	bound.extents.x == 가로길이
	//	bound.extents.y == 세로길이
	for (int i = 0; i < NUM_OF_NPC_TOTAL; ++i)
	{
		if (false == npcArray_[i]->isActive_) continue;
		if (false == npcArray_[i]->InWarriorAttack1Range(player)) continue;

		dynamic_cast<TimerThread*>(threadManager_->FindThread(TIMER_THREAD))->AddTimer(npcArray_[i], EVT_MONSTER_DAMAGED, GetTickCount() + 50, player);
	}
}
void GameObjectManager::ProcessWarriorAttack2(GameObject* player) {

	player->dmg_ = WARRIOR_SKILL2_DMG * (1 + player->level_ / 10.f);

	player->skillMoveRange += SKILL_MOVE_DISTANCE;

	if (player->skillMoveRange >= WARRIOR_SKILL2_MAX_RANGE)
		player->skillMoveRange = WARRIOR_SKILL2_MAX_RANGE;

	float width = 0;

	if (false == dynamic_cast<Player*>(player)->isWarriorUltimateMode)
		width = WARRIOR_SKILL2_WIDTH;
	else
		width = WARRIOR_ULTIMATE_SKILL2_WIDTH;

	player->SetSkillOOBB(XMFLOAT3(-7.f, 0.f - player->skillMoveRange, 29.0),
		XMFLOAT3(23.f * width, 16.f * WARRIOR_SKILL2_DEPTH, 28.f),
		XMFLOAT4(0.f, 0.f, 0.f, 1.f));

	player->skillOOBBTransformed_.Transform(player->skillOOBB_, XMLoadFloat4x4(&(player->world_))); // world_
	XMStoreFloat4(&player->skillOOBBTransformed_.Orientation, XMQuaternionNormalize(XMLoadFloat4(&player->skillOOBBTransformed_.Orientation)));

	for (auto& npc : player->viewList_)
	{
		if (false == npc->isActive_) continue;
		if (ObjectType::TYPE_MONSTER != npc->objectType_) continue;
		if (false == player->skillOOBB_.Contains(npc->xmOOBB_))continue;

		dynamic_cast<TimerThread*>(threadManager_->FindThread(TIMER_THREAD))->AddTimer(npc, EVT_MONSTER_DAMAGED, GetTickCount(), player);
		dynamic_cast<TimerThread*>(threadManager_->FindThread(TIMER_THREAD))->AddTimer(npc, EVT_MONSTER_DAMAGED, GetTickCount() + 300, player);

	}

	if (player->skillMoveRange >= WARRIOR_SKILL2_MAX_RANGE)
	{
		player->skillMoveRange = 0;
	}
	else
		dynamic_cast<TimerThread*>(threadManager_->FindThread(TIMER_THREAD))->AddTimer(player, EVT_WIZARD_ATTACK1, GetTickCount() + WARRIOR_SKILL2_SPEED, nullptr);
}
void GameObjectManager::ProcessWarriorAttack3(GameObject* player) {

	player->dmg_ = WARRIOR_SKILL3_DMG * (1 + player->level_ / 10.f);

	player->SetSkillOOBB(XMFLOAT3(-7.f, -50.f, 29.f),
		XMFLOAT3(23.f * WARRIOR_SKILL3_WIDTH, 16.f * WARRIOR_SKILL3_DEPTH, 28.f),
		XMFLOAT4(0.f, 0.f, 0.f, 1.f));

	player->skillOOBBTransformed_.Transform(player->skillOOBB_, XMLoadFloat4x4(&(player->world_))); // world_
	XMStoreFloat4(&player->skillOOBBTransformed_.Orientation, XMQuaternionNormalize(XMLoadFloat4(&player->skillOOBBTransformed_.Orientation)));

	for (auto& npc : player->viewList_)
	{
		if (false == npc->isActive_) continue;
		if (ObjectType::TYPE_MONSTER != npc->objectType_) continue;
		if (false == player->skillOOBB_.Contains(npc->xmOOBB_))continue;

		dynamic_cast<TimerThread*>(threadManager_->FindThread(TIMER_THREAD))->AddTimer(npc, EVT_MONSTER_DAMAGED, GetTickCount(), player);
	}
}
void GameObjectManager::ProcessWizardAttack1(GameObject* player) {

	player->dmg_ = WIZARD_SKILL1_DMG * (1 + player->level_ / 10.f);

	player->skillMoveRange += SKILL_MOVE_DISTANCE;

	if (player->skillMoveRange >= WIZARD_SKILL1_MAX_RANGE)
		player->skillMoveRange = WIZARD_SKILL1_MAX_RANGE;

	player->SetSkillOOBB(XMFLOAT3(-7.f, 0.f - player->skillMoveRange, 29.f),
		XMFLOAT3(23.f * WIZARD_SKILL1_WIDTH, 16.f * WIZARD_SKILL1_DEPTH, 28.f),
		XMFLOAT4(0.f, 0.f, 0.f, 1.f));

	player->skillOOBBTransformed_.Transform(player->skillOOBB_, XMLoadFloat4x4(&(player->world_))); // world_
	XMStoreFloat4(&player->skillOOBBTransformed_.Orientation, XMQuaternionNormalize(XMLoadFloat4(&player->skillOOBBTransformed_.Orientation)));

	for (auto& npc : player->viewList_)
	{
		if (false == npc->isActive_) continue;
		if (ObjectType::TYPE_MONSTER != npc->objectType_) continue;
		if (false == player->skillOOBB_.Contains(npc->xmOOBB_))continue;

		dynamic_cast<TimerThread*>(threadManager_->FindThread(TIMER_THREAD))->AddTimer(npc, EVT_MONSTER_DAMAGED, GetTickCount(), player);
	}

	if (player->skillMoveRange >= WIZARD_SKILL1_MAX_RANGE)
		player->skillMoveRange = 0;
	else
		dynamic_cast<TimerThread*>(threadManager_->FindThread(TIMER_THREAD))->AddTimer(player, EVT_WARRIOR_ATTACK2, GetTickCount() + WIZARD_SKILL1_SPEED, nullptr);
}
void GameObjectManager::ProcessWizardAttack2(GameObject* player) {

	player->dmg_ = WIZARD_SKILL2_DMG * (1 + player->level_ / 10.f);

	player->SetSkillOOBB(XMFLOAT3(-7.f, -200.f, 29.0f),
		XMFLOAT3(23.f * WIZARD_SKILL2_WIDTH, 16.f * WIZARD_SKILL2_DEPTH, 28.f),
		XMFLOAT4(0.f, 0.f, 0.f, 1.f));

	player->skillOOBBTransformed_.Transform(player->skillOOBB_, XMLoadFloat4x4(&(player->world_))); // world_
	XMStoreFloat4(&player->skillOOBBTransformed_.Orientation, XMQuaternionNormalize(XMLoadFloat4(&player->skillOOBBTransformed_.Orientation)));

	for (auto& npc : player->viewList_)
	{
		if (false == npc->isActive_) continue;
		if (ObjectType::TYPE_MONSTER != npc->objectType_) continue;
		if (false == player->skillOOBB_.Contains(npc->xmOOBB_))continue;

		dynamic_cast<TimerThread*>(threadManager_->FindThread(TIMER_THREAD))->AddTimer(npc, EVT_MONSTER_DAMAGED, GetTickCount(), player);
		dynamic_cast<TimerThread*>(threadManager_->FindThread(TIMER_THREAD))->AddTimer(npc, EVT_MONSTER_DAMAGED, GetTickCount() + 100, player);
		dynamic_cast<TimerThread*>(threadManager_->FindThread(TIMER_THREAD))->AddTimer(npc, EVT_MONSTER_DAMAGED, GetTickCount() + 200, player);
		dynamic_cast<TimerThread*>(threadManager_->FindThread(TIMER_THREAD))->AddTimer(npc, EVT_MONSTER_DAMAGED, GetTickCount() + 300, player);
		dynamic_cast<TimerThread*>(threadManager_->FindThread(TIMER_THREAD))->AddTimer(npc, EVT_MONSTER_DAMAGED, GetTickCount() + 400, player);
	}
}
void GameObjectManager::ProcessWizardAttack3(GameObject* player) {

	sc_packet_wizard_heal p;
	p.size = sizeof(sc_packet_wizard_heal);
	p.type = SC_WIZARD_HEAL;

	for (auto& targetPlayer : player->viewList_)
	{
		if (false == targetPlayer->isActive_) continue;
		if (ObjectType::TYPE_PLAYER != targetPlayer->objectType_) continue;

		for (auto& targetInViewPlayer : targetPlayer->viewList_)
		{
			if (false == targetInViewPlayer->isActive_) continue;
			if (ObjectType::TYPE_PLAYER != targetInViewPlayer->objectType_) continue;

			p.id = targetPlayer->ID_;
			SendManager::SendPacket(targetInViewPlayer, &p);
		}

		targetPlayer->hp_ += WIZARD_SKILL3_DMG * (1 + player->level_ / 10.f);
		if (targetPlayer->hp_ > INIT_PLAYER_HP)
			targetPlayer->hp_ = INIT_PLAYER_HP;
		SendManager::SendObjectHp(targetPlayer, targetPlayer);
	}
}

void GameObjectManager::PlayerDamaged(GameObject* player, GameObject* monster) {

	for (auto& playerObject : player->viewList_)
	{
		if (false == playerObject->isActive_) continue;
		if (ObjectType::TYPE_PLAYER != playerObject->objectType_) continue;
		SendManager::SendObjectDamage(playerObject, player, monster->dmg_);
	}

	if (player->state_ == STATE_DEAD)
		return;

	player->hp_ -= monster->dmg_;

	if (player->hp_ <= 0)
	{
		player->hp_ = 0;

		player->state_ = STATE_DEAD;

		for (auto& playerObject : player->viewList_)
		{
			if (false == playerObject->isActive_) continue;
			if (ObjectType::TYPE_PLAYER != playerObject->objectType_) continue;
			SendManager::SendObjectState(playerObject, player);
		}

		dynamic_cast<TimerThread*>(threadManager_->FindThread(TIMER_THREAD))->AddTimer(player, EVT_PLAYER_RESPAWN, GetTickCount() + 5000, nullptr);
	}
	else
	{
		player->state_ = STATE_HIT;

		for (auto& playerObject : player->viewList_)
		{
			if (false == playerObject->isActive_) continue;
			if (ObjectType::TYPE_PLAYER != playerObject->objectType_) continue;
			SendManager::SendObjectState(playerObject, player);
		}
	}

	SendManager::SendObjectHp(player, player);
}

void GameObjectManager::MonsterDamaged(GameObject* monster, GameObject* player) {

	if (monster->state_ == STATE_DEAD)
		return;

	for (int i = 0; i < NUM_OF_PLAYER; ++i)
	{
		if (false == playerArray_[i]->isActive_) continue;
		if (0 == playerArray_[i]->viewList_.count(monster)) continue;
		SendManager::SendObjectDamage(playerArray_[i], monster, player->dmg_);
	}

	monster->hp_ -= player->dmg_;

	if (monster->hp_ <= 0)
	{
		monster->hp_ = 0;

		monster->state_ = STATE_DEAD;
		monster->isActive_ = false;

		for (int i = 0; i < NUM_OF_PLAYER; ++i)
		{
			if (false == playerArray_[i]->isActive_) continue;
			if (0 == playerArray_[i]->viewList_.count(monster)) continue;
			SendManager::SendObjectState(playerArray_[i], monster);
		}

		dynamic_cast<TimerThread*>(threadManager_->FindThread(TIMER_THREAD))->AddTimer(monster, EVT_MONSTER_RESPAWN, GetTickCount() + 50000, nullptr);

		AddExp(player, monster);
	}
	else
	{
		monster->state_ = STATE_HIT;
		for (int i = 0; i < NUM_OF_PLAYER; ++i)
		{
			if (false == playerArray_[i]->isActive_) continue;
			if (0 == playerArray_[i]->viewList_.count(monster)) continue;
			SendManager::SendObjectState(playerArray_[i], monster);
		}
	}
}

void GameObjectManager::PlayerRespown(GameObject* player)
{
	if (false == player->isActive_)
		return;

	PutNewPlayer(player);
}

void GameObjectManager::MonsterRespown(GameObject* monster)
{
	auto npcType = dynamic_cast<NPC*>(monster)->npcType_;

	if (NPCType::NPC_TURTLE == npcType)
		monster->hp_ = MonsterInitHP::TURTLE_HP;
	else if (NPCType::NPC_SPIDER == npcType)
		monster->hp_ = MonsterInitHP::SPIDER_HP;
	else if (NPCType::NPC_NAGA_GUARD == npcType)
		monster->hp_ = MonsterInitHP::NAGAGUARD_HP;
	else if (NPCType::NPC_TREE_GUARD == npcType)
		monster->hp_ = MonsterInitHP::TREEGUARD_HP;
	else if (NPCType::NPC_ROCK_WARRIOR == npcType)
		monster->hp_ = MonsterInitHP::ROCKWARRIOR_HP;

	SearchNewTargetPlayer(monster);
}

void GameObjectManager::ProcessMove(GameObject* player, unsigned char dirType, unsigned char moveType)
{
	if ((player->state_ == State::STATE_IDLE ||
		player->state_ == State::STATE_WALK ||
		player->state_ == State::STATE_ROLL ||
		player->state_ == State::STATE_HIT) == false)
		return;

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
		if(PlayerType::PLAYER_WARRIOR == pPlayer->playerType_)
			moveSpeed = WARRIOR_ROLL_SPEED;
		else if (PlayerType::PLAYER_WIZARD == pPlayer->playerType_)
			moveSpeed = WIZARD_TELEPORT_SPEED;
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

	float ceta = acos(cosCeta); // 현재 각도

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

	if (SC_WALK_MOVE == moveType || PlayerType::PLAYER_WARRIOR == pPlayer->playerType_)
	{
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
	}

	for (auto& mapObject : mapObjectArray_)
	{
		if (false == mapObject->isActive_) continue;
		if (false == player->CanSeeMapObject(mapObject)) continue;

		IsMapObjectColl = CollisionUtil::ProcessMapObjectColl(mapObject, player, xmf3Shift);
		if (IsMapObjectColl)
			break;
	}

	if (!IsPlayerColl && ! IsNPCColl && !IsMapObjectColl)
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

	unordered_set <GameObject*> new_view_list; // 새로운 뷰리스트 생성
	for (int i = 0; i < NUM_OF_PLAYER; ++i) {
		if (false == playerArray_[i]->isActive_) continue;
		if (false == playerArray_[i]->CanSee(player)) continue;
		// 새로운 뷰리스트 (나를 기준) // 새로 이동하고 보이는 모든 애들
		new_view_list.insert(playerArray_[i]);
	}
	for (int i = 0; i < NUM_OF_NPC_TOTAL; ++i) {
		if (false == npcArray_[i]->CanSee(player)) continue;
		new_view_list.insert(npcArray_[i]);
		WakeUpNPC(npcArray_[i], player);
	}

	for (auto& object : new_view_list) {

		pPlayer->vlm_.lock();
		// 나의 기존 뷰리스트에는 없었다 // 즉 새로 들어왔다
		if (0 == pPlayer->viewList_.count(object))
			pPlayer->viewList_.insert(object);

		pPlayer->vlm_.unlock();

		if (TYPE_MONSTER == object->objectType_)
			SendManager::SendPutMonster(player, object);
		else
		{
			if(player != object)
				SendManager::SendPutPlayer(player, object);
		}

		if (TYPE_MONSTER == object->objectType_) continue;

		auto objectPlayer = dynamic_cast<Player*>(object);

		objectPlayer->vlm_.lock();
		// 상대방한테 내가 없었다? // 추가
		if (0 == objectPlayer->viewList_.count(player)) {
			objectPlayer->viewList_.insert(player);
			objectPlayer->vlm_.unlock();
			SendManager::SendPutPlayer(object, player);
		}
		// 상대방한테 내가 있었다? // 위치값만
		else
		{
			objectPlayer->vlm_.unlock();
			SendManager::SendPacket(object, &sp_pos);
			if (IsRotated)
				SendManager::SendPacket(object, &sp_rotate);
		}
	}
	// 나의 이전 뷰리스트에 있는 애들
	pPlayer->vlm_.lock();
	unordered_set <GameObject*> old_v = pPlayer->viewList_;
	pPlayer->vlm_.unlock();
	for (auto& object : old_v) {
		if (0 == new_view_list.count(object)) {
			if (player == object) continue;
			pPlayer->vlm_.lock();
			pPlayer->viewList_.erase(object); // 계속 락 언락하지 말고 따로 리무브리스트만들고 한번에 지우는게 좋음
			pPlayer->vlm_.unlock();
			SendManager::SendRemoveObject(player, object);

			if (TYPE_PLAYER != object->objectType_) continue;

			auto objectPlayer = dynamic_cast<Player*>(object);

			objectPlayer->vlm_.lock();
			// 내 시야에 없는 애들이니까 걔네 시야에도 내가 있으면 지우게 하자.
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
		if (PlayerType::PLAYER_WARRIOR == pPlayer->playerType_)
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

		for (auto& playerObject : player->viewList_)
		{
			if (false == playerObject->isActive_) continue;
			if (ObjectType::TYPE_PLAYER != playerObject->objectType_) continue;
			if (player == playerObject) continue;
			SendManager::SendPacket(playerObject, &sp);
		}


		return;
	}
	else if ((packet[1] == CS_ATTACK1) || (packet[1] == CS_ATTACK2) || (packet[1] == CS_ATTACK3))
	{
		auto pPlayer = dynamic_cast<Player*>(player);

		if (packet[1] == CS_ATTACK1)
		{
			player->state_ = STATE_ATTACK1;
			if(PlayerType::PLAYER_WARRIOR == pPlayer->playerType_)
				dynamic_cast<TimerThread*>(threadManager_->FindThread(TIMER_THREAD))->AddTimer(player, EVT_WARRIOR_ATTACK1, GetTickCount() + WARRIOR_SKILL1_DELAY, nullptr);
			else
				dynamic_cast<TimerThread*>(threadManager_->FindThread(TIMER_THREAD))->AddTimer(player, EVT_WIZARD_ATTACK1, GetTickCount() + WIZARD_SKILL1_DELAY, nullptr);
		}
		else if (packet[1] == CS_ATTACK2)
		{
			player->state_ = STATE_ATTACK2;
			if (PlayerType::PLAYER_WARRIOR == pPlayer->playerType_)
				dynamic_cast<TimerThread*>(threadManager_->FindThread(TIMER_THREAD))->AddTimer(player, EVT_WARRIOR_ATTACK2, GetTickCount() + WARRIOR_SKILL2_DELAY, nullptr);
			else
				dynamic_cast<TimerThread*>(threadManager_->FindThread(TIMER_THREAD))->AddTimer(player, EVT_WIZARD_ATTACK2, GetTickCount() + WIZARD_SKILL2_DELAY, nullptr);
		}
		else if (packet[1] == CS_ATTACK3)
		{
			player->state_ = STATE_ATTACK3;
			if (PlayerType::PLAYER_WARRIOR == pPlayer->playerType_)
				dynamic_cast<TimerThread*>(threadManager_->FindThread(TIMER_THREAD))->AddTimer(player, EVT_WARRIOR_ATTACK3, GetTickCount() + WARRIOR_SKILL3_DELAY, nullptr);
			else
				dynamic_cast<TimerThread*>(threadManager_->FindThread(TIMER_THREAD))->AddTimer(player, EVT_WIZARD_ATTACK3, GetTickCount(), nullptr);
		}

		for (auto& playerObject : player->viewList_)
		{
			if (false == playerObject->isActive_) continue;
			if (ObjectType::TYPE_PLAYER != playerObject->objectType_) continue;
			SendManager::SendObjectState(playerObject, player);
		}

		return;
	}
	else if (packet[1] == CS_MAP_INIT_DATA)
	{
		static int first_client = player->ID_;
		static int num = 0;

		if (first_client != player->ID_)
			return;

		if (mapObjectNum_ <= num)
			return;
		cs_packet_mapinitdata *p = reinterpret_cast<cs_packet_mapinitdata *>(packet);

		mapObjectArray_[num]->isActive_ = true;
		mapObjectArray_[num]->ID_ = num;
		mapObjectArray_[num]->world_ = p->world;
		dynamic_cast<MapObject*>(mapObjectArray_[num])->Bounds = p->bounds;

		++num;

		return;
	}
	else if (packet[1] == CS_PLAYER_TYPE)
	{
		cs_packet_player_type *p = reinterpret_cast<cs_packet_player_type *>(packet);

		dynamic_cast<Player*>(player)->playerType_ = p->playerType;

		PutNewPlayer(player);

		return;
	}
	else if (packet[1] == CS_ULTIMATE_START)
	{
		player->state_ = STATE_ULTIMATE;
		sc_packet_state sp;
		sp.id = player->ID_;
		sp.size = sizeof(sc_packet_state);
		sp.type = SC_STATE;
		sp.state = player->state_;

		for (auto& playerObject : player->viewList_)
		{
			if (false == playerObject->isActive_) continue;
			if (ObjectType::TYPE_PLAYER != playerObject->objectType_) continue;
			if (player == playerObject) continue;
			SendManager::SendPacket(playerObject, &sp);
		}

		return;
	}
	else if (packet[1] == CS_ULTIMATE_ON)
	{
		auto pPlayer = dynamic_cast<Player*>(player);

		player->state_ = State::STATE_IDLE;

		sc_packet_ultimate_on su;
		su.size = sizeof(sc_packet_ultimate_on);

		if (PlayerType::PLAYER_WARRIOR == pPlayer->playerType_)
		{
			pPlayer->isWarriorUltimateMode = true;

			su.id = player->ID_;
			su.type = SC_ULTIMATE_WARRIOR;

			for (auto& playerObject : player->viewList_)
			{
				if (false == playerObject->isActive_) continue;
				if (ObjectType::TYPE_PLAYER != playerObject->objectType_) continue;
				if (player == playerObject) continue;
				SendManager::SendPacket(playerObject, &su);
			}
		}
		else if(PlayerType::PLAYER_WIZARD == pPlayer->playerType_)
		{
			su.type = SC_ULTIMATE_WIZARD;
			player->dmg_ = MAGE_ULTIMATE_DMG;

			for (auto& targetNPC : player->viewList_)
			{
				if (false == targetNPC->isActive_) continue;
				if (ObjectType::TYPE_MONSTER != targetNPC->objectType_) continue;

				for (int i = 0; i < NUM_OF_PLAYER; ++i)
				{
					if (false == playerArray_[i]->isActive_) continue;
					if (0 == playerArray_[i]->viewList_.count(targetNPC)) continue;

					su.id = targetNPC->ID_ + NPC_ID_START;
					SendManager::SendPacket(playerArray_[i], &su);
				}
				MonsterDamaged(targetNPC, player);
			}
		}


		return;
	}
	else if (packet[1] == CS_ULTIMATE_OFF)
	{
		dynamic_cast<Player*>(player)->isWarriorUltimateMode = false;
		sc_packet_ultimate_off su;
		su.id = player->ID_;
		su.size = sizeof(sc_packet_ultimate_off);
		su.type = SC_ULTIMATE_OFF;

		for (auto& playerObject : player->viewList_)
		{
			if (false == playerObject->isActive_) continue;
			if(ObjectType::TYPE_PLAYER != playerObject->objectType_) continue;
			if (player == playerObject) continue;

			SendManager::SendPacket(playerObject, &su);
		}
		return;
	}
	else if (packet[1] == CS_MAPOBJECT_NUM)
	{
		cs_packet_mapobject_num* p = reinterpret_cast<cs_packet_mapobject_num *>(packet);
		mapObjectNum_ = p->mapObjectNum;
		return;
	}
	else
	{
		//cout << cl << " ProcessPacket Error" << endl;
		return;
	}
}

void GameObjectManager::SearchNewTargetPlayer(GameObject * monster)
{
	for (int i = 0; i < NUM_OF_PLAYER; ++i)
	{
		if (false == playerArray_[i]->isActive_) continue;
		if (0 == playerArray_[i]->viewList_.count(monster)) continue;

		SendManager::SendObjectState(playerArray_[i], monster);

		if (State::STATE_DEAD == playerArray_[i]->state_) continue;
		if (false == playerArray_[i]->IsInAgroRange(monster)) continue;
		
		monster->isActive_ = true;
		monster->state_ = STATE_WALK;
		//cout << "SearchNewTargetPlayer에서 호출 npc id : " << monster->ID_ << endl;
		dynamic_cast<TimerThread*>(threadManager_->FindThread(TIMER_THREAD))->AddTimer(monster, EVT_CHASE, GetTickCount() + 50, playerArray_[i]);
		return;
		
	}
	monster->state_ = STATE_IDLE;
	monster->isActive_ = false;
}

void GameObjectManager::AddExp(GameObject * player, GameObject* monster)
{
	player->exp_ += monster->exp_;

	unsigned int playerMaxExp = 100 + 20 * (player->level_ - 1);

	if (player->exp_ >= playerMaxExp)
	{
		player->exp_ -= playerMaxExp;
		++player->level_;
		player->hp_ = INIT_PLAYER_HP;

		for (auto& playerObject : player->viewList_)
		{
			if (false == playerObject->isActive_) continue;
			if(ObjectType::TYPE_PLAYER != playerObject->objectType_) continue;
				SendManager::SendObjectLevelup(playerObject, player);
		}
	}
	SendManager::SendObjectExp(player);
}
