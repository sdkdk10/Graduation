#include "stdafx.h"
#include "Network.h"
#include "GameObject.h"
#include "Management.h"
#include "TestScene.h"
#include "Player.h"
#include "Skeleton.h"
#include "Spider.h"
#include "GameTimer_Manager.h"

IMPLEMENT_SINGLETON(CNetwork)

CNetwork::CNetwork()
{
}

CNetwork::~CNetwork()
{
}

void CNetwork::InitSock(HWND MainWnd)
{
	WSADATA   wsadata;
	WSAStartup(MAKEWORD(2, 2), &wsadata);

	mysocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);

	int option = false;               //네이글 알고리즘 on/off
	setsockopt(mysocket,             //해당 소켓
		IPPROTO_TCP,          //소켓의 레벨
		TCP_NODELAY,          //설정 옵션
		(const char*)&option, // 옵션 포인터
		sizeof(option));      //옵션 크기

	SOCKADDR_IN ServerAddr;
	ZeroMemory(&ServerAddr, sizeof(SOCKADDR_IN));
	ServerAddr.sin_family = AF_INET;
	ServerAddr.sin_port = htons(MY_SERVER_PORT);
	//ServerAddr.sin_addr.s_addr = inet_addr("192.168.83.227");
	//ServerAddr.sin_addr.s_addr = inet_addr("192.168.82.227");
	ServerAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	//ServerAddr.sin_addr.s_addr = inet_addr("192.168.82.227");
	int Result = WSAConnect(mysocket, (sockaddr *)&ServerAddr, sizeof(ServerAddr), NULL, NULL, NULL, NULL);

	if (Result)
		MessageBox(0, L"Connect Failed.", 0, 0);

	WSAAsyncSelect(mysocket, MainWnd, WM_SOCKET, FD_CLOSE | FD_READ);

	send_wsabuf.buf = send_buffer;
	send_wsabuf.len = MAX_BUFF_SIZE;
	recv_wsabuf.buf = recv_buffer;
	recv_wsabuf.len = MAX_BUFF_SIZE;

}

void CNetwork::ReadPacket(SOCKET sock)
{
	DWORD iobyte, ioflag = 0;

	int ret = WSARecv(sock, &recv_wsabuf, 1, &iobyte, &ioflag, NULL, NULL);
	if (ret) {
		int err_code = WSAGetLastError();
		printf("Recv Error [%d]\n", err_code);
	}

	BYTE *ptr = reinterpret_cast<BYTE *>(recv_buffer);

	while (0 != iobyte) {
		if (0 == in_packet_size) in_packet_size = ptr[0];
		if (iobyte + saved_packet_size >= in_packet_size) {
			memcpy(packet_buffer + saved_packet_size, ptr, in_packet_size - saved_packet_size);
			ProcessPacket(packet_buffer);
			ptr += in_packet_size - saved_packet_size;
			iobyte -= in_packet_size - saved_packet_size;
			in_packet_size = 0;
			saved_packet_size = 0;
		}
		else {
			memcpy(packet_buffer + saved_packet_size, ptr, iobyte);
			saved_packet_size += iobyte;
			iobyte = 0;
		}
	}
}

void CNetwork::SendDirKeyPacket(DWORD& keyInput, XMFLOAT4X4& world)
{
	cs_packet_dir *my_packet = reinterpret_cast<cs_packet_dir *>(send_buffer);
	my_packet->size = sizeof(cs_packet_dir);
	send_wsabuf.len = sizeof(cs_packet_dir);
	DWORD iobyte;

	my_packet->type = keyInput;

	int ret = WSASend(mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);
	if (ret) {
		int error_code = WSAGetLastError();
		printf("Error while sending packet [%d]", error_code);
	}
}

void CNetwork::SendAttack1Packet(void)
{
	cs_packet_attack *my_packet = reinterpret_cast<cs_packet_attack *>(send_buffer);
	my_packet->size = sizeof(cs_packet_attack);
	send_wsabuf.len = sizeof(cs_packet_attack);
	DWORD iobyte;

	my_packet->type = CS_ATTACK1;

	int ret = WSASend(mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);

	if (ret) {
		int error_code = WSAGetLastError();
		printf("Error while sending packet [%d]", error_code);
	}
}

void CNetwork::SendAttack2Packet(void)
{
	cs_packet_attack *my_packet = reinterpret_cast<cs_packet_attack *>(send_buffer);
	my_packet->size = sizeof(cs_packet_attack);
	send_wsabuf.len = sizeof(cs_packet_attack);
	DWORD iobyte;

	my_packet->type = CS_ATTACK2;

	int ret = WSASend(mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);

	if (ret) {
		int error_code = WSAGetLastError();
		printf("Error while sending packet [%d]", error_code);
	}
}

void CNetwork::SendAttack3Packet(void)
{
	cs_packet_attack *my_packet = reinterpret_cast<cs_packet_attack *>(send_buffer);
	my_packet->size = sizeof(cs_packet_attack);
	send_wsabuf.len = sizeof(cs_packet_attack);
	DWORD iobyte;

	my_packet->type = CS_ATTACK3;

	int ret = WSASend(mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);

	if (ret) {
		int error_code = WSAGetLastError();
		printf("Error while sending packet [%d]", error_code);
	}
}

void CNetwork::SendStopPacket(void)
{
	cs_packet_stop *my_packet = reinterpret_cast<cs_packet_stop *>(send_buffer);
	my_packet->size = sizeof(cs_packet_stop);
	send_wsabuf.len = sizeof(cs_packet_stop);
	DWORD iobyte;

	my_packet->type = CS_STOP;
	int ret = WSASend(mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);

	if (ret) {
		int error_code = WSAGetLastError();
		printf("Error while sending packet [%d]", error_code);
	}
}

void CNetwork::ProcessPacket(char * ptr)
{
	switch (ptr[1])
	{
	case SC_PUT_PLAYER:
	{
		sc_packet_put_player * my_packet = reinterpret_cast<sc_packet_put_player *>(ptr);
		static bool first_time = true;
		int id = my_packet->id;
		if (first_time) {
			first_time = false;
			myid = my_packet->id;
		}
		if (myid == id)
		{
			CManagement::GetInstance()->Find_Object(L"Layer_Player", 0)->m_bIsConnected = true;
			CManagement::GetInstance()->Find_Object(L"Layer_Player", 0)->SetPosition(my_packet->posX, my_packet->posY, my_packet->posZ);
			CManagement::GetInstance()->Find_Object(L"Layer_Player", 0)->Rotation(0.f, 0.f, my_packet->lookDegree);
			CManagement::GetInstance()->Find_Object(L"Layer_Player", 0)->SetObjectAnimState(my_packet->state);
			CManagement::GetInstance()->Find_Object(L"Layer_Player", 0)->SetHp(200);
		}
		else if (id < NPC_START)
		{
			CManagement::GetInstance()->Find_Object(L"Layer_Skeleton", id)->m_bIsConnected = true;
			CManagement::GetInstance()->Find_Object(L"Layer_Skeleton", id)->SetPosition(my_packet->posX, my_packet->posY, my_packet->posZ);
			CManagement::GetInstance()->Find_Object(L"Layer_Skeleton", id)->Rotation(0.f, 0.f, my_packet->lookDegree);
			CManagement::GetInstance()->Find_Object(L"Layer_Skeleton", id)->SetObjectAnimState(my_packet->state);
		}
		else
		{
			CManagement::GetInstance()->Find_Object(L"Layer_Spider", id - NPC_START)->m_bIsConnected = true;
			CManagement::GetInstance()->Find_Object(L"Layer_Spider", id - NPC_START)->SetPosition(my_packet->posX, my_packet->posY, my_packet->posZ);
			CManagement::GetInstance()->Find_Object(L"Layer_Spider", id - NPC_START)->Rotation(0.f, my_packet->lookDegree, 0.0f);
			CManagement::GetInstance()->Find_Object(L"Layer_Spider", id - NPC_START)->SetObjectAnimState(my_packet->state);
		}
		break;
	}
	case SC_POS:
	{
		sc_packet_pos * my_packet = reinterpret_cast<sc_packet_pos *>(ptr);
		int id = my_packet->id;
		if (myid == id)
		{
			dynamic_cast<Player*>(CManagement::GetInstance()->Find_Object(L"Layer_Player", 0))->SetPosition(my_packet->posX, my_packet->posY, my_packet->posZ);
			dynamic_cast<Player*>(CManagement::GetInstance()->Find_Object(L"Layer_Player", 0))->SetObjectAnimState(WALK);
		}
		else if (id < NPC_START)
		{
			dynamic_cast<CSkeleton*>(CManagement::GetInstance()->Find_Object(L"Layer_Skeleton", id))->SetPosition(my_packet->posX, my_packet->posY, my_packet->posZ);
			dynamic_cast<CSkeleton*>(CManagement::GetInstance()->Find_Object(L"Layer_Skeleton", id))->SetObjectAnimState(WALK);
		}
		else
		{
			dynamic_cast<Spider*>(CManagement::GetInstance()->Find_Object(L"Layer_Spider", id - NPC_START))->SetPosition(my_packet->posX, my_packet->posY, my_packet->posZ);
			dynamic_cast<Spider*>(CManagement::GetInstance()->Find_Object(L"Layer_Spider", id - NPC_START))->SetObjectAnimState(WALK);
		}
		break;
	}
	case SC_ROTATE:
	{
		sc_packet_look_degree * my_packet = reinterpret_cast<sc_packet_look_degree *>(ptr);
		int id = my_packet->id;
		if (myid == id)
		{
			dynamic_cast<Player*>(CManagement::GetInstance()->Find_Object(L"Layer_Player", 0))->Rotation(0.f, 0.f, my_packet->lookDegree);
			//cout <<  int(my_packet->lookDegree) << endl;
		}
		else if (id < NPC_START)
			dynamic_cast<CSkeleton*>(CManagement::GetInstance()->Find_Object(L"Layer_Skeleton", id))->Rotation(0.f, 0.f, my_packet->lookDegree);
		else
		{
			dynamic_cast<Spider*>(CManagement::GetInstance()->Find_Object(L"Layer_Spider", id - NPC_START))->Rotation(0.f, my_packet->lookDegree, 0.f);
		}
		break;
	}
	case SC_STATE:
	{
		sc_packet_state * my_packet = reinterpret_cast<sc_packet_state *>(ptr);
		int id = my_packet->id;
		if (myid == id)
		{
			dynamic_cast<Player*>(CManagement::GetInstance()->Find_Object(L"Layer_Player", 0))->SetObjectAnimState(my_packet->state);
		}
		else if (id < NPC_START)
		{
			dynamic_cast<CSkeleton*>(CManagement::GetInstance()->Find_Object(L"Layer_Skeleton", id))->SetObjectAnimState(my_packet->state);
		}
		else
		{
			dynamic_cast<Spider*>(CManagement::GetInstance()->Find_Object(L"Layer_Spider", id - NPC_START))->SetObjectAnimState(my_packet->state);
		}
		break;
	}
	case SC_REMOVE_OBJECT:
	{
		sc_packet_remove_object *my_packet = reinterpret_cast<sc_packet_remove_object *>(ptr);
		int id = my_packet->id;
		if (myid == id)
			CManagement::GetInstance()->Find_Object(L"Layer_Player", 0)->m_bIsConnected = false;
		else if (id < NPC_START)
			CManagement::GetInstance()->Find_Object(L"Layer_Skeleton", id)->m_bIsConnected = false;
		else
		{
			CManagement::GetInstance()->Find_Object(L"Layer_Spider", id - NPC_START)->m_bIsConnected = false;
		}
		break;
	}
	case SC_HP:
	{
		sc_packet_hp *my_packet = reinterpret_cast<sc_packet_hp *>(ptr);
		int id = my_packet->id;
		unsigned short hp = my_packet->hp;
		if (myid == id)
			CManagement::GetInstance()->Find_Object(L"Layer_Player", 0)->SetHp(100);
		//else if (id < NPC_START)
		//	CManagement::GetInstance()->Find_Object(L"Layer_Skeleton", id)->m_bIsConnected = false;
		//else
		//{
		//	CManagement::GetInstance()->Find_Object(L"Layer_Spider", id - NPC_START)->m_bIsConnected = false;
		//}
		break;
	}
	case SC_PUT_MONSTER:
	{
		sc_packet_put_monster *my_packet = reinterpret_cast<sc_packet_put_monster *>(ptr);
		int id = my_packet->id;
		CManagement::GetInstance()->Find_Object(L"Layer_Spider", id - NPC_START)->m_bIsConnected = true;
		CManagement::GetInstance()->Find_Object(L"Layer_Spider", id - NPC_START)->SetPosition(my_packet->posX, my_packet->posY, my_packet->posZ);
		CManagement::GetInstance()->Find_Object(L"Layer_Spider", id - NPC_START)->Rotation(0.f, my_packet->lookDegree, 0.0f);
		CManagement::GetInstance()->Find_Object(L"Layer_Spider", id - NPC_START)->SetObjectAnimState(my_packet->state);
		//if (my_packet->monsterType == 2)
		//	my_packet->monsterType = 3;
		dynamic_cast<Spider*>(CManagement::GetInstance()->Find_Object(L"Layer_Spider",id - NPC_START))->SetTexture((SpiderTex)my_packet->monsterType);
	}
	}
}

void CNetwork::Free()
{
}