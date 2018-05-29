#include "stdafx.h"
#include "Network.h"
#include "GameObject.h"
#include "Management.h"
#include "TestScene.h"
#include "Player.h"
#include "Skeleton.h"
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
	ServerAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

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

	XMFLOAT3 xmf3Shift = XMFLOAT3(0, 0, 0);
	XMFLOAT3	m_xmf3Height = XMFLOAT3(0.0f, 0.0f, 1.0f);
	XMFLOAT3	m_xmf3Width = XMFLOAT3(1.0f, 0.0f, 0.0f);

	my_packet->type = keyInput;

	if (keyInput & CS_DIR_FORWARD) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Height, fMoveSpeed);
	if (keyInput & CS_DIR_BACKWARD) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Height, -fMoveSpeed);
	if (keyInput & CS_DIR_RIGHT) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Width, fMoveSpeed);
	if (keyInput & CS_DIR_LEFT) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Width, -fMoveSpeed);

	// m_MovingRefletVector 이게 갱신되어야 할거 같은데..
	float cosCeta = Vector3::DotProduct(CManagement::GetInstance()->Find_Object(L"Layer_Player", 0)->m_MovingRefletVector, xmf3Shift);
	if (cosCeta < 0)
	{
		xmf3Shift = Vector3::Subtract(xmf3Shift, Vector3::MultiplyScalr(CManagement::GetInstance()->Find_Object(L"Layer_Player", 0)->m_MovingRefletVector,
			Vector3::DotProduct(xmf3Shift, CManagement::GetInstance()->Find_Object(L"Layer_Player", 0)->m_MovingRefletVector)));
		keyInput |= CS_COLLSION;
		my_packet->type = keyInput;
	}
	my_packet->Shift = xmf3Shift;
	my_packet->World = world;
	int ret = WSASend(mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);

	//cout << my_packet->look_x << " " << my_packet->look_y << " " << my_packet->look_z << endl;

	if (ret) {
		int error_code = WSAGetLastError();
		printf("Error while sending packet [%d]", error_code);
	}
}

void CNetwork::SendAttackPacket(void)
{
	cs_packet_attack *my_packet = reinterpret_cast<cs_packet_attack *>(send_buffer);
	my_packet->size = sizeof(cs_packet_attack);
	send_wsabuf.len = sizeof(cs_packet_attack);
	DWORD iobyte;

	my_packet->type = CS_ATTACK;

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
			CManagement::GetInstance()->Find_Object(L"Layer_Player", 0)->SetPosition(my_packet->x, my_packet->y, my_packet->z);
		}
		else
		{
			CManagement::GetInstance()->Find_Object(L"Layer_Skeleton", id)->m_bIsConnected = true;
			CManagement::GetInstance()->Find_Object(L"Layer_Skeleton", id)->SetPosition(my_packet->x, my_packet->y, my_packet->z);
			CManagement::GetInstance()->Find_Object(L"Layer_Skeleton", id)->Rotate(0.0f, fRotateSpeed * my_packet->RotateNum
				/** CGameTimer_Manager::GetInstance()->Get_GameTimer(L"Timer_FPS")->DeltaTime() * 20.f*/, 0.0f);
		}
		break;
	}
	case SC_POS:
	{
		sc_packet_pos * my_packet = reinterpret_cast<sc_packet_pos *>(ptr);
		int id = my_packet->id;
		if (myid == id)
		{
			dynamic_cast<Player*>(CManagement::GetInstance()->Find_Object(L"Layer_Player", 0))->SetPosition(my_packet->x, my_packet->y, my_packet->z);
			//dynamic_cast<Player*>(CManagement::GetInstance()->Find_Object(L"Layer_Player", 0))->SetCurState(WALK);
			//if (dynamic_cast<Player*>(CManagement::GetInstance()->Find_Object(L"Layer_Player", 0))->GetCurState()
			//	!= dynamic_cast<Player*>(CManagement::GetInstance()->Find_Object(L"Layer_Player", 0))->GetPreState())
			{
				dynamic_cast<Player*>(CManagement::GetInstance()->Find_Object(L"Layer_Player", 0))->SetObjectAnimState(WALK);
				//dynamic_cast<Player*>(CManagement::GetInstance()->Find_Object(L"Layer_Player", 0))->SetPreState(WALK);
			}

		}
		else
		{
			dynamic_cast<CSkeleton*>(CManagement::GetInstance()->Find_Object(L"Layer_Skeleton", id))->SetPosition(my_packet->x, my_packet->y, my_packet->z);
			//dynamic_cast<CSkeleton*>(CManagement::GetInstance()->Find_Object(L"Layer_Skeleton", id))->SetCurState(WALK);
			//if (dynamic_cast<CSkeleton*>(CManagement::GetInstance()->Find_Object(L"Layer_Skeleton", id))->GetCurState()
			//	!= dynamic_cast<CSkeleton*>(CManagement::GetInstance()->Find_Object(L"Layer_Skeleton", id))->GetPreState())
			{
				dynamic_cast<CSkeleton*>(CManagement::GetInstance()->Find_Object(L"Layer_Skeleton", id))->SetObjectAnimState(WALK);
				//dynamic_cast<CSkeleton*>(CManagement::GetInstance()->Find_Object(L"Layer_Skeleton", id))->SetPreState(WALK);
			}
		}
		break;
	}
	case SC_ROTATE:
	{
		sc_packet_rotate * my_packet = reinterpret_cast<sc_packet_rotate *>(ptr);
		int id = my_packet->id;
		if (myid == id)
		{
			if (my_packet->IsClockWise)
				dynamic_cast<Player*>(CManagement::GetInstance()->Find_Object(L"Layer_Player", 0))->Rotate(0.0f, -fRotateSpeed
					/** CGameTimer_Manager::GetInstance()->Get_GameTimer(L"Timer_FPS")->DeltaTime() * 20.f*/, 0.0f);
			else
				dynamic_cast<Player*>(CManagement::GetInstance()->Find_Object(L"Layer_Player", 0))->Rotate(0.0f, fRotateSpeed
					/** CGameTimer_Manager::GetInstance()->Get_GameTimer(L"Timer_FPS")->DeltaTime() * 20.f*/, 0.0f);
		}
		else
		{
			if (my_packet->IsClockWise)
				dynamic_cast<CSkeleton*>(CManagement::GetInstance()->Find_Object(L"Layer_Skeleton", id))->Rotate(0.0f, -fRotateSpeed
					/** CGameTimer_Manager::GetInstance()->Get_GameTimer(L"Timer_FPS")->DeltaTime() * 20.f*/, 0.0f);
			else
				dynamic_cast<CSkeleton*>(CManagement::GetInstance()->Find_Object(L"Layer_Skeleton", id))->Rotate(0.0f, fRotateSpeed
					/** CGameTimer_Manager::GetInstance()->Get_GameTimer(L"Timer_FPS")->DeltaTime() * 20.f*/, 0.0f);
		}
		break;
	}
	case SC_STATE:
	{
		sc_packet_state * my_packet = reinterpret_cast<sc_packet_state *>(ptr);
		int id = my_packet->id;
		if (myid == id)
		{
			//dynamic_cast<Player*>(CManagement::GetInstance()->Find_Object(L"Layer_Player", 0))->SetCurState(my_packet->state);
			//if (dynamic_cast<Player*>(CManagement::GetInstance()->Find_Object(L"Layer_Player", 0))->GetCurState()
			//	!= dynamic_cast<Player*>(CManagement::GetInstance()->Find_Object(L"Layer_Player", 0))->GetPreState())
			{
				dynamic_cast<Player*>(CManagement::GetInstance()->Find_Object(L"Layer_Player", 0))->SetObjectAnimState(my_packet->state);
				//dynamic_cast<Player*>(CManagement::GetInstance()->Find_Object(L"Layer_Player", 0))->SetPreState(my_packet->state);
			}
		}
		else
		{
			//dynamic_cast<CSkeleton*>(CManagement::GetInstance()->Find_Object(L"Layer_Skeleton", id))->SetCurState(my_packet->state);
			//if (dynamic_cast<CSkeleton*>(CManagement::GetInstance()->Find_Object(L"Layer_Skeleton", id))->GetCurState()
			//	!= dynamic_cast<CSkeleton*>(CManagement::GetInstance()->Find_Object(L"Layer_Skeleton", id))->GetPreState())
			{
				dynamic_cast<CSkeleton*>(CManagement::GetInstance()->Find_Object(L"Layer_Skeleton", id))->SetObjectAnimState(my_packet->state);
				//dynamic_cast<CSkeleton*>(CManagement::GetInstance()->Find_Object(L"Layer_Skeleton", id))->SetPreState(my_packet->state);
			}
		}
		break;
	}
	case SC_REMOVE_PLAYER:
	{
		sc_packet_remove_player *my_packet = reinterpret_cast<sc_packet_remove_player *>(ptr);
		int id = my_packet->id;
		if (myid == id)
			CManagement::GetInstance()->Find_Object(L"Layer_Player", 0)->m_bIsConnected = false;
		else
			CManagement::GetInstance()->Find_Object(L"Layer_Skeleton", id)->m_bIsConnected = false;
		break;
	}
	}

	//switch (ptr[1])
	//{
	//case SC_PUT_PLAYER:
	//{
	//   sc_packet_put_player *my_packet = reinterpret_cast<sc_packet_put_player *>(ptr);
	//   int id = my_packet->id;
	//   if (first_time) {
	//      first_time = false;
	//      g_myid = id; // 처음에 아이디 보내줄때 나의 아이디 설정하는 듯
	//   }
	//   if (id == g_myid) {
	//      player.x = my_packet->x;
	//      player.y = my_packet->y;
	//      player.attr |= ATTR_VISIBLE;
	//   }
	//   else if (id < NPC_START) {
	//      skelaton[id].x = my_packet->x;
	//      skelaton[id].y = my_packet->y;
	//      skelaton[id].attr |= ATTR_VISIBLE;
	//   }
	//   else {
	//      npc[id - NPC_START].x = my_packet->x;
	//      npc[id - NPC_START].y = my_packet->y;
	//      npc[id - NPC_START].attr |= BOB_ATTR_VISIBLE;
	//   }*/
	//   break;
	//}
	//case SC_POS:
	//{
	//   sc_packet_pos *my_packet = reinterpret_cast<sc_packet_pos *>(ptr);
	//   int other_id = my_packet->id;
	//   if (other_id == g_myid) {
	//      g_left_x = my_packet->x - 4;
	//      g_top_y = my_packet->y - 4;
	//      player.x = my_packet->x;
	//      player.y = my_packet->y;
	//   }
	//   else if (other_id < NPC_START) {
	//      skelaton[other_id].x = my_packet->x;
	//      skelaton[other_id].y = my_packet->y;
	//   }
	//   /*else {
	//      npc[other_id - NPC_START].x = my_packet->x;
	//      npc[other_id - NPC_START].y = my_packet->y;
	//   }*/
	//   break;
	//}

	//case SC_REMOVE_PLAYER:
	//{
	//   sc_packet_remove_player *my_packet = reinterpret_cast<sc_packet_remove_player *>(ptr);
	//   int other_id = my_packet->id;
	//   if (other_id == g_myid) {
	//      player.Attr &= ~ATTR_VISIBLE;
	//   }
	//   else if (other_id < NPC_START) {
	//      skelaton[other_id].Attr &= ~ATTR_VISIBLE;
	//   }
	//   else {
	//      npc[other_id - NPC_START].attr &= ~BOB_ATTR_VISIBLE;
	//   }*/
	//   break;
	//}
	//case SC_CHAT:
	//{
	//   sc_packet_chat *my_packet = reinterpret_cast<sc_packet_chat *>(ptr);
	//   int other_id = my_packet->id;
	//   if (other_id == g_myid) {
	//      wcsncpy_s(player.message, my_packet->message, 256);
	//      player.message_time = GetTickCount();
	//   }
	//   else if (other_id < NPC_START) {
	//      wcsncpy_s(skelaton[other_id].message, my_packet->message, 256);
	//      skelaton[other_id].message_time = GetTickCount();
	//   }
	//   else {
	//      wcsncpy_s(npc[other_id - NPC_START].message, my_packet->message, 256);
	//      npc[other_id - NPC_START].message_time = GetTickCount();
	//   }
	//   break;

	//}*/
	//default:
	//   printf("Unknown PACKET type [%d]\n", ptr[1]);
	//}
}

void CNetwork::Free()
{
}