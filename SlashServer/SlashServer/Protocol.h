#pragma once

const float fMoveSpeed = 0.1f;
const float fDegree = 57.3248f;
const float fRotateDegree = 15.0f;
const float fMonsterRotateDegree = 0.66688f * 5.0f;
const float fMonsterMoveSpeed = 0.033344f * 5.0f;
const int iAttackDelay = 500;
const XMFLOAT3	xmf3Height{ 0.0f, 0.0f, 1.0f };
const XMFLOAT3	xmf3Width{ 1.0f, 0.0f, 0.0f };
const XMFLOAT3	xmf3Depth{ 0.0f, 1.0f, 0.0f };
//const XMFLOAT3 InitLookVector{ 1.02601e-05f, 0.00644221f, 0.0495832f }; // ¸ðµ¨ÀÇ ÃÊ±â ·èº¤ÅÍ
//const XMFLOAT3 InitRightVector{ -0.0499999f, 7.96318e-05f, 0 }; // ¸ðµ¨ÀÇ ÃÊ±â ¶óÀÌÆ®º¤ÅÍ
const XMFLOAT3 InitLookVector{ 1.03023e-05f, 0.00644221f, 0.0495832f }; // ¸ðµ¨ÀÇ ÃÊ±â ·èº¤ÅÍ
const XMFLOAT3 InitRightVector{ -0.0499999f, 7.96318e-05f, 3.72529e-08f }; // ¸ðµ¨ÀÇ ÃÊ±â ¶óÀÌÆ®º¤ÅÍ
const XMFLOAT3 UpVector{ 7.8968e-05f, 0.0495832f, -0.00644222f }; // ¸ðµ¨ ÃÊ±â ¾÷º¤ÅÍ °ª

enum STATE { IDLE, WALK, ATTACK1, ATTACK2, ATTACK3, DEAD };

const float CS_SEND_PACKET_DELAY = 10;

static const int EVT_RECV = 0;
static const int EVT_SEND = 1;
static const int EVT_CHASE = 2;
static const int EVT_MONSTER_ATTACK = 3;
static const int EVT_PLAYER_ATTACK = 4;
static const int EVT_DAMAGE = 5;
static const int EVT_ATTACKMOVE = 6;
static const int EVT_RESPOWN = 7;

#define MAX_BUFF_SIZE   1024
#define MAX_PACKET_SIZE  256

#define BOARD_WIDTH   400
#define BOARD_HEIGHT  400

#define VIEW_RADIUS		12
#define AGRO_RADIUS		8
#define CLOSE_RADIUS	4

#define MAX_MAPOBJECT 61
#define MAX_USER 1000

#define NPC_START  2000
#define NUM_OF_NPC  3000

#define MY_SERVER_PORT  4000

#define MAX_STR_SIZE  100


#define CS_DIR_FORWARD					0x01
#define CS_DIR_BACKWARD					0x02
#define CS_DIR_LEFT						0x04
#define CS_DIR_RIGHT					0x08
#define CS_DIR_UP						0x10
#define CS_DIR_DOWN						0x20
#define CS_STOP							0x40
#define CS_ATTACK						0x41
#define CS_MAP_INIT_DATA				0x42

#define SC_POS   1
#define SC_PUT_PLAYER    2
#define SC_REMOVE_OBJECT 3
#define SC_CHAT			4
#define SC_ROTATE		5
#define SC_STATE		6
#define SC_HP			7

static const int MOVE_PACKET_START = CS_DIR_FORWARD;
static const int MOVE_PACKET_END = CS_DIR_FORWARD + CS_DIR_BACKWARD + CS_DIR_LEFT + CS_DIR_RIGHT + CS_DIR_UP + CS_DIR_DOWN;

#pragma pack (push, 1)

struct cs_packet_dir {
	BYTE size;
	BYTE type;
};

struct cs_packet_stop {
	BYTE size;
	BYTE type;
};

struct cs_packet_attack {
	BYTE size;
	BYTE type;
};

struct cs_packet_chat {
	BYTE size;
	BYTE type;
	WCHAR message[MAX_STR_SIZE];
};

struct cs_packet_mapinitdata {
	BYTE size;
	BYTE type;
	XMFLOAT4X4 world;
	BoundingBox bounds;
	//BoundingOrientedBox xmOOBB;
	//BoundingOrientedBox xmOOBBTransformed;
};

struct sc_packet_pos {
	BYTE size;
	BYTE type;
	WORD id;
	float posX;
	float posY;
	float posZ;
};

struct sc_packet_look_degree {
	BYTE size;
	BYTE type;
	WORD id;
	float lookDegree;
};

struct sc_packet_state {
	BYTE size;
	BYTE type;
	WORD id;
	BYTE state;
};

struct sc_packet_put_player {
	BYTE size;
	BYTE type;
	WORD id;
	BYTE state;
	float posX;
	float posY;
	float posZ;
	float lookDegree;
};

struct sc_packet_remove_object {
	BYTE size;
	BYTE type;
	WORD id;
};

struct sc_packet_hp {
	BYTE size;
	BYTE type;
	WORD id;
	unsigned short hp;
};

struct sc_packet_chat {
	BYTE size;
	BYTE type;
	WORD id;
	WCHAR message[MAX_STR_SIZE];
};

#pragma pack (pop)