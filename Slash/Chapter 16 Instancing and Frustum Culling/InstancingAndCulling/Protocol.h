#pragma once

const float fMoveSpeed = 0.1f;
const float CS_SEND_PACKET_DELAY = 10;
const float fRotateSpeed = 15.0f;

enum STATE { IDLE, WALK, ATTACK1, ATTACK2, ATTACK3, DAMAGE };

#define MAX_BUFF_SIZE   4000
#define MAX_PACKET_SIZE  255

#define BOARD_WIDTH   400
#define BOARD_HEIGHT  400

#define VIEW_RADIUS   3

#define MAX_USER 10

#define NPC_START  1000
#define NUM_OF_NPC  20000

#define MY_SERVER_PORT  4000

#define MAX_STR_SIZE  100

#define SC_POS   1
#define SC_PUT_PLAYER    2
#define SC_REMOVE_PLAYER 3
#define SC_CHAT			4
#define SC_ROTATE		5
#define SC_STATE		6

#define CS_DIR_FORWARD					0x01
#define CS_DIR_BACKWARD					0x02
#define CS_DIR_LEFT						0x04
#define CS_DIR_RIGHT					0x08
#define CS_COLLSION						0x16
#define CS_STOP							0x32
#define CS_ATTACK						0x33


#pragma pack (push, 1)

struct cs_packet_dir {
	BYTE size;
	BYTE type;
	XMFLOAT3 Shift;
	XMFLOAT4X4 World;
};

struct cs_packet_attack {
	BYTE size;
	BYTE type;
};

struct cs_packet_stop {
	BYTE size;
	BYTE type;
};

struct cs_packet_chat {
	BYTE size;
	BYTE type;
	WCHAR message[MAX_STR_SIZE];
};

struct sc_packet_pos {
	BYTE size;
	BYTE type;
	WORD id;
	float x;
	float y;
	float z;
};

struct sc_packet_rotate {
	BYTE size;
	BYTE type;
	WORD id;
	bool IsClockWise;
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
	float x;
	float y;
	float z;
	float RotateNum;
};
struct sc_packet_remove_player {
	BYTE size;
	BYTE type;
	WORD id;
};

struct sc_packet_chat {
	BYTE size;
	BYTE type;
	WORD id;
	WCHAR message[MAX_STR_SIZE];
};

#pragma pack (pop)