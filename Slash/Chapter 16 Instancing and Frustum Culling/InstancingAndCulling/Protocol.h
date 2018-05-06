#pragma once

const float CS_SEND_PACKET_DELAY = 0.001;

#define	BUF_SIZE					1024

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

#define SC_POS           1
#define SC_PUT_PLAYER    2
#define SC_REMOVE_PLAYER 3
#define SC_CHAT			4

#pragma pack (push, 1)

struct cs_packet_dir {
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

struct sc_packet_put_player {
	BYTE size;
	BYTE type;
	WORD id;
	float x;
	float y;
	float z;
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