#pragma once

#include <WinSock2.h>
#include <windows.h>   // include important windows stuff
#include <windowsx.h>
#include "Protocol.h"
#pragma comment (lib, "ws2_32.lib")

#define	WM_SOCKET				WM_USER + 1

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
	void SendPacket(const DWORD& keyInput);
	void ProcessPacket(char* ptr);
public:
	SOCKET mysocket;
	WSABUF	send_wsabuf;
	char 	send_buffer[BUF_SIZE];
	WSABUF	recv_wsabuf;
	char	recv_buffer[BUF_SIZE];
	char	packet_buffer[BUF_SIZE];
	DWORD		in_packet_size = 0;
	int		saved_packet_size = 0;
	int		myid;
private:
	virtual void Free();
	CGameObject* pObj;
};
