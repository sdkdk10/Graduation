#define WIN32_LEAN_AND_MEAN  
#define INITGUID

#include <WinSock2.h>
#include <windows.h>   // include important windows stuff
#include <windowsx.h>
#include <thread>
#include <vector>
#include <iostream>
#include <mutex>
#include <set>
#include <unordered_set>
#include <queue>

#include "Functor.h"
#include "../../Slash/Common/MathHelper.h"
#pragma comment (lib, "ws2_32.lib")

#include "protocol.h"

using namespace std;

mutex p_lock;

void SendObjectPos(int client, int object_id);
void SendObjectLook(int client, int object_id);
void SendObjectState(int client, int object_id);
void SendRemoveObject(int client, int object_id);
void SendPutObject(int client, int object_id);
void SendPacket(int cl, void *packet);

struct EXOver {
	WSAOVERLAPPED wsaover;
	char event_type;
	int event_target;
	WSABUF wsabuf;
	char io_buf[MAX_BUFF_SIZE];
};

struct CLIENT {
	SOCKET s;
	bool in_use;
	XMFLOAT4X4 World;
	float LookDegree;
	BYTE cur_state;
	BYTE pre_state;
	BoundingOrientedBox xmOOBB;
	BoundingOrientedBox xmOOBBTransformed;
	unordered_set<int> viewlist;
	mutex vlm;
	bool is_active;
	int Hp;
	int Dmg;

	EXOver exover;
	int packet_size;
	int prev_size;
	char prev_packet[MAX_PACKET_SIZE];
};

HANDLE g_iocp; // IOCP �������� ��ü
CLIENT g_clients[NUM_OF_NPC];

struct EVENT {
	unsigned int s_time;
	int type;
	int object; // ���� �����ϰ� �����ؾ� �ϴ°�
	int target;
};

class mycomparison
{
	bool reverse;
public:
	mycomparison() {}
	bool operator() (const EVENT lhs, const EVENT rhs) const
	{
		return (lhs.s_time > rhs.s_time);
	}
};

priority_queue <EVENT, vector<EVENT>, mycomparison> timer_queue;

void add_timer(int id, int type, unsigned int s_time, int target)
{
	timer_queue.push(EVENT{ s_time, type, id, target });
}

void TimerThread()
{
	while (true) {
		Sleep(10);
		while (false == timer_queue.empty()) {
			if (timer_queue.top().s_time >= GetTickCount()) break;
			EVENT ev = timer_queue.top();
			timer_queue.pop();
			EXOver *ex = new EXOver;
			ex->event_type = ev.type;
			ex->event_target = ev.target;
			PostQueuedCompletionStatus(g_iocp, 1, ev.object, &ex->wsaover);

		}
	}
}

bool CanSee(int a, int b)
{
	float dist_sq = (g_clients[a].World._41 - g_clients[b].World._41) * (g_clients[a].World._41 - g_clients[b].World._41)
		+ (g_clients[a].World._42 - g_clients[b].World._42) * (g_clients[a].World._42 - g_clients[b].World._42)
		+ (g_clients[a].World._43 - g_clients[b].World._43) * (g_clients[a].World._43 - g_clients[b].World._43);
	return (dist_sq <= VIEW_RADIUS * VIEW_RADIUS);
}

bool IsClose(int a, int b)
{
	float dist_sq = (g_clients[a].World._41 - g_clients[b].World._41) * (g_clients[a].World._41 - g_clients[b].World._41)
		+ (g_clients[a].World._42 - g_clients[b].World._42) * (g_clients[a].World._42 - g_clients[b].World._42)
		+ (g_clients[a].World._43 - g_clients[b].World._43) * (g_clients[a].World._43 - g_clients[b].World._43);
	return (dist_sq <= CLOSE_RADIUS * CLOSE_RADIUS);
}

bool IsInAgroRange(int a, int b)
{
	float dist_sq = (g_clients[a].World._41 - g_clients[b].World._41) * (g_clients[a].World._41 - g_clients[b].World._41)
		+ (g_clients[a].World._42 - g_clients[b].World._42) * (g_clients[a].World._42 - g_clients[b].World._42)
		+ (g_clients[a].World._43 - g_clients[b].World._43) * (g_clients[a].World._43 - g_clients[b].World._43);
	return (dist_sq <= AGRO_RADIUS * AGRO_RADIUS);
}

bool IsNPC(int id)
{
	return (id >= NPC_START) && (id < NUM_OF_NPC);
}

void WakeUpNPC(int id, int target)
{
	if (false == IsNPC(id)) return;

	if (g_clients[id].is_active) return;
	if (!IsInAgroRange(id, target))return;
	g_clients[id].is_active = true;

	add_timer(id, EVT_CHASE, GetTickCount(), target);
}

XMFLOAT3 GetSlideVector(CLIENT& Obj, CLIENT& CollObj)
{
	XMFLOAT3 Center = XMFLOAT3{ CollObj.World._41, CollObj.World._42, CollObj.World._43 };
	XMFLOAT3 Player = XMFLOAT3{ Obj.World._41, Obj.World._42, Obj.World._43 };
	XMFLOAT3 dirVector = Vector3::Subtract(Player, Center);   // �浹 ��ü���� �÷��̾�� ���� ����
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

void ChasingPlayer(int source, int target) {

	g_clients[source].cur_state = WALK;

	// �Ź̳��� �浹üũ�Ҷ� ��� �Ÿ� ����ϴ°�
	// �ƴϸ� ��ü �Ź̳��� �浹�ϴ��� �˻��ϴ°�

	// �Ź��� ���� �������ͽ��� �̵������� ����

	// �Ź̰� �÷��̾� ��ġ ��� �����̴°� ���� �̻���

	XMFLOAT3 MonsterLook = XMFLOAT3(g_clients[source].World._31, g_clients[source].World._32, g_clients[source].World._33);
	XMFLOAT3 MonsterPos = XMFLOAT3(g_clients[source].World._41, g_clients[source].World._42, g_clients[source].World._43);

	//cout << MonsterLook.x << " " << MonsterLook.y << " " << MonsterLook.z << endl;
	//cout << MonsterPos.x << " " << MonsterPos.y << " " << MonsterPos.z << endl;

	XMFLOAT3 playerPos = XMFLOAT3(g_clients[target].World._41, g_clients[target].World._42, g_clients[target].World._43);
	XMFLOAT3 Shift = Vector3::Normalize(MonsterLook);

	//cout << playerPos.x << " " << playerPos.y << " " << playerPos.z << endl;
	//cout << Shift.x << " " << Shift.y << " " << Shift.z << endl << endl;
	
	XMFLOAT3 dirVector = Vector3::Subtract(playerPos, MonsterPos);   // ��ü���� �÷��̾�� ���� ����
	
	dirVector = Vector3::Normalize(dirVector);
	
	//cout << dirVector.x << " " << dirVector.y << " " << dirVector.z << endl;

	XMFLOAT3 crossVector = Vector3::CrossProduct(Shift, dirVector, true);

	float dotproduct = Vector3::DotProduct(Shift, dirVector);
	float ShifttLength = Vector3::Length(Shift);
	float dirVectorLength = Vector3::Length(dirVector);
	
	float cosCeta = (dotproduct / ShifttLength * dirVectorLength);
	
	float ceta = acos(cosCeta);
	
	if (playerPos.z < MonsterPos.z)
		ceta = 360.f - ceta * 57.3248f;// +180.0f;
	else
		ceta = ceta * 57.3248f;

	cout << ceta << endl;

	XMFLOAT3 MovingReflectVector = XMFLOAT3(0, 0, 0); // ���߿� �浹�ϸ� ����������� �̵��ϱ�����

	bool IsRotated = false;

	if (ceta > 0.8f && ceta < 359.f)
	{
		if (crossVector.y > 0)
		{
			XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(0.f), XMConvertToRadians(fMonsterRotateDegree), XMConvertToRadians(0.f));
			g_clients[source].World = Matrix4x4::Multiply(mtxRotate, g_clients[source].World);
			g_clients[source].LookDegree += fMonsterRotateDegree;
		}
		else if (crossVector.y < 0)
		{
			XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(0.f), XMConvertToRadians(-fMonsterRotateDegree), XMConvertToRadians(0.f));
			g_clients[source].World = Matrix4x4::Multiply(mtxRotate, g_clients[source].World);
			g_clients[source].LookDegree += (360.f - fMonsterRotateDegree);
		}

		IsRotated = true;
	}

	if (g_clients[source].LookDegree >= 360.f)
		g_clients[source].LookDegree -= 360.f;

	//cout << g_clients[source].World._11 << " " << g_clients[source].World._12 << " " << g_clients[source].World._13 << " " << g_clients[source].World._14 << " " << endl;
	//cout << g_clients[source].World._21 << " " << g_clients[source].World._22 << " " << g_clients[source].World._23 << " " << g_clients[source].World._24 << " " << endl;
	//cout << g_clients[source].World._31 << " " << g_clients[source].World._32 << " " << g_clients[source].World._33 << " " << g_clients[source].World._34 << " " << endl;
	//cout << g_clients[source].World._41 << " " << g_clients[source].World._42 << " " << g_clients[source].World._43 << " " << g_clients[source].World._44 << " " << endl << endl;

	XMFLOAT3 movingVector = XMFLOAT3(dirVector.x * fMonsterMoveSpeed, dirVector.y * fMonsterMoveSpeed, dirVector.z * fMonsterMoveSpeed); // ����������

	//cout << movingVector.x << " " << movingVector.y << " " << movingVector.z << endl;

	unordered_set <int> old_vl;
	for (int id = 0; id < MAX_USER; ++id)
		if (true == g_clients[id].in_use)
			if (true == CanSee(id, source)) {
				old_vl.insert(id);
			}


	for (int i = NPC_START; i < NUM_OF_NPC; ++i)
	{
		if (false == g_clients[i].is_active) continue;
		if (false == IsClose(source, i)) continue;

		if (g_clients[source].xmOOBB.Contains(g_clients[i].xmOOBB))
		{
			MovingReflectVector = GetSlideVector(g_clients[source], g_clients[i]);
			break;
		}
	}

	auto result = Vector3::MultiplyScalr(MovingReflectVector, Vector3::DotProduct(movingVector, MovingReflectVector));
	movingVector = Vector3::Subtract(movingVector, result);

	//g_clients[source].World._41 += movingVector.x;
	//g_clients[source].World._42 += movingVector.y;
	//g_clients[source].World._43 += movingVector.z;

	XMFLOAT3 xmf3Position = Vector3::Add(MonsterPos, movingVector);
	g_clients[source].World._41 = xmf3Position.x;
	g_clients[source].World._42 = xmf3Position.y;
	g_clients[source].World._43 = xmf3Position.z;


	g_clients[source].xmOOBBTransformed.Transform(g_clients[source].xmOOBB, XMLoadFloat4x4(&g_clients[source].World));
	XMStoreFloat4(&g_clients[source].xmOOBBTransformed.Orientation, XMQuaternionNormalize(XMLoadFloat4(&g_clients[source].xmOOBBTransformed.Orientation)));

	unordered_set<int> new_vl;
	for (int i = 0; i < MAX_USER; ++i)
	{
		if (false == g_clients[i].in_use) continue;
		if (false == CanSee(i, source)) continue;

		new_vl.insert(i);
	}

	// PutObject
	for (auto id : new_vl)
	{
		g_clients[id].vlm.lock();
		if (0 == g_clients[id].viewlist.count(source))
		{
			g_clients[id].viewlist.insert(source);
			g_clients[id].vlm.unlock();
			SendPutObject(id, source);
		}
		else
		{
			g_clients[id].vlm.unlock();
			SendObjectPos(id, source);
			if (IsRotated)
				SendObjectLook(id, source);
		}
	}
	// RemoveObject
	for (auto id : old_vl)
	{
		if (0 == new_vl.count(id))
		{
			g_clients[id].vlm.lock();
			if (0 != g_clients[id].viewlist.count(source)) {
				g_clients[id].viewlist.erase(source);
				g_clients[id].vlm.unlock();
				SendRemoveObject(id, source);
			}
			else {
				g_clients[id].vlm.unlock();
			}
		}
	}

	if (!CanSee(source, target))
	{
		for (int i = 0; i < MAX_USER; ++i)
		{
			if (false == g_clients[i].in_use) continue;
			if (false == CanSee(i, source)) continue;
			if (IsInAgroRange(source, i))
			{
				add_timer(source, EVT_CHASE, GetTickCount() + 50, i);
				return;
			}
		}
		
		g_clients[source].cur_state = IDLE;
		g_clients[source].is_active = false;

		for (int i = 0; i < MAX_USER; ++i)
		{
			if (false == g_clients[i].in_use) continue;
			if (false == CanSee(i, source)) continue;
			SendObjectState(i, source);
		}
	}
	else if (IsClose(target, source))	 //�浹�� ������ ����� ������
	{
		add_timer(source, EVT_MONSTER_ATTACK, GetTickCount(), target);
		
			// ���� ������ ���·� �����ϰ� Ŭ��� ������ (�׳� Ŭ�󿡼� ��� ����ϴٰ� �Ǵ��ұ�?)
			// �׸��� ���� �� �÷��̾� ü�� ��´�.
	}
	else
	{
		add_timer(source, EVT_CHASE, GetTickCount() + 50, target);
	}


}

void PutNewPlayer(int new_key) {

	g_clients[new_key].vlm.lock();
	g_clients[new_key].viewlist.clear();
	g_clients[new_key].vlm.unlock();

	sc_packet_put_player p;
	p.id = new_key;
	p.size = sizeof(sc_packet_put_player);
	p.type = SC_PUT_PLAYER;
	p.posX = g_clients[new_key].World._41;
	p.posY = g_clients[new_key].World._42;
	p.posZ = g_clients[new_key].World._43;
	p.lookDegree = g_clients[new_key].LookDegree;
	p.state = IDLE;

	SendPacket(new_key, &p);

	// ���� ������ ��� �÷��̾�� �˸�
	for (int i = 0; i < MAX_USER; ++i)
	{
		if (false == g_clients[i].in_use) continue;
		if (false == CanSee(i, new_key)) continue;
		if (i == new_key) continue;

		SendPacket(i, &p);

		g_clients[i].vlm.lock();
		g_clients[i].viewlist.insert(new_key);
		g_clients[i].vlm.unlock();
	}
	// �������� �ٸ� �÷��̾� ������ ����
	for (int i = 0; i < MAX_USER; ++i) {
		if (false == g_clients[i].in_use) continue;
		if (false == CanSee(i, new_key)) continue;
		if (i == new_key) continue;

		p.id = i;
		p.posX = g_clients[i].World._41;
		p.posY = g_clients[i].World._42;
		p.posZ = g_clients[i].World._43;
		p.lookDegree = g_clients[i].LookDegree;
		p.state = g_clients[i].cur_state;

		SendPacket(new_key, &p);

		g_clients[new_key].vlm.lock();
		g_clients[new_key].viewlist.insert(i);
		g_clients[new_key].vlm.unlock();
	}
	for (int i = NPC_START; i < NUM_OF_NPC; i++)
	{
		if (false == CanSee(new_key, i)) continue;

		p.id = i;
		p.posX = g_clients[i].World._41;
		p.posY = g_clients[i].World._42;
		p.posZ = g_clients[i].World._43;
		p.lookDegree = g_clients[i].LookDegree;
		p.state = g_clients[i].cur_state;

		g_clients[new_key].vlm.lock();
		g_clients[new_key].viewlist.insert(i);
		g_clients[new_key].vlm.unlock();
		WakeUpNPC(i, new_key);
		SendPacket(new_key, &p);
		//cout << "���� : " << i << " ��° ���� ����" << endl;
	}

}

void MonsterAttack(int source, int target) {


	if (g_clients[source].cur_state == DEAD)
	{
		return;
	}
	else if (g_clients[target].cur_state == DEAD)
	{
		g_clients[source].cur_state = IDLE;

		for (int i = 0; i < MAX_USER; ++i)
		{
			if (false == g_clients[i].in_use) continue;
			if (false == CanSee(i, source)) continue;
			SendObjectState(i, source);
		}
		return;
	}
	else if (!IsClose(source, target))
	{
		add_timer(source, EVT_CHASE, GetTickCount(), target);
		return;
	}
	else
	{
		g_clients[source].cur_state = ATTACK1;

		for (int i = 0; i < MAX_USER; ++i)
		{
			if (false == g_clients[i].in_use) continue;
			if (false == CanSee(i, source)) continue;
			SendObjectState(i, source);
		}

		add_timer(source, EVT_DAMAGE, GetTickCount() + 500, target);
		if((g_clients[target].Hp - g_clients[source].Dmg) > 0)
			add_timer(source, EVT_MONSTER_ATTACK, GetTickCount() + 500, target);
	}
}

void PlayerAttack(int source) {

	for (int i = NPC_START; i < NUM_OF_NPC; ++i) // �丮��Ʈ�� �ִ� �ֵ� ���°� �� ��������..
	{
		if (false == g_clients[i].is_active) continue;
		if (false == IsClose(source, i)) continue;

		// ���Ͱ� �տ� �ִ°� ���⿡�� �Ǵ�

		add_timer(source, EVT_DAMAGE, GetTickCount() + 500, i);
	}

}

void ProcessDamage(int source, int target) {

	g_clients[target].Hp -= g_clients[source].Dmg;

	if ((g_clients[target].Hp) <= 0)
	{
		g_clients[target].cur_state = DEAD;

		for (int i = 0; i < MAX_USER; ++i)
		{
			if (false == g_clients[i].in_use) continue;
			if (false == CanSee(i, target)) continue;
			SendObjectState(i, target);
		}

		add_timer(target, EVT_RESPOWN, GetTickCount() + 10000, 0);
	}
}

void ProcessRespown(int source)
{
	g_clients[source].cur_state = IDLE;

	if (IsNPC(source))
	{
		// ���� ����ġ�� ������?
		// ������ �ִ� �÷��̾� �丮��Ʈ�� �־������
		// PutNewPlayer�� ����ϰ� PutNewMonster�ϸ� �ɵ�
		// ��׷� �Ÿ��� �ִ� �÷��̾� �˻��ؼ� �Ѿư�����
	}
	else
	{
		g_clients[source].Hp = 200;
		g_clients[source].World._41 = 15;
		g_clients[source].World._42 = 0;
		g_clients[source].World._43 = 0;
		PutNewPlayer(source);
	}
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
	wcout << L"���� " << lpMsgBuf << endl;
	LocalFree(lpMsgBuf);
}
void Initialize()
{
	wcout.imbue(locale("korean"));

	for (auto &cl : g_clients) {
		cl.in_use = false;
		cl.exover.event_type = EVT_RECV;
		cl.exover.wsabuf.buf = cl.exover.io_buf;
		cl.exover.wsabuf.len = sizeof(cl.exover.io_buf);
		cl.packet_size = 0;
		cl.prev_size = 0;
		cl.LookDegree = 0;
		cl.cur_state = IDLE;
		cl.is_active = false;
	}

	for (int i = 0; i < MAX_USER; ++i)
	{
		XMStoreFloat4x4(&g_clients[i].World, XMMatrixScaling(0.05f, 0.05f, 0.05f)*XMMatrixRotationX(1.7f)*XMMatrixRotationZ(3.14f)*XMMatrixTranslation(0.0f, 0.0f, 0.0f));
		g_clients[i].World._41 = 15, g_clients[i].World._42 = 0, g_clients[i].World._43 = 0; // �÷��̾� ��ġ �ʱ�ȭ
		g_clients[i].Hp = 200;
		g_clients[i].Dmg = 20;
	}
	for (int i = NPC_START; i < NUM_OF_NPC; ++i)
	{
		XMStoreFloat4x4(&g_clients[i].World, XMMatrixScaling(2.0f, 2.0f, 2.0f)*XMMatrixTranslation(0.0f, 0.0f, 0.0f)*XMMatrixTranslation(0.0f, 0.0f, 0.0f));
		g_clients[i].World._41 = 0, g_clients[i].World._42 = 0, g_clients[i].World._43 = (i - NPC_START); // �Ź� ��ġ �ʱ�ȭ
		g_clients[i].Hp = 100;
		g_clients[i].Dmg = 10;
	}

	WSADATA   wsadata;
	WSAStartup(MAKEWORD(2, 2), &wsadata);

	g_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);

	cout << "Slash ���� ���� �Ϸ�" << endl;
}

void SendPacket(int cl, void *packet)
{
	EXOver *o = new EXOver;
	char *p = reinterpret_cast<char *>(packet);
	memcpy(o->io_buf, packet, p[0]);
	o->event_type = EVT_SEND; 
	o->wsabuf.buf = o->io_buf;
	o->wsabuf.len = p[0];
	ZeroMemory(&o->wsaover, sizeof(WSAOVERLAPPED));

	int ret = WSASend(g_clients[cl].s, &o->wsabuf, 1, NULL, 0, &o->wsaover, NULL);
	if (0 != ret) {
		int err_no = WSAGetLastError();
		if (WSA_IO_PENDING != err_no) // WSA_IO_PENDING ����? ���尡 ��� ����ǰ� �ִ�. IOCP�� WSA_IO_PENDING ������ �����Ѵ�.
			error_display("Error in SendPacket:", err_no);
	}

	//printf("SendPacket to Client [ %d ] Type [ %d ] Size [ %d ]\n", cl, (int)p[1], (int)p[0]);
}

void DisconnectPlayer(int cl)
{
	closesocket(g_clients[cl].s); // closesocket ���� �ؾ��� // ��Ƽ ������� 
	g_clients[cl].in_use = false;
	printf("%d�� �÷��̾� ��������.\n", cl);

	sc_packet_remove_object p;
	p.ID = cl;
	p.SIZE = sizeof(p);
	p.TYPE = SC_REMOVE_OBJECT;

	g_clients[cl].vlm.lock();
	unordered_set <int> vl_copy = g_clients[cl].viewlist;
	g_clients[cl].vlm.unlock();
	g_clients[cl].viewlist.clear();

	for (int id : vl_copy) {
		if (true == IsNPC(id)) continue;
		g_clients[id].vlm.lock();
		if (g_clients[id].in_use == true) {
			if (0 != g_clients[id].viewlist.count(cl)) {
				g_clients[id].viewlist.erase(cl);
				g_clients[id].vlm.unlock();
				SendPacket(id, &p);
			}
		}
		else
			g_clients[id].vlm.unlock();
	}
	g_clients[cl].in_use = false;

}

void SendObjectPos(int client, int object_id)
{
	sc_packet_pos pos_p;
	pos_p.id = object_id;
	pos_p.size = sizeof(sc_packet_pos);
	pos_p.type = SC_POS;
	pos_p.posX = g_clients[object_id].World._41;
	pos_p.posY = g_clients[object_id].World._42;
	pos_p.posZ = g_clients[object_id].World._43;

	SendPacket(client, &pos_p);
}

void SendObjectLook(int client, int object_id)
{
	sc_packet_look_degree degree_p;
	degree_p.id = object_id;
	degree_p.size = sizeof(sc_packet_look_degree);
	degree_p.type = SC_ROTATE;
	degree_p.lookDegree = g_clients[object_id].LookDegree;

	SendPacket(client, &degree_p);
}

void SendObjectState(int client, int object_id)
{
	sc_packet_state state_p;
	state_p.id = object_id;
	state_p.size = sizeof(sc_packet_state);
	state_p.type = SC_STATE;
	state_p.state = g_clients[object_id].cur_state;

	SendPacket(client, &state_p);
}

void SendPutObject(int client, int object_id)
{
	sc_packet_put_player put_p;
	put_p.id = object_id;
	put_p.size = sizeof(sc_packet_put_player);
	put_p.type = SC_PUT_PLAYER;
	put_p.posX = g_clients[object_id].World._41;
	put_p.posY = g_clients[object_id].World._42;
	put_p.posZ = g_clients[object_id].World._43;
	put_p.lookDegree = g_clients[object_id].LookDegree;
	put_p.state = g_clients[object_id].cur_state;

	SendPacket(client, &put_p);
}

void SendRemoveObject(int client, int object_id)
{
	sc_packet_remove_object p;
	p.ID = object_id;
	p.SIZE = sizeof(sc_packet_remove_object);
	p.TYPE = SC_REMOVE_OBJECT;

	SendPacket(client, &p);
}

void ProcessPacket(int cl, char *packet)
{
	if (packet[1] <= MOVE_PACKET_END && packet[1] >= MOVE_PACKET_START)
	{
		// LEFT RIGHT �̵� �� �躤�� 3�� ��ȣ �ٸ�
		// UP DOWN �̵� �� ����Ʈ���� 3�� ��ȣ �ٸ�

		g_clients[cl].cur_state = WALK;

		XMFLOAT3 xmf3Shift{ 0.0f, 0.0f, 0.0f };

		cs_packet_dir *p = reinterpret_cast<cs_packet_dir *>(packet);

		if (p->type & CS_DIR_FORWARD) xmf3Shift = Vector3::Add(xmf3Shift, const_cast<XMFLOAT3&>(xmf3Height), fMoveSpeed);
		if (p->type & CS_DIR_BACKWARD) xmf3Shift = Vector3::Add(xmf3Shift, const_cast<XMFLOAT3&>(xmf3Height), -fMoveSpeed);
		if (p->type & CS_DIR_RIGHT) xmf3Shift = Vector3::Add(xmf3Shift, const_cast<XMFLOAT3&>(xmf3Width), fMoveSpeed);
		if (p->type & CS_DIR_LEFT) xmf3Shift = Vector3::Add(xmf3Shift, const_cast<XMFLOAT3&>(xmf3Width), -fMoveSpeed);

		XMFLOAT3 SlideVector{};

		bool IsRotated = false;

		XMFLOAT3 playerLook = XMFLOAT3(-g_clients[cl].World._21, -g_clients[cl].World._22, -g_clients[cl].World._23);
		playerLook = Vector3::Normalize(playerLook);
		XMFLOAT3 n_xmf3Shift = Vector3::Normalize(xmf3Shift);
		XMFLOAT3 crossVector = Vector3::CrossProduct(n_xmf3Shift, playerLook, true);

		float dotproduct = Vector3::DotProduct(n_xmf3Shift, playerLook);
		float xmf3ShiftLength = Vector3::Length(n_xmf3Shift);
		float xmf3PlayerLooklength = Vector3::Length(playerLook);

		float cosCeta = dotproduct / xmf3ShiftLength * xmf3PlayerLooklength;

		float ceta = acos(cosCeta); // ���� ����

		ceta = ceta * fDegree;

		//cout << ceta << endl;

		if (ceta > 8.0f)
		{
			if (crossVector.y > 0)
			{
				XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(0.f), XMConvertToRadians(0.f), XMConvertToRadians(-fRotateDegree));
				g_clients[cl].World = Matrix4x4::Multiply(mtxRotate, g_clients[cl].World);
				g_clients[cl].LookDegree += (360 - fRotateDegree);
			}
			else
			{
				XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(0.f), XMConvertToRadians(0.f), XMConvertToRadians(fRotateDegree));
				g_clients[cl].World = Matrix4x4::Multiply(mtxRotate, g_clients[cl].World);
				g_clients[cl].LookDegree += fRotateDegree;
			}
			if (g_clients[cl].LookDegree >= 360.f)
				g_clients[cl].LookDegree -= 360.f;
			IsRotated = true;
		}

		//cout << g_clients[cl].World._11 << " " << g_clients[cl].World._12 << " " << g_clients[cl].World._13 << " " << g_clients[cl].World._14 << " " << endl;
		//cout << g_clients[cl].World._21 << " " << g_clients[cl].World._22 << " " << g_clients[cl].World._23 << " " << g_clients[cl].World._24 << " " << endl;
		//cout << g_clients[cl].World._31 << " " << g_clients[cl].World._32 << " " << g_clients[cl].World._33 << " " << g_clients[cl].World._34 << " " << endl;
		//cout << g_clients[cl].World._41 << " " << g_clients[cl].World._42 << " " << g_clients[cl].World._43 << " " << g_clients[cl].World._44 << " " << endl << endl;


		g_clients[cl].xmOOBBTransformed.Transform(g_clients[cl].xmOOBB, XMLoadFloat4x4(&(g_clients[cl].World)));
		XMStoreFloat4(&g_clients[cl].xmOOBBTransformed.Orientation, XMQuaternionNormalize(XMLoadFloat4(&g_clients[cl].xmOOBBTransformed.Orientation)));

		bool IsCollision = false;

		for (int i = 0; i < MAX_USER; ++i)
		{
			if (false == g_clients[i].in_use) continue;
			if (false == IsClose(i, cl)) continue;
			if (i == cl) continue;

			p_lock.lock();
			g_clients[cl].xmOOBB.Extents.x = 1.f; // �̰� ����
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
					XMFLOAT3 sub_xmf3Shift = Vector3::Subtract(xmf3Shift, result);

					g_clients[cl].World._41 += sub_xmf3Shift.x, g_clients[cl].World._42 += sub_xmf3Shift.y, g_clients[cl].World._43 += sub_xmf3Shift.z;
					IsCollision = true;
					p_lock.unlock();
					break; // ���� �浹�� ��ü�� ó��
				}
			}
			p_lock.unlock();
		}

		if (!IsCollision)
			g_clients[cl].World._41 += xmf3Shift.x, g_clients[cl].World._42 += xmf3Shift.y, g_clients[cl].World._43 += xmf3Shift.z;

		sc_packet_pos sp_pos;
		sp_pos.id = cl;
		sp_pos.size = sizeof(sc_packet_pos);
		sp_pos.type = SC_POS;
		sp_pos.posX = g_clients[cl].World._41;
		sp_pos.posY = g_clients[cl].World._42;
		sp_pos.posZ = g_clients[cl].World._43;

		sc_packet_look_degree sp_rotate;
		sp_rotate.id = cl;
		sp_rotate.size = sizeof(sc_packet_look_degree);
		sp_rotate.type = SC_ROTATE;
		sp_rotate.lookDegree = g_clients[cl].LookDegree;

		//////////////

		unordered_set <int> new_view_list; // ���ο� �丮��Ʈ ����
		for (int i = 0; i < MAX_USER; ++i) {
			if (i == cl) continue;
			if (false == g_clients[i].in_use) continue;
			if (false == CanSee(cl, i)) continue;
			// ���ο� �丮��Ʈ (���� ����) // ���� �̵��ϰ� ���̴� ��� �ֵ�
			new_view_list.insert(i);
		}
		for (int i = NPC_START; i < NUM_OF_NPC; ++i) {
			if (false == CanSee(cl, i)) continue;
			new_view_list.insert(i);
		}

		SendPacket(cl, &sp_pos);
		if(IsRotated)
			SendPacket(cl, &sp_rotate);

		for (auto id : new_view_list) {
			g_clients[cl].vlm.lock();

			WakeUpNPC(id, cl);

			// ���� ���� �丮��Ʈ���� ������ // �� ���� ���Դ�
			if (0 == g_clients[cl].viewlist.count(id))
			{
				g_clients[cl].viewlist.insert(id);
				g_clients[cl].vlm.unlock();
				//cout << "�丮��Ʈ ��ġ : " << id << " ��° ���� ����" << endl;
				SendPutObject(cl, id);
			}
			else
				g_clients[cl].vlm.unlock();

			if (true == IsNPC(id)) continue;

			g_clients[id].vlm.lock();
			// �������� ���� ������? // �߰�
			if (0 == g_clients[id].viewlist.count(cl)) {
				g_clients[id].viewlist.insert(cl);
				g_clients[id].vlm.unlock();
				SendPutObject(id, cl);
			}
			// �������� ���� �־���? // ��ġ����
			else
			{
				g_clients[id].vlm.unlock();
				SendPacket(id, &sp_pos);
				if (IsRotated)
					SendPacket(id, &sp_rotate);
			}
		}
		// ���� ���� �丮��Ʈ�� �ִ� �ֵ�
		g_clients[cl].vlm.lock();
		unordered_set <int> old_v = g_clients[cl].viewlist;
		g_clients[cl].vlm.unlock();
		for (auto id : old_v) {
			if (0 == new_view_list.count(id)) {
				if (cl == id) continue;
				g_clients[cl].vlm.lock();
				g_clients[cl].viewlist.erase(id); // ��� �� ������� ���� ���� �����긮��Ʈ����� �ѹ��� ����°� ����
				g_clients[cl].vlm.unlock();
				SendRemoveObject(cl, id);

				if (true == IsNPC(id)) continue;
				g_clients[id].vlm.lock();
				// ���� ��쿡�� ����� ����.
				if (0 != g_clients[id].viewlist.count(cl)) {
					g_clients[id].viewlist.erase(cl);
					g_clients[id].vlm.unlock();
					SendRemoveObject(id, cl);
				}
				else {
					g_clients[id].vlm.unlock();
				}
			}
		}

		/////////////
	}
	else if(packet[1] == CS_STOP)
	{
		//cout << cl << g_clients[cl].LookVector.x << " " << g_clients[cl].LookVector.y << " " << g_clients[cl].LookVector.z << endl;

		g_clients[cl].cur_state = IDLE;
		sc_packet_state sp;
		sp.id = cl;
		sp.size = sizeof(sc_packet_state);
		sp.type = SC_STATE;
		sp.state = g_clients[cl].cur_state;
		for (int i = 0; i < MAX_USER; ++i)
		{
			if (false == g_clients[i].in_use) continue;
			if (false == CanSee(cl, i)) continue;
				SendPacket(i, &sp);
		}

		return;
	}
	else if (packet[1] == CS_ATTACK)
	{
		g_clients[cl].cur_state = ATTACK1;

		for (int i = 0; i < MAX_USER; ++i)
		{
			if (false == g_clients[i].in_use) continue;
			if (false == CanSee(i, cl)) continue;

			SendObjectState(i, cl);
		}

		add_timer(cl, EVT_PLAYER_ATTACK, GetTickCount() + iAttackDelay, 0);

		return;
	}
	else
	{
		cout << cl << " ProcessPacket Error" << endl;
		return;
	}
}

void WorkerThread()
{
	while (true) {
		unsigned long data_size;
		unsigned long long key; // 64��Ʈ ��忡���� long long���� 32��Ʈ������ long����
		WSAOVERLAPPED *p_over;

		BOOL is_success = GetQueuedCompletionStatus(g_iocp,
			&data_size, &key, &p_over, INFINITE);

		//printf("GQCS from client [ %d ] with size [ %d ]\n", key, data_size);

		// �������� ó��
		if (0 == data_size) {
			DisconnectPlayer(key);
			continue;
		}
		// ���� ó��
		if (0 == is_success) {
			//printf("Error in GQCS key[ %d ]\n", key);
			DisconnectPlayer(key);
			continue;
		}

		// Send/Recv ó��
		EXOver *o = reinterpret_cast<EXOver *>(p_over);
		if (EVT_RECV == o->event_type) {
			int r_size = data_size;
			char *ptr = o->io_buf;
			while (0 < r_size) {
				if (0 == g_clients[key].packet_size)
					g_clients[key].packet_size = ptr[0];
				int remain = g_clients[key].packet_size - g_clients[key].prev_size; // �޾ƾ��ϴ� ��Ŷ �� ������ - ������ ���� ��
				if (remain <= r_size) { // ��Ŷ�� ���� �� ���� ��
					memcpy(g_clients[key].prev_packet + g_clients[key].prev_size,
						ptr, remain);
					ProcessPacket(static_cast<int>(key), g_clients[key].prev_packet);
					r_size -= remain;
					ptr += remain;
					g_clients[key].packet_size = 0;
					g_clients[key].prev_size = 0;
				}
				else {// ��Ŷ�� ���� �� ���� ��
					memcpy(g_clients[key].prev_packet + g_clients[key].prev_size,
						ptr,
						r_size);
					g_clients[key].prev_size += r_size;
					r_size -= r_size;
					ptr += r_size;
				}
			}
			unsigned long rflag = 0;
			ZeroMemory(&o->wsaover, sizeof(WSAOVERLAPPED)); // �����ҰŴϱ� �ʱ�ȭ����� �Ѵ�.
			WSARecv(g_clients[key].s, &o->wsabuf, 1, NULL,
				&rflag, &o->wsaover, NULL);
		}
		else if (EVT_SEND == o->event_type)
		{
			delete o;
		}
		else if (EVT_CHASE == o->event_type)
		{
			EXOver *o = reinterpret_cast<EXOver *>(p_over);
			int player = o->event_target;
			ChasingPlayer(key, player);
			delete o;
		}
		else if (EVT_MONSTER_ATTACK == o->event_type)
		{
			EXOver *o = reinterpret_cast<EXOver *>(p_over);
			int target = o->event_target;
			MonsterAttack(key, target);
			delete o;
		}
		else if (EVT_PLAYER_ATTACK == o->event_type)
		{
			EXOver *o = reinterpret_cast<EXOver *>(p_over);
			PlayerAttack(key);
			delete o;
		}
		else if (EVT_DAMAGE == o->event_type)
		{
			EXOver *o = reinterpret_cast<EXOver *>(p_over);
			int target = o->event_target;
			ProcessDamage(key, target);
			delete o;
		}
		else if (EVT_RESPOWN == o->event_type)
		{
			EXOver *o = reinterpret_cast<EXOver *>(p_over);
			ProcessRespown(key);
			delete o;
		}
		else {

		}
	}
}

void AcceptThread()
{
	auto g_socket = WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED); // ������ ���ڿ� Overlapped �־������

	SOCKADDR_IN bind_addr;
	ZeroMemory(&bind_addr, sizeof(SOCKADDR_IN));
	bind_addr.sin_family = AF_INET;
	bind_addr.sin_port = htons(MY_SERVER_PORT);
	bind_addr.sin_addr.s_addr = INADDR_ANY; // �ƹ� �ּҳ� �� �ްڴ�.

	::bind(g_socket, reinterpret_cast<sockaddr *>(&bind_addr),
		sizeof(SOCKADDR)); // ���� �Ⱦ��� C++11�� bind��� ���� �ٸ� �Լ��� ȣ���
	listen(g_socket, 1000); // ���� 2��°�� ��α� // �ִ� ����� ��ٸ��� �� ���ΰ�
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
		cout << "���ο� Ŭ���̾�Ʈ ����\n";
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
		printf("%d�� �÷��̾� ����.\n", new_key);
		g_clients[new_key].s = new_socket;
		ZeroMemory(&g_clients[new_key].exover.wsaover, sizeof(WSAOVERLAPPED)); // ���ú��Ҷ����� Ŭ���� ����� �Ѵ�.

		CreateIoCompletionPort(reinterpret_cast<HANDLE>(new_socket),
			g_iocp, new_key, 0);
		//g_clients[new_key].viewlist.clear();
		g_clients[new_key].in_use = true; // �̰� ��ġ ���⿩�߸� �Ѵٴµ� �ֱ׷��� �𸣰��� // ��Ƽ������ �����̶�µ�....
		unsigned long flag = 0;
		int ret = WSARecv(new_socket, &g_clients[new_key].exover.wsabuf, 1,
			NULL, &flag, &g_clients[new_key].exover.wsaover, NULL);
		if (0 != ret) {
			int err_no = WSAGetLastError();
			if (WSA_IO_PENDING != err_no)
				error_display("Recv in AcceptThread", err_no);
		}

		PutNewPlayer(new_key);
	}
}

int main()
{
	vector <thread> all_threads;
	Initialize();

	for (int i = 0; i < 4; ++i)
		all_threads.push_back(thread{ WorkerThread });
	all_threads.push_back(thread{ AcceptThread });
	all_threads.push_back(thread{ TimerThread });

	for (auto &th : all_threads) th.join(); // ���۷����� �޾ƾ���
	WSACleanup();
}