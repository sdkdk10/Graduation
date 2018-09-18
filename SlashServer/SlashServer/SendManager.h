#pragma once
#include "GameObjectManager.h"

class SendManager
{
public:
	explicit SendManager() {};
	virtual ~SendManager() {};

public:

	static void SendPacket(GameObject* player, void *packet);
	static void SendObjectWalk(GameObject* player, GameObject* object);
	static void SendObjectRoll(GameObject* player, GameObject* object);
	static void SendObjectLook(GameObject* player, GameObject* object);
	static void SendObjectState(GameObject* player, GameObject* object);
	static void SendRemoveObject(GameObject* player, GameObject* object);
	static void SendPutPlayer(GameObject* player, GameObject* object);
	static void SendObjectHp(GameObject* player, GameObject* object);
	static void SendObjectDamage(GameObject * player, GameObject * damagedObj, int damage);
	static void SendObjectLevelup(GameObject * player, GameObject * Obj);
	static void SendObjectExp(GameObject * player);
	static void SendPlayEnding(GameObject * player);
	static void SendPutMonster(GameObject* player, GameObject* object);
};