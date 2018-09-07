#include "GameObjectManager.h"
#include "SendManager.h"
#include "MathUtil.h"

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
	for (int i = 0; i < NUM_OF_NPC; ++i)
	{
		npcArray_[i] = new NPC;
		npcArray_[i]->ID_ = i;
		npcArray_[i]->Initialize();
	}
	for (auto& d : mapObjectArray_)
	{
		d = new MapObject;
		d->Initialize();
	}
}

void GameObjectManager::PutNewPlayer(int newKey)
{
	auto player = dynamic_cast<Player*>(playerArray_[newKey]);

	player->hp_ = 200;
	player->world_._41 = 15;
	player->world_._42 = 0;
	player->world_._43 = 0;

	player->vlm_.lock();
	player->viewList_.clear();
	player->vlm_.unlock();

	sc_packet_put_player p;
	p.id = player->ID_;
	p.size = sizeof(sc_packet_put_player);
	p.type = SC_PUT_PLAYER;
	p.posX = player->world_._41;
	p.posY = player->world_._42;
	p.posZ = player->world_._43;
	p.lookDegree = player->lookDegree_;
	p.state = STATE_IDLE;

	SendManager::SendPacket(playerArray_[newKey], &p);

	// 나의 접속을 모든 플레이어에게 알림
	for (int i = 0; i < NUM_OF_PLAYER; ++i)
	{
		if (false == playerArray_[i]->isActive_) continue;
		if (false == playerArray_[i]->CanSee(playerArray_[newKey])) continue;
		if (i == newKey) continue;

		SendManager::SendPacket(playerArray_[i], &p);

		dynamic_cast<Player*>(playerArray_[i])->vlm_.lock();
		dynamic_cast<Player*>(playerArray_[i])->viewList_.insert(playerArray_[newKey]);
		dynamic_cast<Player*>(playerArray_[i])->vlm_.unlock();
	}
	// 접속중인 다른 플레이어 정보를 전송
	for (int i = 0; i < NUM_OF_PLAYER; ++i) {
		if (false == playerArray_[i]->isActive_) continue;
		if (false == playerArray_[i]->CanSee(playerArray_[newKey])) continue;
		if (i == newKey) continue;

		p.id = i;
		p.posX = playerArray_[i]->world_._41;
		p.posY = playerArray_[i]->world_._42;
		p.posZ = playerArray_[i]->world_._43;
		p.lookDegree = playerArray_[i]->lookDegree_;
		p.state = playerArray_[i]->state_;

		SendManager::SendPacket(playerArray_[newKey], &p);

		player->vlm_.lock();
		player->viewList_.insert(playerArray_[i]);
		player->vlm_.unlock();
	}
	for (int i = 0; i < NUM_OF_NPC; i++)
	{
		if (false == player->CanSee(npcArray_[i])) continue; // 이거 이전에 활성화되어 있는가도 확인해야함
		if (player->IsInAgroRange(npcArray_[i]))
			WakeUpNPC(npcArray_[i], playerArray_[newKey]); // 상태 2번 보냄 수정해야함

		SendManager::SendPutMonster(playerArray_[newKey], npcArray_[i]);

		player->vlm_.lock();
		player->viewList_.insert(npcArray_[i]);
		player->vlm_.unlock();

		//cout << "접속 : " << i << " 번째 몬스터 보임" << endl;
	}
}

void GameObjectManager::DisconnectPlayer(GameObject* player) {

	auto pPlayer = dynamic_cast<Player*>(player);

	closesocket(pPlayer->s_); // closesocket 먼저 해야함 // 멀티 쓰레드라서 
	pPlayer->isActive_ = false;
	printf("%d번 플레이어 접속종료.\n", player->ID_);

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
	
	player->isActive_ = false;
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

	XMFLOAT3 MonsterLook = XMFLOAT3(npc->world_._31, npc->world_._32, npc->world_._33);
	XMFLOAT3 MonsterPos = XMFLOAT3(npc->world_._41, npc->world_._42, npc->world_._43);

	XMFLOAT3 playerPos = XMFLOAT3(player->world_._41, player->world_._42, player->world_._43);
	XMFLOAT3 Shift = Vector3::Normalize(MonsterLook);

	XMFLOAT3 dirVector = Vector3::Subtract(playerPos, MonsterPos);   // 객체에서 플레이어로 가는 벡터

	dirVector = Vector3::Normalize(dirVector);

	XMFLOAT3 crossVector = Vector3::CrossProduct(Shift, dirVector, true);

	float dotproduct = Vector3::DotProduct(Shift, dirVector);
	float ShifttLength = Vector3::Length(Shift);
	float dirVectorLength = Vector3::Length(dirVector);

	float cosCeta = (dotproduct / ShifttLength * dirVectorLength);

	float ceta = acos(cosCeta);

	if (playerPos.z < MonsterPos.z)
		ceta = 360.f - ceta * 57.3248f;// +180.0f;
	else
		ceta = ceta * 57.3248f;

	XMFLOAT3 MovingReflectVector = XMFLOAT3(0, 0, 0); // 나중에 충돌하면 갱신해줘야함 이동하기전에

	bool IsRotated = false;

	if (ceta > 0.8f && ceta < 359.f)
	{
		if (crossVector.y > 0)
		{
			XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(0.f), XMConvertToRadians(MONSTER_ROTATE_DEGREE), XMConvertToRadians(0.f));
			npc->world_ = Matrix4x4::Multiply(mtxRotate, npc->world_);
			npc->lookDegree_ += MONSTER_ROTATE_DEGREE;
		}
		else if (crossVector.y < 0)
		{
			XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(0.f), XMConvertToRadians(-MONSTER_ROTATE_DEGREE), XMConvertToRadians(0.f));
			npc->world_ = Matrix4x4::Multiply(mtxRotate, npc->world_);
			npc->lookDegree_ += (360.f - MONSTER_ROTATE_DEGREE);
		}

		IsRotated = true;
	}

	if (npc->lookDegree_ >= 360.f)
		npc->lookDegree_ -= 360.f;

	XMFLOAT3 movingVector = XMFLOAT3(dirVector.x * MONSTER_MOVE_SPEED, dirVector.y * MONSTER_MOVE_SPEED, dirVector.z * MONSTER_MOVE_SPEED); 

	unordered_set <GameObject*> old_vl;
	for (int id = 0; id < NUM_OF_PLAYER; ++id)
		if (true == playerArray_[id]->isActive_)
			if (true == playerArray_[id]->CanSee(npc)) {
				old_vl.insert(playerArray_[id]);
			}


	for (int i = 0; i < NUM_OF_NPC; ++i)
	{
		if (false == npcArray_[i]->isActive_) continue;
		if (false == npcArray_[i]->IsClose(npc)) continue;

		if (npc->xmOOBB_.Contains(npcArray_[i]->xmOOBB_))
		{
			MovingReflectVector = MathUtil::GetSlideVector(npc, npcArray_[i]);
			break;
		}
	}

	for (auto d : mapObjectArray_)
	{
		if (false == npc->IsClose(d)) continue;

		if (npc->xmOOBB_.Contains(d->xmOOBB_))
		{
			MovingReflectVector = MathUtil::GetMapSlideVector(npc, d);
			break;
		}
	}

	auto result = Vector3::MultiplyScalr(MovingReflectVector, Vector3::DotProduct(movingVector, MovingReflectVector)); 
	movingVector = Vector3::Subtract(movingVector, result);

	XMFLOAT3 xmf3Position = Vector3::Add(MonsterPos, movingVector);
	npc->world_._41 = xmf3Position.x;
	npc->world_._42 = xmf3Position.y;
	npc->world_._43 = xmf3Position.z;


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
			SendManager::SendPutObject(object, npc);
		}
		else
		{
			newPlayer->vlm_.unlock();
			SendManager::SendObjectPos(object, npc);
			if (IsRotated)
				SendManager::SendObjectLook(object, npc);
		}
	}
	// RemoveObject
	for (auto object : old_vl)
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
	else if (player->IsClose(npc))	 //충돌할 정도로 가까워 졌으면
	{
		if (STATE_ATTACK1 == npc->state_) return;

		dynamic_cast<TimerThread*>(threadManager_->FindThread(TIMER_THREAD))->AddTimer(npc, EVT_MONSTER_ATTACK, GetTickCount(), player);

		// 몬스터 때리는 상태로 변경하고 클라로 보내기 (그냥 클라에서 계속 계산하다가 판단할까?)
		// 그리고 몇초 뒤 플레이어 체력 깎는다.
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

	for (int i = 0; i < NUM_OF_NPC; ++i) // 뷰리스트에 있는 애들 쓰는게 더 나으려나..
	{
		if (false == npcArray_[i]->isActive_) continue;
		if (false == npcArray_[i]->IsAttackRange(player)) continue;

		// 몬스터가 앞에 있는가 여기에서 판단

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

	PutNewPlayer(player->ID_);
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

void GameObjectManager::ProcessPacket(GameObject* player, char *packet)
{
	if (packet[1] <= MOVE_PACKET_END && packet[1] >= MOVE_PACKET_START)
	{
		// LEFT RIGHT 이동 시 룩벡터 3열 부호 다름
		// UP DOWN 이동 시 라이트벡터 3열 부호 다름

		player->state_ = STATE_WALK;

		XMFLOAT3 xmf3Shift{ 0.0f, 0.0f, 0.0f };

		cs_packet_dir *p = reinterpret_cast<cs_packet_dir *>(packet);

		if (p->type & CS_DIR_FORWARD) xmf3Shift = Vector3::Add(xmf3Shift, const_cast<XMFLOAT3&>(DIR_VECTOR_Z), PLAYER_MOVE_SPEED);
		if (p->type & CS_DIR_BACKWARD) xmf3Shift = Vector3::Add(xmf3Shift, const_cast<XMFLOAT3&>(DIR_VECTOR_Z), -PLAYER_MOVE_SPEED);
		if (p->type & CS_DIR_RIGHT) xmf3Shift = Vector3::Add(xmf3Shift, const_cast<XMFLOAT3&>(DIR_VECTOR_X), PLAYER_MOVE_SPEED);
		if (p->type & CS_DIR_LEFT) xmf3Shift = Vector3::Add(xmf3Shift, const_cast<XMFLOAT3&>(DIR_VECTOR_X), -PLAYER_MOVE_SPEED);

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
			if (crossVector.y > 0)
			{
				XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(0.f), XMConvertToRadians(0.f), XMConvertToRadians(-PLAYER_ROTATE_DEGREE));
				player->world_ = Matrix4x4::Multiply(mtxRotate, player->world_);
				player->lookDegree_ += (360 - PLAYER_ROTATE_DEGREE);
			}
			else
			{
				XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(0.f), XMConvertToRadians(0.f), XMConvertToRadians(PLAYER_ROTATE_DEGREE));
				player->world_ = Matrix4x4::Multiply(mtxRotate, player->world_);
				player->lookDegree_ += PLAYER_ROTATE_DEGREE;
			}
			if (player->lookDegree_ >= 360.f)
				player->lookDegree_ -= 360.f;
			IsRotated = true;
		}

		//cout << player.World._11 << " " << player.World._12 << " " << player.World._13 << " " << player.World._14 << " " << endl;
		//cout << player.World._21 << " " << player.World._22 << " " << player.World._23 << " " << player.World._24 << " " << endl;
		//cout << player.World._31 << " " << player.World._32 << " " << player.World._33 << " " << player.World._34 << " " << endl;
		//cout << player.World._41 << " " << player.World._42 << " " << player.World._43 << " " << player.World._44 << " " << endl << endl;


		player->xmOOBBTransformed_.Transform(player->xmOOBB_, XMLoadFloat4x4(&(player->world_)));
		XMStoreFloat4(&player->xmOOBBTransformed_.Orientation, XMQuaternionNormalize(XMLoadFloat4(&player->xmOOBBTransformed_.Orientation)));

		bool IsCollision = false;

		//cout << "Player Pos << " << player.World._41 << "\t" << player.World._42 << "\t" << player.World._43 << endl;


		// 플레이어 이동시 충돌체크

		for (auto& d : mapObjectArray_)
		{
			if (false == player->CanSeeMapObject(d)) continue;
			if (false == d->isActive_) continue;

			XMMATRIX world = XMLoadFloat4x4(&d->world_);
			XMMATRIX invWorld = XMMatrixInverse(&XMMatrixDeterminant(world), world);
			BoundingOrientedBox mLocalPlayerBounds;
			player->xmOOBB_.Transform(mLocalPlayerBounds, invWorld);

			if (mLocalPlayerBounds.Contains(dynamic_cast<MapObject*>(d)->Bounds) != DirectX::DISJOINT) // 여기 문제
			{

				SlideVector = MathUtil::GetMapSlideVector(player, d);
				float cosCeta = Vector3::DotProduct(SlideVector, xmf3Shift);
				if (cosCeta < 0)
				{
					auto result = Vector3::MultiplyScalr(SlideVector, Vector3::DotProduct(xmf3Shift, SlideVector));
					XMFLOAT3 sub_xmf3Shift = Vector3::Subtract(xmf3Shift, result);

					player->world_._41 += sub_xmf3Shift.x, player->world_._42 += sub_xmf3Shift.y, player->world_._43 += sub_xmf3Shift.z;
					IsCollision = true;
					break;
				}
			}
		}

		for (int i = 0; i < NUM_OF_PLAYER; ++i)
		{
			if (false == playerArray_[i]->isActive_) continue;
			if (false == playerArray_[i]->IsClose(player)) continue;
			if (i == player->ID_) continue;

			if (player->xmOOBB_.Contains(playerArray_[i]->xmOOBB_))
			{
				//cout << i << "번째 객체와 충돌 Type : " << int(g_clients[i].Type) << endl;
				SlideVector = MathUtil::GetSlideVector(player, playerArray_[i]);
				float cosCeta = Vector3::DotProduct(SlideVector, xmf3Shift);
				if (cosCeta < 0)
				{
					auto result = Vector3::MultiplyScalr(SlideVector, Vector3::DotProduct(xmf3Shift, SlideVector));
					XMFLOAT3 sub_xmf3Shift = Vector3::Subtract(xmf3Shift, result);

					player->world_._41 += sub_xmf3Shift.x, player->world_._42 += sub_xmf3Shift.y, player->world_._43 += sub_xmf3Shift.z;
					IsCollision = true;
					break; // 먼저 충돌한 객체만 처리
				}
			}
		}

		for (int i = 0; i < NUM_OF_NPC; ++i)
		{
			if (false == npcArray_[i]->isActive_) continue;
			if (false == npcArray_[i]->IsClose(player)) continue;
			if (i == player->ID_) continue;

			if (player->xmOOBB_.Contains(npcArray_[i]->xmOOBB_))
			{
				//cout << i << "번째 객체와 충돌 Type : " << int(g_clients[i].Type) << endl;
				SlideVector = MathUtil::GetSlideVector(player, npcArray_[i]);
				float cosCeta = Vector3::DotProduct(SlideVector, xmf3Shift);
				if (cosCeta < 0)
				{
					auto result = Vector3::MultiplyScalr(SlideVector, Vector3::DotProduct(xmf3Shift, SlideVector));
					XMFLOAT3 sub_xmf3Shift = Vector3::Subtract(xmf3Shift, result);

					player->world_._41 += sub_xmf3Shift.x, player->world_._42 += sub_xmf3Shift.y, player->world_._43 += sub_xmf3Shift.z;
					IsCollision = true;
					break; // 먼저 충돌한 객체만 처리
				}
			}
		}

		if (!IsCollision)
			player->world_._41 += xmf3Shift.x, player->world_._42 += xmf3Shift.y, player->world_._43 += xmf3Shift.z;

		//cout << " Pos = X : " << player.World._41 << " Y : " << player.World._42 << " Z : " << player.World._43 << endl;

		sc_packet_pos sp_pos;
		sp_pos.id = player->ID_;
		sp_pos.size = sizeof(sc_packet_pos);
		sp_pos.type = SC_POS;
		sp_pos.posX = player->world_._41;
		sp_pos.posY = player->world_._42;
		sp_pos.posZ = player->world_._43;

		sc_packet_look_degree sp_rotate;
		sp_rotate.id = player->ID_;
		sp_rotate.size = sizeof(sc_packet_look_degree);
		sp_rotate.type = SC_ROTATE;
		sp_rotate.lookDegree = player->lookDegree_;

		//////////////

		unordered_set <GameObject*> new_view_list; // 새로운 뷰리스트 생성
		for (int i = 0; i < NUM_OF_PLAYER; ++i) {
			if (i == player->ID_) continue;
			if (false == playerArray_[i]->isActive_) continue;
			if (false == playerArray_[i]->CanSee(player)) continue;
			// 새로운 뷰리스트 (나를 기준) // 새로 이동하고 보이는 모든 애들
			new_view_list.insert(playerArray_[i]);
		}
		for (int i = 0; i < NUM_OF_NPC; ++i) {
			if (false == npcArray_[i]->CanSee(player)) continue;
			new_view_list.insert(npcArray_[i]);
		}

		SendManager::SendPacket(player, &sp_pos);
		if (IsRotated)
			SendManager::SendPacket(player, &sp_rotate);

		auto pPlayer = dynamic_cast<Player*>(player);

		for (auto& object : new_view_list) {

			if(TYPE_MONSTER == object->objectType_)
				WakeUpNPC(object, player);

			pPlayer->vlm_.lock();
			// 나의 기존 뷰리스트에는 없었다 // 즉 새로 들어왔다
			if (0 == pPlayer->viewList_.count(object))
			{
				pPlayer->viewList_.insert(object);
				pPlayer->vlm_.unlock();

				if(TYPE_MONSTER == object->objectType_)
					SendManager::SendPutMonster(player, object);
				else
					SendManager::SendPutObject(player, object);

			}
			else // 수정해야함
			{
				if (TYPE_MONSTER == object->objectType_)
					SendManager::SendPutMonster(player, object);
				else
					SendManager::SendPutObject(player, object);

				pPlayer->vlm_.unlock();
			}

			if (TYPE_MONSTER == object->objectType_) continue;

			auto objectPlayer = dynamic_cast<Player*>(object);

			objectPlayer->vlm_.lock();
			// 상대방한테 내가 없었다? // 추가
			if (0 == objectPlayer->viewList_.count(player)) {
				objectPlayer->viewList_.insert(player);
				objectPlayer->vlm_.unlock();
				SendManager::SendPutObject(player, object);
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
				if (player->ID_ == object->ID_) continue;
				pPlayer->vlm_.lock();
				pPlayer->viewList_.erase(object); // 계속 락 언락하지 말고 따로 리무브리스트만들고 한번에 지우는게 좋음
				pPlayer->vlm_.unlock();
				SendManager::SendRemoveObject(player, object);

				if (TYPE_PLAYER != object->objectType_) continue;

				auto objectPlayer = dynamic_cast<Player*>(object);

				objectPlayer->vlm_.lock();
				// 있을 경우에만 지우게 하자.
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

		/////////////
	}
	else if (packet[1] == CS_STOP)
	{
		//cout << cl << player.LookVector.x << " " << player.LookVector.y << " " << player.LookVector.z << endl;

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
	else
	{
		//cout << cl << " ProcessPacket Error" << endl;
		return;
	}
}