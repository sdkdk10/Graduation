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
	static void SendPutObject(GameObject* player, GameObject* object);
	static void SendObjectHp(GameObject* player, GameObject* object);
	static void SendPutMonster(GameObject* player, GameObject* object);
};