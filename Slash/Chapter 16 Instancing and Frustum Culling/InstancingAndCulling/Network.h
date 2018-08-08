#pragma once

#include <WinSock2.h>
#include <windows.h>   // include important windows stuff
#include <windowsx.h>
#include "../../SlashServer/SlashServer/Protocol.h"
#pragma comment (lib, "ws2_32.lib")

#define   WM_SOCKET            WM_USER + 1
const int MYPLAYERID = 0;

#include "Base.h"
#include "Define.h"

class CGameObject;

class CNetwork
	: public CBase
{
	DECLARE_SINGLETON(CNetwork)

private:
	explicit CNetwork();
public:
	virtual ~CNetwork();
	void InitSock(HWND MainWnd);
	void ReadPacket(SOCKET sock);
	void SendDirKeyPacket(DWORD& keyInput, XMFLOAT4X4& world);
	void SendAttackPacket(void);
	void SendStopPacket(void);
	void ProcessPacket(char* ptr);
	int GetMyID() {
		return myid;
	}
public:
	SOCKET mysocket;
	WSABUF   send_wsabuf;
	char    send_buffer[MAX_BUFF_SIZE];
	WSABUF   recv_wsabuf;
	char   recv_buffer[MAX_BUFF_SIZE];
	char   packet_buffer[MAX_BUFF_SIZE];
	DWORD      in_packet_size = 0;
	int      saved_packet_size = 0;
	int      myid;
private:
	virtual void Free();
};