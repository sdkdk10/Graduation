#pragma once
#include <mutex>
#include <unordered_set>
#include "GameObject.h"
#include "Defines.h"
#include "Protocol.h"

class Player : public GameObject
{
public:
	explicit Player();
	virtual ~Player();

public:
	virtual void Initialize();

public:
	SOCKET s_;
	std::unordered_set<GameObject*> viewList_;
	std::mutex vlm_;
	EXOver exover_;
	int packetSize_;
	int prevSize_;
	char prevPacket_[MAX_PACKET_SIZE];
};