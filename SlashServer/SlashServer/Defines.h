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
const double TEST_MOVE_SPEED		{ 1.0 };
const double WARRIOR_ROLL_SPEED		{ 0.5 };
const double WIZARD_TELEPORT_SPEED	{ 5.0 };
const unsigned char ROLL_MAX_FRAME	{ 10 };
const double	RADIAN_TO_DEGREE		{ 57.3248 };
const int ROLL_DELAY				{ 30 };

const float SKILL_MOVE_DISTANCE{ 50 };

const int WARRIOR_SKILL1_DMG{ 10 };
const float WARRIOR_SKILL1_DELAY{ 150.f };

const int WARRIOR_SKILL2_DMG{ 15 };
const float WARRIOR_SKILL2_DELAY{ 30.f };
const float WARRIOR_SKILL2_SPEED{ 100.f };
const float WARRIOR_SKILL2_MAX_RANGE{ 180.f };
const float WARRIOR_SKILL2_WIDTH{ 3.f };
const float WARRIOR_ULTIMATE_SKILL2_WIDTH{ 4.f };
const float WARRIOR_SKILL2_DEPTH{ 0.1f };

const int WARRIOR_SKILL3_DMG{ 40 };
const float WARRIOR_SKILL3_DELAY{ 200.f };
const float WARRIOR_SKILL3_WIDTH{ 4.f };
const float WARRIOR_SKILL3_DEPTH{ 6.f };

const int WIZARD_SKILL1_DMG{ 15 };
const float WIZARD_SKILL1_DELAY{ 100.f };
const float WIZARD_SKILL1_SPEED{ 100.f };
const float WIZARD_SKILL1_MAX_RANGE{ 130.f };
const float WIZARD_SKILL1_WIDTH{ 2.f };
const float WIZARD_SKILL1_DEPTH{ 0.1f };

const int WIZARD_SKILL2_DMG{ 10 };
const float WIZARD_SKILL2_DELAY{ 1200.f };
const float WIZARD_SKILL2_WIDTH{ 3.f };
const float WIZARD_SKILL2_DEPTH{ 4.f };

const int WIZARD_SKILL3_DMG{ 50 };

const int MAGE_ULTIMATE_DMG{ 100 };
const int INIT_PLAYER_DMG{ 10 };

enum MonsterInitHP 
{
	TURTLE_HP = 10,
	SPIDER_HP = 20,
	NAGAGUARD_HP = 50,
	TREEGUARD_HP = 100,
	ROCKWARRIOR_HP = 200
};

const DirectX::XMFLOAT3	DIR_VECTOR_Z{ 0.0f, 0.0f, 1.0f };
const DirectX::XMFLOAT3	DIR_VECTOR_X{ 1.0f, 0.0f, 0.0f };

const DirectX::XMFLOAT3	INIT_PLAYER_POS{ -11.0f, 0.0f, -3.0f };


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