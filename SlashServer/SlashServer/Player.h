#pragma once
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
	EXOver exover_;
	int packetSize_;
	int prevSize_;
	char prevPacket_[MAX_PACKET_SIZE];
	unsigned char rollDir;
	unsigned char rollCurFrame;
	unsigned char playerType_;
	bool isWarriorUltimateMode;
};