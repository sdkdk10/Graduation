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
	void PutNewPlayer(GameObject * player, BYTE type);
	void DisconnectPlayer(GameObject* player);
	void WakeUpNPC(GameObject * npc, GameObject * target);
	void ChasingPlayer(GameObject* npc, GameObject* player);
	void MonsterAttack(GameObject* monster, GameObject* player);
	void ProcessWarriorAttack1(GameObject* player);
	void ProcessWarriorAttack2(GameObject * player);
	void ProcessWarriorAttack3(GameObject * player);
	void ProcessWizardAttack1(GameObject * player);
	void ProcessWizardAttack2(GameObject * player);
	void ProcessWizardAttack3(GameObject * player);
	void PlayerDamaged(GameObject* player, GameObject* monster);
	void MonsterDamaged(GameObject * monster, GameObject * player);
	void PlayerRespown(GameObject* player);
	void MonsterRespown(GameObject* monster);
	void ProcessMove(GameObject * player, unsigned char dirType);
	void ProcessPacket(GameObject* cl, char * packet);
	void SearchNewTargetPlayer(GameObject* monster);
	void AddExp(GameObject * player, GameObject* monster);
public:
	inline GameObject*&										GetPlayer(unsigned int playerIndex) { return playerArray_[playerIndex]; }
	inline std::array<GameObject*, NUM_OF_PLAYER>&				GetPlayerList() { return playerArray_; }
	inline GameObject*&										GetNPC(unsigned int npcIndex) { return npcArray_[npcIndex]; }
	inline std::array<GameObject*, NUM_OF_NPC_TOTAL>&		GetNPCList() { return npcArray_; }
	inline GameObject*&										GetMapObject(unsigned int mapObjectIndex) { return mapObjectArray_[mapObjectIndex]; }
	inline std::array<GameObject*, NUM_OF_MAPOBJECT>&			GetMapObjectList() { return mapObjectArray_; }
	GameObjectManager*										GetGameObjectManager() { return this; }
private:
	ThreadManager* threadManager_;
private:
	std::array<GameObject*, NUM_OF_PLAYER>			playerArray_;
	std::array<GameObject*, NUM_OF_NPC_TOTAL>				npcArray_;
	std::array<GameObject*, NUM_OF_MAPOBJECT>		mapObjectArray_;
private:
 	WORD mapObjectNum_;
};
