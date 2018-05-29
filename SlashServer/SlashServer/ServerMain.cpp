#define WIN32_LEAN_AND_MEAN  
#define INITGUID

#include <WinSock2.h>
#include <windows.h>   // include important windows stuff
#include <windowsx.h>
#include <thread>
#include <vector>
#include <iostream>
#include <mutex>

#include"Functor.h"
#pragma comment (lib, "ws2_32.lib")

#include "protocol.h"

using namespace std;

mutex p_lock;

struct EXOver {
	WSAOVERLAPPED wsaover;
	bool is_recv;
	WSABUF wsabuf;
	char io_buf[MAX_BUFF_SIZE];
};

struct CLIENT {
	SOCKET s;
	bool in_use;
	float x, y, z;
	XMFLOAT3 LookVector;
	float RotateNum;
	BYTE cur_state;
	BYTE pre_state;
	XMFLOAT3 cur_move;
	XMFLOAT3 pre_move;
	BoundingOrientedBox xmOOBB;
	BoundingOrientedBox xmOOBBTransformed;

	EXOver exover;
	int packet_size;
	int prev_size;
	char prev_packet[MAX_PACKET_SIZE];
};

HANDLE g_iocp; // IOCP 전역변수 객체
CLIENT g_clients[MAX_USER];

XMFLOAT3 GetSlideVector(CLIENT& Obj, CLIENT& CollObj)
{
	XMFLOAT3 Center = XMFLOAT3{ CollObj.x, CollObj.y, CollObj.z };
	XMFLOAT3 Player = XMFLOAT3{ Obj.x, Obj.y, Obj.z };
	XMFLOAT3 dirVector = Vector3::Subtract(Player, Center);   // 충돌 객체에서 플레이어로 가는 벡터
	XMFLOAT3 MovingRefletVector = XMFLOAT3{ 0, 0, 0 };

	float tanceta = dirVector.z / dirVector.x;
	float ceta = atan(tanceta) * 57.3248f;

	float extenttanceta = CollObj.xmOOBB.Extents.z / CollObj.xmOOBB.Extents.x;
	float extentceta = atan(extenttanceta) * 57.3248f;

	dirVector = Vector3::Normalize(dirVector);

	if (Player.x > Center.x && 0 < ceta && ceta< extentceta)
		MovingRefletVector = XMFLOAT3(1, 0, 0); // Right
	if (Player.z > Center.z && extentceta <ceta && ceta < 90)
		MovingRefletVector = XMFLOAT3(0, 0, 1); // Top
	if (Player.z > Center.z && -90 < ceta && ceta < -extentceta)
		MovingRefletVector = XMFLOAT3(0, 0, 1); // Top
	if (Player.x < Center.x && -extentceta < ceta && ceta < 0)
		MovingRefletVector = XMFLOAT3(-1, 0, 0); // Left
	if (Player.x < Center.x && 0 < ceta && ceta < extentceta)
		MovingRefletVector = XMFLOAT3(-1, 0, 0); // Left
	if (Player.z < Center.z && extentceta < ceta && ceta < 90)
		MovingRefletVector = XMFLOAT3(0, 0, -1); // Bottom
	if (Player.z < Center.z && -90 < ceta && ceta < -extentceta)
		MovingRefletVector = XMFLOAT3(0, 0, -1); // Bottom
	if (Player.x > Center.x && -extentceta < ceta && ceta < 0)
		MovingRefletVector = XMFLOAT3(1, 0, 0); // Right

	return MovingRefletVector;
}

void error_display(const char* msg, int err_no) {
	WCHAR *lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, err_no,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	cout << msg;
	wcout << L"에러 " << lpMsgBuf << endl;
	LocalFree(lpMsgBuf);
}
void Initialize()
{
	wcout.imbue(locale("korean"));
	//wcout << L"한글 메세지 출력!\n";
	cout << "Slash 서버 생성 완료" << endl;

	for (auto &cl : g_clients) {
		cl.in_use = false;
		cl.exover.is_recv = true;
		cl.exover.wsabuf.buf = cl.exover.io_buf;
		cl.exover.wsabuf.len = sizeof(cl.exover.io_buf);
		cl.packet_size = 0;
		cl.prev_size = 0;
		cl.LookVector = {};
		cl.cur_state = IDLE;
		cl.RotateNum = 0;
		cl.cur_move = {};
		cl.pre_move = {};
	}

	WSADATA   wsadata;
	WSAStartup(MAKEWORD(2, 2), &wsadata);

	g_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
}

void SendPacket(int cl, void *packet)
{
	EXOver *o = new EXOver;
	char *p = reinterpret_cast<char *>(packet);
	memcpy(o->io_buf, packet, p[0]);
	o->is_recv = false;
	o->wsabuf.buf = o->io_buf;
	o->wsabuf.len = p[0];
	ZeroMemory(&o->wsaover, sizeof(WSAOVERLAPPED));

	int ret = WSASend(g_clients[cl].s, &o->wsabuf, 1, NULL, 0, &o->wsaover, NULL);
	if (0 != ret) {
		int err_no = WSAGetLastError();
		if (WSA_IO_PENDING != err_no) // WSA_IO_PENDING 뭐냐? 샌드가 계속 실행되고 있다. IOCP는 WSA_IO_PENDING 에러를 리턴한다.
			error_display("Error in SendPacket:", err_no);
	}

	//printf("SendPacket to Client [ %d ] Type [ %d ] Size [ %d ]\n", cl, (int)p[1], (int)p[0]);
}

void DisconnectPlayer(int cl)
{
	closesocket(g_clients[cl].s); // closesocket 먼저 해야함 // 멀티 쓰레드라서 
	g_clients[cl].in_use = false;
	printf("%d번 플레이어 접속종료.\n", cl);

	sc_packet_remove_player p;
	p.id = cl;
	p.size = sizeof(p);
	p.type = SC_REMOVE_PLAYER;

	for (int i = 0; i < MAX_USER; ++i)
		if (true == g_clients[i].in_use)
			SendPacket(i, &p);

}

void ProcessPacket(int cl, char *packet)
{
	switch (packet[1])
	{
	case CS_STOP:
	{
		//cout << cl << g_clients[cl].LookVector.x << " " << g_clients[cl].LookVector.y << " " << g_clients[cl].LookVector.z << endl;

		g_clients[cl].cur_state = IDLE;
		sc_packet_state sp;
		sp.id = cl;
		sp.size = sizeof(sc_packet_state);
		sp.type = SC_STATE;
		sp.state = g_clients[cl].cur_state;
		for (int i = 0; i < MAX_USER; ++i)
			if (true == g_clients[i].in_use)
				SendPacket(i, &sp);
		break;
	}
	case CS_ATTACK:
	{
		if (ATTACK1 == g_clients[cl].pre_state)
			g_clients[cl].cur_state = ATTACK2;
		else
			g_clients[cl].cur_state = ATTACK1;
		g_clients[cl].pre_state = g_clients[cl].cur_state;

		sc_packet_state sp;
		sp.id = cl;
		sp.size = sizeof(sc_packet_state);
		sp.type = SC_STATE;
		sp.state = g_clients[cl].cur_state;
		for (int i = 0; i < MAX_USER; ++i)
			if (true == g_clients[i].in_use)
				SendPacket(i, &sp);
		break;
	}
	default: // 이동 패킷
	{
		g_clients[cl].cur_state = WALK;

		cs_packet_dir *p = reinterpret_cast<cs_packet_dir *>(packet);

		XMFLOAT3 xmf3Shift = p->Shift;

		float Temp_x = g_clients[cl].x + xmf3Shift.x;
		float Temp_y = g_clients[cl].y + xmf3Shift.y;
		float Temp_z = g_clients[cl].z + xmf3Shift.z;

		// 1행 라이트 (VECTOR3::crossvector)
		// 2행 룩(-)
		// 3행 업(010)

		XMFLOAT3 playerLook = XMFLOAT3(-(p->World._21), -(p->World._22), -(p->World._23));
		g_clients[cl].LookVector = playerLook;

		XMFLOAT4X4 World = {};
		XMFLOAT4X4 Temp = p->World;
		//XMFLOAT3 UpVector = { 0.f, 1.f, 0.f };
		//XMFLOAT3 RightVector = Vector3::CrossProduct(g_clients[cl].LookVector, UpVector, false);
		XMFLOAT3 SlideVector = { 0.f, 0.f, 0.f };


		//World._11 = RightVector.x, World._12 = RightVector.y, World._13 = RightVector.z, World._14 = 0;
		//World._21 = g_clients[cl].LookVector.x, World._22 = g_clients[cl].LookVector.y, World._23 = g_clients[cl].LookVector.z, World._24 = 0;
		//World._31 = UpVector.x, World._32 = UpVector.y, World._33 = UpVector.z, World._34 = 0;
		//World._41 = Temp_x, World._42 = Temp_y, World._43 = Temp_z, World._44 = 1;

		g_clients[cl].xmOOBBTransformed.Transform(g_clients[cl].xmOOBB, XMLoadFloat4x4(&(Temp)));
		XMStoreFloat4(&g_clients[cl].xmOOBBTransformed.Orientation, XMQuaternionNormalize(XMLoadFloat4(&g_clients[cl].xmOOBBTransformed.Orientation)));

		bool IsCollision = false;
		
		if (p->type != CS_COLLSION)
		{
			for (int i = 0; i < MAX_USER; ++i)
			{
				if (true == g_clients[i].in_use)
				{
					if (i != cl)
					{
						p_lock.lock();
						g_clients[cl].xmOOBB.Extents.x = 1.f;
						g_clients[cl].xmOOBB.Extents.y = 1.f;
						g_clients[cl].xmOOBB.Extents.z = 1.f;
						g_clients[i].xmOOBB.Extents.x = 1.f;
						g_clients[i].xmOOBB.Extents.y = 1.f;
						g_clients[i].xmOOBB.Extents.z = 1.f;
						if (g_clients[cl].xmOOBB.Contains(g_clients[i].xmOOBB))
						{
							SlideVector = GetSlideVector(g_clients[cl], g_clients[i]);
							float cosCeta = Vector3::DotProduct(SlideVector, xmf3Shift);
							if (cosCeta < 0)
							{
								auto result = Vector3::MultiplyScalr(SlideVector, Vector3::DotProduct(xmf3Shift, SlideVector));
								//if(!Vector3::IsEqual(Vector3::Normalize(m_MovingRefletVector), Vector3::Normalize(xmf3Shift)))
								XMFLOAT3 sub_xmf3Shift = Vector3::Subtract(xmf3Shift, result);

								g_clients[cl].x += sub_xmf3Shift.x, g_clients[cl].y += sub_xmf3Shift.y, g_clients[cl].z += sub_xmf3Shift.z;
								IsCollision = true;
								p_lock.unlock();
								break; // 먼저 충돌한 객체만 처리
							}
						}
						p_lock.unlock();
					}
				}
			}
		}

		if (!IsCollision)
		{
			g_clients[cl].x = Temp_x, g_clients[cl].y = Temp_y, g_clients[cl].z = Temp_z;
		}

		sc_packet_pos sp_pos;
		sp_pos.id = cl;
		sp_pos.size = sizeof(sc_packet_pos);
		sp_pos.type = SC_POS;
		sp_pos.x = g_clients[cl].x;
		sp_pos.y = g_clients[cl].y;
		sp_pos.z = g_clients[cl].z;

		//cout << xmf3Shift.x << "\t" << xmf3Shift.y << "\t" << xmf3Shift.z << endl;
		for (int i = 0; i < MAX_USER; ++i)
			if (true == g_clients[i].in_use)
				SendPacket(i, &sp_pos);

		playerLook = Vector3::Normalize(playerLook);
		XMFLOAT3 n_xmf3Shift = Vector3::Normalize(xmf3Shift);
		XMFLOAT3 crossVector = Vector3::CrossProduct(n_xmf3Shift, playerLook, true);

		float dotproduct = Vector3::DotProduct(n_xmf3Shift, playerLook);
		float xmf3ShiftLength = Vector3::Length(n_xmf3Shift);
		float xmf3PlayerLooklength = Vector3::Length(playerLook);

		float cosCeta = dotproduct / xmf3ShiftLength * xmf3PlayerLooklength;

		float ceta = acos(cosCeta); // 현재 각도

		ceta = ceta * fDegree;

		//cout << cl << " " << playerLook.x << " " << playerLook.y << " " << playerLook.z << endl;

		if (ceta > 8.0f)
		{
			//cout << ceta << endl;
			g_clients[cl].cur_move = xmf3Shift;

			if (g_clients[cl].cur_move.x == g_clients[cl].pre_move.x &&
				g_clients[cl].cur_move.y == g_clients[cl].pre_move.y &&
				g_clients[cl].cur_move.z == g_clients[cl].pre_move.z)
			{
				return;
			}

			//cout << ceta << endl;
			sc_packet_rotate sp_rotate;
			sp_rotate.id = cl;
			sp_rotate.size = sizeof(sc_packet_rotate);
			sp_rotate.type = SC_ROTATE;

			if (crossVector.y > 0)
			{
				sp_rotate.IsClockWise = true;
				g_clients[cl].RotateNum--;
			}
			else
			{
				sp_rotate.IsClockWise = false;
				g_clients[cl].RotateNum++;
			}
			for (int i = 0; i < MAX_USER; ++i)
				if (true == g_clients[i].in_use)
				{
					SendPacket(i, &sp_rotate);
				}
		}
		else
		{
			g_clients[cl].pre_move = xmf3Shift;
		}

		break;
	}
	}
}



void WorkerThread()
{
	while (true) {
		unsigned long data_size;
		unsigned long long key; // 64비트 모드에서는 long long으로 32비트에서는 long으로
		WSAOVERLAPPED *p_over;

		BOOL is_success = GetQueuedCompletionStatus(g_iocp,
			&data_size, &key, &p_over, INFINITE);

		//printf("GQCS from client [ %d ] with size [ %d ]\n", key, data_size);

		// 에러 처리
		if (0 == is_success) {
			//printf("Error in GQCS key[ %d ]\n", key);
			DisconnectPlayer(key);
			continue;
		}
		// 접속종료 처리
		if (0 == data_size) {
			DisconnectPlayer(key);
			continue;
		}

		// Send/Recv 처리
		EXOver *o = reinterpret_cast<EXOver *>(p_over);
		if (true == o->is_recv) {
			int r_size = data_size;
			char *ptr = o->io_buf;
			while (0 < r_size) {
				if (0 == g_clients[key].packet_size)
					g_clients[key].packet_size = ptr[0];
				int remain = g_clients[key].packet_size - g_clients[key].prev_size; // 받아야하는 패킷 총 사이즈 - 이전에 받은 값
				if (remain <= r_size) { // 패킷을 만들 수 있을 때
					memcpy(g_clients[key].prev_packet + g_clients[key].prev_size,
						ptr, remain);
					ProcessPacket(static_cast<int>(key), g_clients[key].prev_packet);
					r_size -= remain;
					ptr += remain;
					g_clients[key].packet_size = 0;
					g_clients[key].prev_size = 0;
				}
				else {// 패킷을 만들 수 없을 때
					memcpy(g_clients[key].prev_packet + g_clients[key].prev_size,
						ptr,
						r_size);
					g_clients[key].prev_size += r_size;
					r_size -= r_size;
					ptr += r_size;
				}
			}
			unsigned long rflag = 0;
			ZeroMemory(&o->wsaover, sizeof(WSAOVERLAPPED)); // 재사용할거니까 초기화해줘야 한다.
			WSARecv(g_clients[key].s, &o->wsabuf, 1, NULL,
				&rflag, &o->wsaover, NULL);
		}
		else {
			delete o;
		}
	}
}

void AcceptThread()
{
	auto g_socket = WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED); // 마지막 인자에 Overlapped 넣어줘야함

	SOCKADDR_IN bind_addr;
	ZeroMemory(&bind_addr, sizeof(SOCKADDR_IN));
	bind_addr.sin_family = AF_INET;
	bind_addr.sin_port = htons(MY_SERVER_PORT);
	bind_addr.sin_addr.s_addr = INADDR_ANY; // 아무 주소나 다 받겠다.

	::bind(g_socket, reinterpret_cast<sockaddr *>(&bind_addr),
		sizeof(SOCKADDR)); // 땡땡 안쓰면 C++11의 bind라는 전혀 다른 함수가 호출됨
	listen(g_socket, 1000); // 인자 2번째는 백로그 // 최대 몇개까지 기다리게 할 것인가
	while (true) {
		SOCKADDR_IN c_addr;
		ZeroMemory(&c_addr, sizeof(SOCKADDR_IN));
		c_addr.sin_family = AF_INET;
		c_addr.sin_port = htons(MY_SERVER_PORT);
		c_addr.sin_addr.s_addr = INADDR_ANY;
		int c_length = sizeof(SOCKADDR_IN);

		auto new_socket = WSAAccept(g_socket,
			reinterpret_cast<sockaddr *>(&c_addr),
			&c_length, NULL, NULL);
		cout << "새로운 클라이언트 접속\n";
		int new_key = -1;
		for (int i = 0; i < MAX_USER; ++i)
			if (false == g_clients[i].in_use) {
				new_key = i;
				break;
			}
		if (-1 == new_key) {
			cout << "MAX USER EXCEEDED!!!" << endl;
			continue;
		}
		printf("%d번 플레이어 접속.\n", new_key);
		g_clients[new_key].s = new_socket;
		g_clients[new_key].x = 0;
		g_clients[new_key].y = 0;
		g_clients[new_key].z = 5;
		ZeroMemory(&g_clients[new_key].exover.wsaover, sizeof(WSAOVERLAPPED)); // 리시브할때마다 클리어 해줘야 한다.

		CreateIoCompletionPort(reinterpret_cast<HANDLE>(new_socket),
			g_iocp, new_key, 0);
		g_clients[new_key].in_use = true; // 이거 위치 여기여야만 한다는데 왜그런지 모르겠음 // 멀티쓰레드 떄문이라는데....
		unsigned long flag = 0;
		int ret = WSARecv(new_socket, &g_clients[new_key].exover.wsabuf, 1,
			NULL, &flag, &g_clients[new_key].exover.wsaover, NULL);
		if (0 != ret) {
			int err_no = WSAGetLastError();
			if (WSA_IO_PENDING != err_no)
				error_display("Recv in AcceptThread", err_no);
		}

		sc_packet_put_player p;
		p.id = new_key;
		p.size = sizeof(sc_packet_put_player);
		p.type = SC_PUT_PLAYER;
		p.x = g_clients[new_key].x;
		p.y = g_clients[new_key].y;
		p.z = g_clients[new_key].z;
		p.RotateNum = g_clients[new_key].RotateNum;

		// 나의 접속을 모든 플레이어에게 알림
		for (int i = 0; i < MAX_USER; ++i) {
			if (true == g_clients[i].in_use)
				SendPacket(i, &p);
		}
		// 접속중인 다른 플레이어 정보를 전송
		for (int i = 0; i < MAX_USER; ++i) {
			if (true == g_clients[i].in_use)
				if (i != new_key) {
					p.id = i;
					p.x = g_clients[i].x;
					p.y = g_clients[i].y;
					p.z = g_clients[i].z;
					p.RotateNum = g_clients[i].RotateNum;
					SendPacket(new_key, &p);
				}
		}
	}
}

void TimerThread()
{
	// PPT보면서 구현해야 함
}

int main()
{
	vector <thread> all_threads;
	Initialize();

	for (int i = 0; i < 4; ++i)
		all_threads.push_back(thread{ WorkerThread });
	all_threads.push_back(thread{ AcceptThread });

	for (auto &th : all_threads) th.join(); // 레퍼런스로 받아야함
	WSACleanup();
}


//if (g_clients[cl].isPlayerViewMode)
//{
//   if (p->type == CS_DIR_FORWARD)
//      p->type = CS_DIR_BACKWARD;
//   else if (p->type == CS_DIR_BACKWARD)
//      p->type = CS_DIR_FORWARD;
//   else if (p->type == CS_DIR_LEFT)
//      p->type = CS_DIR_RIGHT;
//   else if (p->type == CS_DIR_RIGHT)
//      p->type = CS_DIR_LEFT;
//}

//switch (p->type) {
//case CS_DIR_FORWARD:
//   g_clients[cl].z -= 0.05;
//   //if (0 > g_clients[cl].y) g_clients[cl].y = 0;
//   break;
//case CS_DIR_BACKWARD:
//   g_clients[cl].z += 0.05;
//   //if (BOARD_HEIGHT <= g_clients[cl].y) g_clients[cl].y = BOARD_HEIGHT - 1;
//   break;
//case CS_DIR_LEFT:
//   g_clients[cl].x += 0.05;
//   //if (0 > g_clients[cl].x) g_clients[cl].x = 0;
//   break;
//case CS_DIR_RIGHT:
//   g_clients[cl].x -= 0.05;
//   //if (BOARD_WIDTH <= g_clients[cl].x) g_clients[cl].x = BOARD_WIDTH - 1;
//   break;
//case CS_PLAYER_VEIW_MDOE:
//   g_clients[cl].isPlayerViewMode = true;
//   break;
//case CS_FREE_VEIW_MDOE:
//   g_clients[cl].isPlayerViewMode = false;
//   break;
//default: printf("Unknown Protocol from Client[ %d ]\n", cl);
//   return;
//}