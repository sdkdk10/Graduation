#pragma once

#define WIN32_LEAN_AND_MEAN

#pragma comment (lib, "ws2_32.lib")
#pragma comment(lib,  "lua53.lib")

#include <WinSock2.h>
#include "Protocol.h"

const int MAX_WORKERTHREAD_NUM			{ 4 };
const double MONSTER_MOVE_SPEED		{ 0.3 };
const double MONSTER_ROTATE_DEGREE	{ 7.0 };
const double PLAYER_MOVE_SPEED		{ 0.15 };
const double PLAYER_ROTATE_DEGREE	{ 15.0 };
const double PLAYER_ROLL_SPEED		{ 0.5 };
const unsigned char ROLL_MAX_FRAME	{ 10 };
const double	RADIAN_TO_DEGREE		{ 57.3248 };
const int ATTACK_DELAY				{ 500 };
const int ROLL_DELAY				{ 30 };

const int MAGE_ULTIMATE_DAMAGE{ 50 };

const DirectX::XMFLOAT3	DIR_VECTOR_Z{ 0.0f, 0.0f, 1.0f };
const DirectX::XMFLOAT3	DIR_VECTOR_X{ 1.0f, 0.0f, 0.0f };

class GameObject;

enum ThreadNum
{
	ACCEPT_THREAD, 
	WORKER_THREAD1, 
	WORKER_THREAD2, 
	WORKER_THREAD3, 
	WORKER_THREAD4, 
	TIMER_THREAD
};

enum ObjectType
{
	TYPE_PLAYER,
	TYPE_MONSTER,
	TYPE_MAPOBJECT
};


struct EXOver {
	WSAOVERLAPPED wsaOver;
	char eventType;
	GameObject* eventTarget;
	WSABUF wsaBuf;
	char ioBuf[MAX_BUFF_SIZE];
};