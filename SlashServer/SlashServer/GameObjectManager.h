#pragma once
#include <array>
#include "SingleTone.h"
#include "Protocol.h"
#include "Player.h"
#include "NPC.h"
#include "MapObject.h"
#include "ThreadManager.h"

class ThreadManager;
class GameObjectManager : public SingleTone<GameObjectManager>
{
public:
	explicit GameObjectManager();
	virtual ~GameObjectManager();
public:
	void InitGameObjects();
	void PutNewPlayer(GameObject* player);
	void DisconnectPlayer(GameObject* player);
	void WakeUpNPC(GameObject * npc, GameObject * target);
	void ChasingPlayer(GameObject* npc, GameObject* player);
	void MonsterAttack(GameObject* monsterID, GameObject* playerID);
	void PlayerAttack(GameObject* playerID);
	void PlayerDamaged(GameObject* monsterID, GameObject* playerID);
	void MonsterDamaged(GameObject* playerID, GameObject* monsterID);
	void PlayerRespown(GameObject* playerID);
	void MonsterRespown(GameObject* monsterID);
	void ProcessMove(GameObject * player, unsigned char dirType, unsigned char moveType);
	void ProcessPacket(GameObject* cl, char * packet);
public:
	 GameObject*										GetPlayer(unsigned int playerIndex) { if (playerIndex >= playerArray_.size()) return nullptr; return playerArray_[playerIndex]; }
	inline std::array<GameObject*, NUM_OF_PLAYER>&				GetPlayerList() { return playerArray_; }
	inline GameObject*										GetNPC(unsigned int npcIndex) { if (npcIndex >= npcArray_.size()) return nullptr; return npcArray_[npcIndex]; }
	inline std::array<GameObject*, NUM_OF_NPC_TOTAL>&		GetNPCList() { return npcArray_; }
	inline GameObject*										GetMapObject(unsigned int mapObjectIndex) { if (mapObjectIndex >= mapObjectArray_.size()) return nullptr; return mapObjectArray_[mapObjectIndex]; }
	inline std::array<GameObject*, NUM_OF_MAPOBJECT>&			GetMapObjectList() { return mapObjectArray_; }
	GameObjectManager*										GetGameObjectManager() { return this; }
private:
	ThreadManager* threadManager_;
private:
	std::array<GameObject*, NUM_OF_PLAYER>			playerArray_;
	std::array<GameObject*, NUM_OF_NPC_TOTAL>				npcArray_;
	std::array<GameObject*, NUM_OF_MAPOBJECT>		mapObjectArray_;
};
