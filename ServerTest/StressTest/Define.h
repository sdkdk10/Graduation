#pragma once
#include "../../SlashServer/SlashServer/Protocol.h"

enum MODE { HOTSPOT_TEST = 1, MULTI_TEST };
enum OPTYPE { OP_SEND, OP_RECV, OP_DO_MOVE };

const int MAP_WIDTH{ 750 };
const int MAP_HEIGHT{ 750 };

struct OverlappedEx {
	WSAOVERLAPPED over;
	WSABUF wsaBuf;
	unsigned char iocpBuf[MAX_BUFF_SIZE];
	OPTYPE eventType;
	int event_target;
};