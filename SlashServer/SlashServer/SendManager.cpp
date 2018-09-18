#include "SendManager.h"

void SendManager::SendPacket(GameObject* player, void *packet) {
	

	EXOver *o = new EXOver;
	char *p = reinterpret_cast<char *>(packet);
	memcpy(o->ioBuf, packet, p[0]);
	o->eventType = EVT_SEND;
	o->wsaBuf.buf = o->ioBuf;
	o->wsaBuf.len = p[0];
	ZeroMemory(&o->wsaOver, sizeof(WSAOVERLAPPED));

	int ret = WSASend(dynamic_cast<Player*>(player)->s_, &o->wsaBuf, 1, NULL, 0, &o->wsaOver, NULL);
	if (0 != ret) {
		//int err_no = WSAGetLastError();
		//if (WSA_IO_PENDING != err_no) // WSA_IO_PENDING 뭐냐? 샌드가 계속 실행되고 있다. IOCP는 WSA_IO_PENDING 에러를 리턴한다.
		//	error_display("Error in SendPacket:", err_no);
	}


	//printf("SendPacket to Client [ %d ] Type [ %d ] Size [ %d ]\n", cl, (int)p[1], (int)p[0]);
}

void SendManager::SendObjectWalk(GameObject* player, GameObject* object)
{

	sc_packet_move move_p;

	if (TYPE_MONSTER == object->objectType_)
		move_p.id = object->ID_ + NPC_ID_START;
	else
		move_p.id = object->ID_;

	move_p.size = sizeof(sc_packet_move);
	move_p.type = SC_WALK_MOVE;
	move_p.posX = object->world_._41;
	move_p.posY = object->world_._42;
	move_p.posZ = object->world_._43;

	SendPacket(player, &move_p);
}

void SendManager::SendObjectRoll(GameObject * player, GameObject * object)
{
	sc_packet_move roll_p;

	if (TYPE_MONSTER == object->objectType_)
		roll_p.id = object->ID_ + NPC_ID_START;
	else
		roll_p.id = object->ID_;

	roll_p.size = sizeof(sc_packet_move);
	roll_p.type = SC_ROLL_MOVE;
	roll_p.posX = object->world_._41;
	roll_p.posY = object->world_._42;
	roll_p.posZ = object->world_._43;

	SendPacket(player, &roll_p);
}

void SendManager::SendObjectLook(GameObject* player, GameObject* object)
{
	sc_packet_look_degree degree_p;

	if (TYPE_MONSTER == object->objectType_)
		degree_p.id = object->ID_ + NPC_ID_START;
	else
		degree_p.id = object->ID_;

	degree_p.size = sizeof(sc_packet_look_degree);
	degree_p.type = SC_ROTATE;
	degree_p.lookDegree = object->lookDegree_;

	SendPacket(player, &degree_p);
}

void SendManager::SendObjectState(GameObject* player, GameObject* object)
{
	sc_packet_state state_p;

	if (TYPE_MONSTER == object->objectType_)
		state_p.id = object->ID_ + NPC_ID_START;
	else
		state_p.id = object->ID_;

	state_p.size = sizeof(sc_packet_state);
	state_p.type = SC_STATE;
	state_p.state = object->state_;

	SendPacket(player, &state_p);
}

void SendManager::SendPutPlayer(GameObject* player, GameObject* object)
{
	sc_packet_put_player put_p;

	if (TYPE_MONSTER == object->objectType_)
		put_p.id = object->ID_ + NPC_ID_START;
	else
		put_p.id = object->ID_;

	put_p.size = sizeof(sc_packet_put_player);
	put_p.type = SC_PUT_PLAYER;
	put_p.posX = object->world_._41;
	put_p.posY = object->world_._42;
	put_p.posZ = object->world_._43;
	put_p.lookDegree = object->lookDegree_;
	put_p.state = object->state_;
	put_p.playerType = dynamic_cast<Player*>(object)->playerType_;

	SendPacket(player, &put_p);
}

void SendManager::SendPutMonster(GameObject* player, GameObject* object)
{
	sc_packet_put_monster put_p;

	if (TYPE_MONSTER == object->objectType_)
		put_p.id = object->ID_ + NPC_ID_START;
	else
		put_p.id = object->ID_;

	put_p.size = sizeof(sc_packet_put_monster);
	put_p.type = SC_PUT_MONSTER;
	put_p.posX = object->world_._41;
	put_p.posY = object->world_._42;
	put_p.posZ = object->world_._43;
	put_p.lookDegree = object->lookDegree_;
	put_p.state = object->state_;

	SendPacket(player, &put_p);
}

void SendManager::SendRemoveObject(GameObject* player, GameObject* object)
{
	sc_packet_remove_object p;

	if (TYPE_MONSTER == object->objectType_)
		p.id = object->ID_ + NPC_ID_START;
	else
		p.id = object->ID_;

	p.size = sizeof(sc_packet_remove_object);
	p.type = SC_REMOVE_OBJECT;

	SendPacket(player, &p);
}

void SendManager::SendObjectHp(GameObject* player, GameObject* object)
{
	sc_packet_hp p;

	if (TYPE_MONSTER == object->objectType_)
		p.id = object->ID_ + NPC_ID_START;
	else
		p.id = object->ID_;

	p.size = sizeof(sc_packet_hp);
	p.type = SC_HP;
	p.hp = object->hp_;

	SendPacket(player, &p);
}

void SendManager::SendObjectDamage(GameObject* player, GameObject* damagedObj, int damage)
{
	sc_packet_damage p;

	if (TYPE_MONSTER == damagedObj->objectType_)
		p.id = damagedObj->ID_ + NPC_ID_START;
	else
		p.id = damagedObj->ID_;

	p.size = sizeof(sc_packet_damage);
	p.type = SC_DAMAGE;
	p.dmg = damage;

	SendPacket(player, &p);
}

void SendManager::SendObjectLevelup(GameObject* player, GameObject* Obj)
{
	sc_packet_level_up p;

	if (TYPE_MONSTER == Obj->objectType_)
		p.id = Obj->ID_ + NPC_ID_START;
	else
		p.id = Obj->ID_;

	p.size = sizeof(sc_packet_level_up);
	p.type = SC_LEVEL_UP;
	p.level = Obj->level_;

	SendPacket(player, &p);
}

void SendManager::SendObjectExp(GameObject* player)
{
	sc_packet_exp p;

	if (TYPE_MONSTER == player->objectType_)
		p.id = player->ID_ + NPC_ID_START;
	else
		p.id = player->ID_;

	p.size = sizeof(sc_packet_exp);
	p.type = SC_EXP;
	p.exp = player->exp_;

	SendPacket(player, &p);
}


void SendManager::SendPlayEnding(GameObject* player)
{
	sc_packet_play_ending p;

	if (TYPE_MONSTER == player->objectType_)
		p.id = player->ID_ + NPC_ID_START;
	else
		p.id = player->ID_;

	p.size = sizeof(sc_packet_play_ending);
	p.type = SC_PLAY_ENDING;

	SendPacket(player, &p);
}
