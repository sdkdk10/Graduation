#include "stdafx.h"
#include "Network.h"
#include "GameObject.h"
#include "Management.h"
#include "TestScene.h"

IMPLEMENT_SINGLETON(CNetwork)

CNetwork::CNetwork()
{
}

CNetwork::~CNetwork()
{
}

void CNetwork::InitSock(HWND MainWnd)
{
	WSADATA	wsadata;
	WSAStartup(MAKEWORD(2, 2), &wsadata);

	mysocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);

	SOCKADDR_IN ServerAddr;
	ZeroMemory(&ServerAddr, sizeof(SOCKADDR_IN));
	ServerAddr.sin_family = AF_INET;
	ServerAddr.sin_port = htons(MY_SERVER_PORT);
	ServerAddr.sin_addr.s_addr = inet_addr("192.168.82.126");

	int Result = WSAConnect(mysocket, (sockaddr *)&ServerAddr, sizeof(ServerAddr), NULL, NULL, NULL, NULL);

	if (Result)
		MessageBox(0, L"Connect Failed.", 0, 0);

	WSAAsyncSelect(mysocket, MainWnd, WM_SOCKET, FD_CLOSE | FD_READ);

	send_wsabuf.buf = send_buffer;
	send_wsabuf.len = BUF_SIZE;
	recv_wsabuf.buf = recv_buffer;
	recv_wsabuf.len = BUF_SIZE;

	//pObj = CManagement::GetInstance()->Find_Object(L"Layer_Player");
	int i = 0;
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

void CNetwork::SendDirKeyPacket(const DWORD& keyInput)
{
	cs_packet_dir *my_packet = reinterpret_cast<cs_packet_dir *>(send_buffer);
	my_packet->size = sizeof(my_packet);
	send_wsabuf.len = sizeof(my_packet);
	DWORD iobyte;
	my_packet->type = keyInput; // DWORD -> BYTE ��� ©�������� ��Ȯ�� �� // �̹� 4���� ��Ʈ������ �� �ִ� ���� // Ÿ���̶� Ű��ǲ ��ġ������
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
		serverid_to_objectvindex(id);
		dynamic_cast<CTestScene*>(CManagement::GetInstance()->Get_CurScene())->Put_Player(my_packet->x, my_packet->y, my_packet->z, id);
		break;
	}
	case SC_POS:
	{
		sc_packet_pos * my_packet = reinterpret_cast<sc_packet_pos *>(ptr);
		int id = my_packet->id;
		serverid_to_objectvindex(id);
		CManagement::GetInstance()->Find_Object(L"Layer_Player", id)->SetPosition(my_packet->x, my_packet->y, my_packet->z);
		break;
	}
	}

	//switch (ptr[1])
	//{
	//case SC_PUT_PLAYER:
	//{
	//	sc_packet_put_player *my_packet = reinterpret_cast<sc_packet_put_player *>(ptr);
	//	int id = my_packet->id;
	//	if (first_time) {
	//		first_time = false;
	//		g_myid = id; // ó���� ���̵� �����ٶ� ���� ���̵� �����ϴ� ��
	//	}
	//	if (id == g_myid) {
	//		player.x = my_packet->x;
	//		player.y = my_packet->y;
	//		player.attr |= ATTR_VISIBLE;
	//	}
	//	else if (id < NPC_START) {
	//		skelaton[id].x = my_packet->x;
	//		skelaton[id].y = my_packet->y;
	//		skelaton[id].attr |= ATTR_VISIBLE;
	//	}
	//	else {
	//		npc[id - NPC_START].x = my_packet->x;
	//		npc[id - NPC_START].y = my_packet->y;
	//		npc[id - NPC_START].attr |= BOB_ATTR_VISIBLE;
	//	}*/
	//	break;
	//}
	//case SC_POS:
	//{
	//	sc_packet_pos *my_packet = reinterpret_cast<sc_packet_pos *>(ptr);
	//	int other_id = my_packet->id;
	//	if (other_id == g_myid) {
	//		g_left_x = my_packet->x - 4;
	//		g_top_y = my_packet->y - 4;
	//		player.x = my_packet->x;
	//		player.y = my_packet->y;
	//	}
	//	else if (other_id < NPC_START) {
	//		skelaton[other_id].x = my_packet->x;
	//		skelaton[other_id].y = my_packet->y;
	//	}
	//	/*else {
	//		npc[other_id - NPC_START].x = my_packet->x;
	//		npc[other_id - NPC_START].y = my_packet->y;
	//	}*/
	//	break;
	//}

	//case SC_REMOVE_PLAYER:
	//{
	//	sc_packet_remove_player *my_packet = reinterpret_cast<sc_packet_remove_player *>(ptr);
	//	int other_id = my_packet->id;
	//	if (other_id == g_myid) {
	//		player.Attr &= ~ATTR_VISIBLE;
	//	}
	//	else if (other_id < NPC_START) {
	//		skelaton[other_id].Attr &= ~ATTR_VISIBLE;
	//	}
	//	else {
	//		npc[other_id - NPC_START].attr &= ~BOB_ATTR_VISIBLE;
	//	}*/
	//	break;
	//}
	//case SC_CHAT:
	//{
	//	sc_packet_chat *my_packet = reinterpret_cast<sc_packet_chat *>(ptr);
	//	int other_id = my_packet->id;
	//	if (other_id == g_myid) {
	//		wcsncpy_s(player.message, my_packet->message, 256);
	//		player.message_time = GetTickCount();
	//	}
	//	else if (other_id < NPC_START) {
	//		wcsncpy_s(skelaton[other_id].message, my_packet->message, 256);
	//		skelaton[other_id].message_time = GetTickCount();
	//	}
	//	else {
	//		wcsncpy_s(npc[other_id - NPC_START].message, my_packet->message, 256);
	//		npc[other_id - NPC_START].message_time = GetTickCount();
	//	}
	//	break;

	//}*/
	//default:
	//	printf("Unknown PACKET type [%d]\n", ptr[1]);
	//}
}

void CNetwork::Free()
{
}

void CNetwork::serverid_to_objectvindex(int& id)
{
	if (id == myid)
		id = MYPLAYERID;
	else if (id < myid)
		id++;
}