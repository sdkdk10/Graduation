#pragma once
#include <DirectXCollision.h>

enum State 
{ 
	STATE_IDLE,
	STATE_WALK,
	STATE_ATTACK1, 
	STATE_ATTACK2, 
	STATE_ATTACK3,
	STATE_DEAD,
	STATE_ULTIMATE,
	STATE_ROLL,
	STATE_HIT,
	STATE_END,
};
enum MonsterZone
{ 
	MONSTER_ZONE1, 
	MONSTER_ZONE2,
	MONSTER_ZONE3
};
enum NPCType
{
	NPC_SPIDER,
	NPC_NAGA_GUARD,
	NPC_ROCK_WARRIOR,
	NPC_TREE_GUARD,
	NPC_TURTLE,
	NPC_DRAGON
};
enum SpiderType
{
	SPIDER_BRICK,
	SPIDER_STONE,
	SPIDER_TILE,
	SPIDER_ICE,
	SPIDER_END
};
enum PlayerType
{
	PLAYER_WARRIOR,
	PLAYER_MAGE
};

const float CS_SEND_PACKET_DELAY = 10;
static const int EVT_RECV = 0;
static const int EVT_SEND = 1;
static const int EVT_CHASE = 2;
static const int EVT_MONSTER_ATTACK = 3;
static const int EVT_PLAYER_ATTACK = 4;
static const int EVT_MONSTER_DAMAGED = 5;
static const int EVT_PLAYER_DAMAGED = 6;
static const int EVT_ATTACKMOVE = 7;
static const int EVT_MONSTER_RESPOWN = 8;
static const int EVT_PLAYER_RESPOWN = 9;
static const int EVT_PLAYER_ROLL = 10;

#define MY_SERVER_PORT  4000

#define MAX_BUFF_SIZE   1024
#define MAX_PACKET_SIZE  256
#define MAX_STR_SIZE  100

#define MAPOBJECT_RADIUS	30
#define VIEW_RADIUS		24
#define AGRO_RADIUS		18
#define PLAYER_ATTACK_RADIUS	6
#define CLOSE_RADIUS	4

#define NUM_OF_PLAYER 1000
#define NUM_OF_MAPOBJECT 100


#define NUM_OF_NPC_SPIDER			5
#define NUM_OF_NPC_NAGAGUARD		5
#define NUM_OF_NPC_ROCKWARRIOR		5
#define NUM_OF_NPC_TREEGUARD		5
#define NUM_OF_NPC_TURTLE			5
#define NUM_OF_NPC_TOTAL			NUM_OF_NPC_SPIDER + NUM_OF_NPC_NAGAGUARD + NUM_OF_NPC_ROCKWARRIOR + NUM_OF_NPC_TREEGUARD + NUM_OF_NPC_TURTLE

#define NPC_ID_START				NUM_OF_PLAYER
#define SPIDER_ID_START				0
#define NAGAGUARD_ID_START			SPIDER_ID_START + NUM_OF_NPC_SPIDER
#define ROCKWARRIOR_ID_START		NAGAGUARD_ID_START + NUM_OF_NPC_NAGAGUARD
#define TREEGUARD_ID_START			ROCKWARRIOR_ID_START + NUM_OF_NPC_ROCKWARRIOR
#define TURTLE_ID_START				TREEGUARD_ID_START + NUM_OF_NPC_TREEGUARD


#define CS_DIR_FORWARD					0x01
#define CS_DIR_BACKWARD					0x02
#define CS_DIR_LEFT						0x04
#define CS_DIR_RIGHT					0x08
#define CS_ROLL							0x10
#define CS_STOP							0x20
#define CS_ATTACK1						0x21
#define CS_ATTACK2						0x22
#define CS_ATTACK3						0x23
#define CS_MAP_INIT_DATA				0x24
#define CS_PLAYER_TYPE					0x25
#define CS_ULTIMATE_START				0x26
#define CS_ULTIMATE_ON					0x27
#define CS_ULTIMATE_OFF					0x28
#define CS_MAPOBJECT_NUM				0x29

#define SC_WALK_MOVE					0x01
#define SC_ROLL_MOVE					0x02
#define SC_PUT_PLAYER					0x03
#define SC_REMOVE_OBJECT				0x04
#define SC_CHAT							0x05
#define SC_ROTATE						0x06
#define SC_STATE						0x07
#define SC_HP							0x08
#define SC_PUT_MONSTER					0x09
#define SC_DAMAGE						0x0a
#define SC_ULTIMATE_ON					0x0b
#define SC_ULTIMATE_OFF					0x0c

static const int MOVE_PACKET_START = CS_DIR_FORWARD;
static const int MOVE_PACKET_END = CS_DIR_FORWARD + CS_DIR_BACKWARD + CS_DIR_LEFT + CS_DIR_RIGHT + CS_ROLL;

using BYTE = unsigned char;
using WORD = unsigned short;
using WCHAR = wchar_t;

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
	DirectX::XMFLOAT4X4 world;
	DirectX::BoundingBox bounds;
};

struct cs_packet_player_type {
	BYTE size;
	BYTE type;
	BYTE playerType;
};

struct cs_packet_ultimate_start {
	BYTE size;
	BYTE type;
};
struct cs_packet_ultimate_on {
	BYTE size;
	BYTE type;
};
struct cs_packet_ultimate_off {
	BYTE size;
	BYTE type;
};
struct cs_packet_mapobject_num {
	BYTE size;
	BYTE type;
	WORD mapObjectNum;
};


struct sc_packet_move {
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
	BYTE playerType;
};

struct sc_packet_put_monster {
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
	WORD hp;
};

struct sc_packet_damage {
	BYTE size;
	BYTE type;
	WORD id;
	WORD dmg;
};

struct sc_packet_chat {
	BYTE size;
	BYTE type;
	WORD id;
	WCHAR message[MAX_STR_SIZE];
};

struct sc_packet_ultimate_on {
	BYTE size;
	BYTE type;
	WORD id;
	BYTE state;
};

struct sc_packet_ultimate_off {
	BYTE size;
	BYTE type;
	WORD id;
};
#pragma pack (pop)