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
#include <fstream>
#include <type_traits>
#include "Functor.h"
#include "../../Slash/Common/MathHelper.h"


#pragma comment (lib, "ws2_32.lib")
#pragma comment(lib,  "lua53.lib")

#include "protocol.h"

using namespace std;

// For Script
extern "C"
{
#include "include\lua.h"
#include "include\lauxlib.h"
#include "include\lualib.h"
}

void display_error(lua_State* L, int errnum)
{
	printf("Erorr : %s\n", lua_tostring(L, -1));
	lua_pop(L, 1);
}

void SendObjectPos(int client, int object_id);
void SendObjectLook(int client, int object_id);
void SendObjectState(int client, int object_id);
void SendRemoveObject(int client, int object_id);
void SendPutObject(int client, int object_id);
void SendObjectHp(int client, int object_id);
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
	BYTE State;
	BoundingOrientedBox xmOOBB;
	BoundingOrientedBox xmOOBBTransformed;
	unordered_set<int> viewlist;
	mutex vlm;
	bool is_active;
	int Hp;
	int Dmg;
	lua_State* L;
	BYTE Type;

	EXOver exover;
	int packet_size;
	int prev_size;
	char prev_packet[MAX_PACKET_SIZE];
};

struct MapObject {
	XMFLOAT4X4 World;
	BoundingBox Bounds;
	BoundingOrientedBox xmOOBB;
	BoundingOrientedBox xmOOBBTransformed;
};

HANDLE g_iocp; // IOCP 전역변수 객체
CLIENT g_clients[NUM_OF_NPC];
vector<MapObject*> g_mapobjects;

struct EVENT {
	unsigned int s_time;
	int type;
	int object; // 누가 공격하고 힐링해야 하는가
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

void SetOOBB(MapObject* d, XMFLOAT3& xmCenter, XMFLOAT3& xmExtents, const XMFLOAT4& xmOrientation)
{
	d->xmOOBBTransformed = d->xmOOBB = BoundingOrientedBox(xmCenter, xmExtents, xmOrientation);
}

void SetOOBB(CLIENT& cl, const XMFLOAT3& xmCenter, const XMFLOAT3& xmExtents, const XMFLOAT4& xmOrientation)
{
	cl.xmOOBBTransformed = cl.xmOOBB = BoundingOrientedBox(xmCenter, xmExtents, xmOrientation);
}


//void LoadMap()
//{
//	ifstream fin("../../Slash/Chapter 16 Instancing and Frustum Culling/InstancingAndCulling/Assets/Data/MapData.txt");
//	if (!fin)
//	{
//		cout << "MapData Load Failed" << endl;
//	}
//
//	g_mapobjects.reserve(MAX_MAPOBJECT);
//
//	string ignore;
//
//	while (!fin.eof())
//	{
//
//		fin >> ignore;		// > Com_Mesh_이름
//		wstring wstrFileName;
//		wstring meshName = wstrFileName.assign(ignore.begin(), ignore.end());
//
//		int iSize = 1;
//		fin >> iSize;			// > Instancing Size
//
//		std::string::size_type sz;
//
//		string Value[9];
//		float fValue[9];
//
//		for (int i = 0; i < iSize; ++i)
//		{
//			int Alpha;
//			fin >> Alpha;
//			fin >> ignore;			// > Tex_Name
//		
//			
//			for (int j = 0; j < 9; ++j)
//			{
//				fin >> Value[j];
//				fValue[j] = std::stof(Value[j], &sz);
//			}
//
//			MapObject* p = new MapObject;
//
//			p->World = MathHelper::Identity4x4();
//
//			XMStoreFloat4x4(&p->World, XMMatrixScaling(fValue[3], fValue[4], fValue[5])
//				* XMMatrixRotationX(XMConvertToRadians(fValue[6])) * XMMatrixRotationY(XMConvertToRadians(fValue[7])) * XMMatrixRotationZ(XMConvertToRadians(fValue[8]))
//				*XMMatrixTranslation(fValue[0], fValue[1], fValue[2]));
//
//			p->xmOOBBTransformed.Transform(p->xmOOBB, XMLoadFloat4x4(&p->World));
//			XMStoreFloat4(&p->xmOOBBTransformed.Orientation, XMQuaternionNormalize(XMLoadFloat4(&p->xmOOBBTransformed.Orientation)));
//
//			g_mapobjects.emplace_back(p);
//
//			//dynamic_cast<CInstancingObject*>(pObject)->GetTransform(i)->Translation(fValue[0], fValue[1], fValue[2]);
//			//dynamic_cast<CInstancingObject*>(pObject)->GetTransform(i)->Scaling(fValue[3], fValue[4], fValue[5]);
//			//dynamic_cast<CInstancingObject*>(pObject)->GetTransform(i)->Rotation(fValue[6], fValue[7], fValue[8]);
//
//		}
//	}
//}

bool CanSeeMapObject(int a, MapObject* b)
{
	float dist_sq = (g_clients[a].World._41 - b->World._41) * (g_clients[a].World._41 - b->World._41)
		+ (g_clients[a].World._42 - b->World._42) * (g_clients[a].World._42 - b->World._42)
		+ (g_clients[a].World._43 - b->World._43) * (g_clients[a].World._43 - b->World._43);
	return (dist_sq <= MAPOBJECT_RADIUS * MAPOBJECT_RADIUS);
}

bool CanSee(int a, int b)
{
	float dist_sq = (g_clients[a].World._41 - g_clients[b].World._41) * (g_clients[a].World._41 - g_clients[b].World._41)
		+ (g_clients[a].World._42 - g_clients[b].World._42) * (g_clients[a].World._42 - g_clients[b].World._42)
		+ (g_clients[a].World._43 - g_clients[b].World._43) * (g_clients[a].World._43 - g_clients[b].World._43);
	return (dist_sq <= VIEW_RADIUS * VIEW_RADIUS);
}

bool CanSee(int a, MapObject* b)
{
	float dist_sq = (g_clients[a].World._41 - b->World._41) * (g_clients[a].World._41 - b->World._41)
		+ (g_clients[a].World._42 - b->World._42) * (g_clients[a].World._42 - b->World._42)
		+ (g_clients[a].World._43 - b->World._43) * (g_clients[a].World._43 - b->World._43);
	return (dist_sq <= VIEW_RADIUS * VIEW_RADIUS);
}

bool IsClose(int a, int b)
{
	float dist_sq = (g_clients[a].World._41 - g_clients[b].World._41) * (g_clients[a].World._41 - g_clients[b].World._41)
		+ (g_clients[a].World._42 - g_clients[b].World._42) * (g_clients[a].World._42 - g_clients[b].World._42)
		+ (g_clients[a].World._43 - g_clients[b].World._43) * (g_clients[a].World._43 - g_clients[b].World._43);
	return (dist_sq <= CLOSE_RADIUS * CLOSE_RADIUS);
}

bool IsClose(int a, MapObject* b)
{
	float dist_sq = (g_clients[a].World._41 - b->World._41) * (g_clients[a].World._41 - b->World._41)
		+ (g_clients[a].World._42 - b->World._42) * (g_clients[a].World._42 - b->World._42)
		+ (g_clients[a].World._43 - b->World._43) * (g_clients[a].World._43 - b->World._43);
	return (dist_sq <= CLOSE_RADIUS * CLOSE_RADIUS);
}

bool IsAttackRange(int a, int b)
{
	float dist_sq = (g_clients[a].World._41 - g_clients[b].World._41) * (g_clients[a].World._41 - g_clients[b].World._41)
		+ (g_clients[a].World._42 - g_clients[b].World._42) * (g_clients[a].World._42 - g_clients[b].World._42)
		+ (g_clients[a].World._43 - g_clients[b].World._43) * (g_clients[a].World._43 - g_clients[b].World._43);
	return (dist_sq <= PLAYER_ATTACK_RADIUS * PLAYER_ATTACK_RADIUS);
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
	if (DEAD == g_clients[id].State) return;
	g_clients[id].is_active = true;
	g_clients[id].State = WALK;

	for (int i = 0; i < MAX_USER; ++i)
	{
		if (false == g_clients[i].in_use) continue;
		if (false == CanSee(id, i)) continue;
		SendObjectState(i, id);
	}

	add_timer(id, EVT_CHASE, GetTickCount(), target);
}

XMFLOAT3 GetSlideVector(CLIENT& Obj, CLIENT& CollObj)
{
	XMFLOAT3 Center = XMFLOAT3{ CollObj.World._41, CollObj.World._42, CollObj.World._43 };

	XMFLOAT3 Player = XMFLOAT3{ Obj.World._41, Obj.World._42, Obj.World._43 };
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

XMFLOAT3 GetSlideVector(CLIENT& Obj, MapObject* CollObj)
{
	XMMATRIX world = XMLoadFloat4x4(&CollObj->World);

	XMMATRIX invWorld = XMMatrixInverse(&XMMatrixDeterminant(world), world);

	// View space to the object's local space.

	// Transform the camera frustum from view space to the object's local space.

	BoundingOrientedBox mLocalPlayerBounds;
	// Transform the camera frustum from view space to the object's local space.
	Obj.xmOOBB.Transform(mLocalPlayerBounds, invWorld);

	BoundingBox WorldBounds = CollObj->Bounds;
	// Transform the camera frustum from view space to the object's local space.
	WorldBounds.Transform(WorldBounds, XMLoadFloat4x4(&(CollObj->World)));


	//XMFLOAT3 pCollObjPos2= pCollobj->m_pTransCom->m_f3Position;
	//XMFLOAT3 Center = pCollobj->GetTransform()->GetPosition();
	//XMFLOAT3 Player = pobj->GetPosition();

	XMFLOAT3 Center = XMFLOAT3(CollObj->World._41, CollObj->World._42, CollObj->World._43);
	XMFLOAT3 Player = XMFLOAT3(Obj.World._41, Obj.World._42, Obj.World._43);
	XMFLOAT3 look = XMFLOAT3(CollObj->World._21, CollObj->World._22, CollObj->World._23);
	XMFLOAT3 dirVector = Vector3::Subtract(Player, Center);   // 충돌 객체에서 플레이어로 가는 벡터
	XMFLOAT3 MovingRefletVector = XMFLOAT3{ 0, 0, 0 };

	float tanceta = dirVector.z / dirVector.x;
	float ceta = atan(tanceta) * 57.3248f;

	float extenttanceta = WorldBounds.Extents.z / WorldBounds.Extents.x;

	float extentceta = atan(extenttanceta) * 57.3248f;

	look = Vector3::Normalize(look);
	dirVector = Vector3::Normalize(dirVector);

	if (Player.x > Center.x && 0 < ceta && ceta < extentceta)
		MovingRefletVector = XMFLOAT3(1, 0, 0);
	if (Player.z > Center.z && extentceta < ceta && ceta < 90)
		MovingRefletVector = XMFLOAT3(0, 0, 1);
	if (Player.z > Center.z && -90 < ceta && ceta < -extentceta)
		MovingRefletVector = XMFLOAT3(0, 0, 1);
	if (Player.x < Center.x && -extentceta < ceta && ceta < 0)
		MovingRefletVector = XMFLOAT3(-1, 0, 0);
	if (Player.x < Center.x && 0 < ceta && ceta < extentceta)
		MovingRefletVector = XMFLOAT3(-1, 0, 0);
	if (Player.z < Center.z && extentceta < ceta && ceta < 90)
		MovingRefletVector = XMFLOAT3(0, 0, -1);
	if (Player.z < Center.z && -90 < ceta && ceta < -extentceta)
		MovingRefletVector = XMFLOAT3(0, 0, -1);
	if (Player.x > Center.x && -extentceta < ceta && ceta < 0)
		MovingRefletVector = XMFLOAT3(1, 0, 0);

	return MovingRefletVector;

}
void ChasingPlayer(int source, int target) {

	if (g_clients[source].State == DEAD)
		return;

	// 거미끼리 충돌체크할때 계속 거리 재야하는가
	// 아니면 전체 거미끼리 충돌하는지 검사하는가

	// 거미의 서버 스테이터스에 이동중으로 설정

	// 거미가 플레이어 위치 기반 움직이는게 조금 이상함

	XMFLOAT3 MonsterLook = XMFLOAT3(g_clients[source].World._31, g_clients[source].World._32, g_clients[source].World._33);
	XMFLOAT3 MonsterPos = XMFLOAT3(g_clients[source].World._41, g_clients[source].World._42, g_clients[source].World._43);

	//cout << MonsterLook.x << " " << MonsterLook.y << " " << MonsterLook.z << endl;
	//cout << MonsterPos.x << " " << MonsterPos.y << " " << MonsterPos.z << endl;

	XMFLOAT3 playerPos = XMFLOAT3(g_clients[target].World._41, g_clients[target].World._42, g_clients[target].World._43);
	XMFLOAT3 Shift = Vector3::Normalize(MonsterLook);

	//cout << playerPos.x << " " << playerPos.y << " " << playerPos.z << endl;
	//cout << Shift.x << " " << Shift.y << " " << Shift.z << endl << endl;

	XMFLOAT3 dirVector = Vector3::Subtract(playerPos, MonsterPos);   // 객체에서 플레이어로 가는 벡터

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

	//cout << ceta << endl;

	XMFLOAT3 MovingReflectVector = XMFLOAT3(0, 0, 0); // 나중에 충돌하면 갱신해줘야함 이동하기전에

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

	XMFLOAT3 movingVector = XMFLOAT3(dirVector.x * dMonsterMoveSpeed, dirVector.y * dMonsterMoveSpeed, dirVector.z * dMonsterMoveSpeed); // 빠를수있음

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

	for (auto d : g_mapobjects)
	{
		if (false == IsClose(source, d)) continue;

		if (g_clients[source].xmOOBB.Contains(d->xmOOBB))
		{
			MovingReflectVector = GetSlideVector(g_clients[source], d);
			break;
		}
	}

	auto result = Vector3::MultiplyScalr(MovingReflectVector, Vector3::DotProduct(movingVector, MovingReflectVector)); // 쓸데없이 실행되는가 체크
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

		g_clients[source].State = IDLE;
		g_clients[source].is_active = false;

		for (int i = 0; i < MAX_USER; ++i)
		{
			if (false == g_clients[i].in_use) continue;
			if (false == CanSee(i, source)) continue;
			SendObjectState(i, source);
		}
	}
	else if (IsClose(target, source))	 //충돌할 정도로 가까워 졌으면
	{
		if (ATTACK1 == g_clients[source].State) return;

		add_timer(source, EVT_MONSTER_ATTACK, GetTickCount(), target);

		// 몬스터 때리는 상태로 변경하고 클라로 보내기 (그냥 클라에서 계속 계산하다가 판단할까?)
		// 그리고 몇초 뒤 플레이어 체력 깎는다.
	}
	else
	{
		add_timer(source, EVT_CHASE, GetTickCount() + 50, target);
	}


}
void SendPutMonster(int client, int object_id)
{
	sc_packet_put_monster put_p;
	put_p.id = object_id;
	put_p.size = sizeof(sc_packet_put_monster);
	put_p.type = SC_PUT_MONSTER;
	put_p.posX = g_clients[object_id].World._41;
	put_p.posY = g_clients[object_id].World._42;
	put_p.posZ = g_clients[object_id].World._43;
	put_p.lookDegree = g_clients[object_id].LookDegree;
	put_p.state = g_clients[object_id].State;
	put_p.monsterType = g_clients[object_id].Type;

	SendPacket(client, &put_p);
}

void PutNewPlayer(int new_key) {

	g_clients[new_key].Hp = 200;
	g_clients[new_key].World._41 = 15;
	g_clients[new_key].World._42 = 0;
	g_clients[new_key].World._43 = 0;

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

	// 나의 접속을 모든 플레이어에게 알림
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
	// 접속중인 다른 플레이어 정보를 전송
	for (int i = 0; i < MAX_USER; ++i) {
		if (false == g_clients[i].in_use) continue;
		if (false == CanSee(i, new_key)) continue;
		if (i == new_key) continue;

		p.id = i;
		p.posX = g_clients[i].World._41;
		p.posY = g_clients[i].World._42;
		p.posZ = g_clients[i].World._43;
		p.lookDegree = g_clients[i].LookDegree;
		p.state = g_clients[i].State;

		SendPacket(new_key, &p);

		g_clients[new_key].vlm.lock();
		g_clients[new_key].viewlist.insert(i);
		g_clients[new_key].vlm.unlock();
	}
	for (int i = NPC_START; i < NUM_OF_NPC; i++)
	{
		if (false == CanSee(new_key, i)) continue;

		if (IsInAgroRange(i, new_key))
			WakeUpNPC(i, new_key); // 상태 2번 보냄 수정해야함


		SendPutMonster(new_key, i);

		g_clients[new_key].vlm.lock();
		g_clients[new_key].viewlist.insert(i);
		g_clients[new_key].vlm.unlock();

		//cout << "접속 : " << i << " 번째 몬스터 보임" << endl;
	}

}

void MonsterAttack(int source, int target) {

	if (g_clients[target].in_use == false)
		return;
	if (g_clients[source].State == DEAD)
		return;

	if (!IsClose(source, target))
	{
		g_clients[source].State = WALK;

		for (int i = 0; i < MAX_USER; ++i)
		{
			if (false == g_clients[i].in_use) continue;
			if (false == CanSee(i, source)) continue;
			SendObjectState(i, source);
		}

		add_timer(source, EVT_CHASE, GetTickCount(), target);
		return;
	}
	else
	{
		g_clients[source].State = ATTACK1;

		for (int i = 0; i < MAX_USER; ++i)
		{
			if (false == g_clients[i].in_use) continue;
			if (false == CanSee(i, source)) continue;
			SendObjectState(i, source);
		}

		add_timer(source, EVT_DAMAGE, GetTickCount() + 300, target);

		if ((g_clients[target].Hp - g_clients[source].Dmg) > 0)
		{
			add_timer(source, EVT_MONSTER_ATTACK, GetTickCount() + 1200, target);
		}
		else
		{
			g_clients[source].State = IDLE;
			g_clients[source].is_active = false;

			for (int i = 0; i < MAX_USER; ++i)
			{
				if (false == g_clients[i].in_use) continue;
				if (false == CanSee(i, source)) continue;
				SendObjectState(i, source);
			}
		}
	}
}

void PlayerAttack(int source) {

	for (int i = NPC_START; i < NUM_OF_NPC; ++i) // 뷰리스트에 있는 애들 쓰는게 더 나으려나..
	{
		if (false == g_clients[i].is_active) continue;
		if (false == IsAttackRange(source, i)) continue;

		// 몬스터가 앞에 있는가 여기에서 판단

		add_timer(source, EVT_DAMAGE, GetTickCount() + 50, i);
	}

}

void ProcessDamage(int source, int target) {

	if (g_clients[target].State == DEAD)
		return;

	g_clients[target].Hp -= g_clients[source].Dmg;

	if ((g_clients[target].Hp - g_clients[source].Dmg) < 0)
		g_clients[target].Hp = 0;

	if (!IsNPC(target))
		SendObjectHp(target, target);

	if ((g_clients[target].Hp) <= 0)
	{
		g_clients[target].State = DEAD;
		if (IsNPC(target))
			g_clients[target].is_active = false;

		for (int i = 0; i < MAX_USER; ++i)
		{
			if (false == g_clients[i].in_use) continue;
			if (false == CanSee(i, target)) continue;
			SendObjectState(i, target);
		}
		cout << "살아남" << endl;
		if(IsNPC(target))
			add_timer(target, EVT_RESPOWN, GetTickCount() + 50000, 0);
		else
			add_timer(target, EVT_RESPOWN, GetTickCount() + 5000, 0);
	}
}

void ProcessRespown(int source)
{
	if (false == IsNPC(source))
		if (false == g_clients[source].in_use)
			return;

	g_clients[source].State = IDLE;

	if (IsNPC(source))
	{
		g_clients[source].Hp = 5;

		for (int i = 0; i < MAX_USER; ++i)
		{
			if (false == g_clients[i].in_use) continue;
			if (false == CanSee(i, source)) continue;
			SendObjectState(i, source);
			if (false == IsInAgroRange(i, source)) continue;
			ChasingPlayer(source, i);
			break;
		}
	}
	else
	{
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
	wcout << L"에러 " << lpMsgBuf << endl;
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
		cl.State = IDLE;
		cl.is_active = false;
	}

	for (int i = 0; i < MAX_USER; ++i)
	{
		XMStoreFloat4x4(&g_clients[i].World, XMMatrixScaling(0.05f, 0.05f, 0.05f)*XMMatrixRotationX(1.7f)*XMMatrixRotationZ(3.14f)*XMMatrixTranslation(0.0f, 0.0f, 0.0f));
		g_clients[i].World._41 = 15, g_clients[i].World._42 = 0, g_clients[i].World._43 = 0; // 플레이어 위치 초기화
		g_clients[i].Hp = 200;
		g_clients[i].Dmg = 10;
		SetOOBB(g_clients[i], XMFLOAT3(-7.5388, -5.98235, 28.8367), XMFLOAT3(23.1505, 16.4752, 28.5554), XMFLOAT4(0.0, 0.0, 0.0, 1.0));
	}
	for (int i = NPC_START; i < NUM_OF_NPC; ++i)
	{
		XMStoreFloat4x4(&g_clients[i].World, XMMatrixScaling(2.0f, 2.0f, 2.0f)*XMMatrixTranslation(0.0f, 0.0f, 0.0f)*XMMatrixTranslation(0.0f, 0.0f, 0.0f));
		//g_clients[i].World._41 = 0, g_clients[i].World._42 = 0, g_clients[i].World._43 = (i - NPC_START); // 거미 위치 초기화
		//g_clients[i].Hp = 60;
		//g_clients[i].Dmg = 10;

		lua_State* L = luaL_newstate();
		luaL_openlibs(L);
		int error = luaL_loadfile(L, "monster.lua");
		lua_pcall(L, 0, 0, 0);
		lua_getglobal(L, "set_myid");
		lua_pushnumber(L, i);
		lua_pcall(L, 1, 0, 0);
		g_clients[i].L = L;

		lua_getglobal(g_clients[i].L, "LoadMonsterData");
		lua_pcall(g_clients[i].L, 0, 5, 0);
		g_clients[i].World._41 = (int)lua_tonumber(L, -5);
		g_clients[i].World._42 = 0;
		g_clients[i].World._43 = (int)lua_tonumber(L, -4);
		g_clients[i].Hp = (int)lua_tonumber(L, -3);
		g_clients[i].Dmg = (int)lua_tonumber(L, -2);
		g_clients[i].Type = (int)lua_tonumber(L, -1);
		lua_pop(L, 5);

		SetOOBB(g_clients[i], XMFLOAT3(0, 0.3232, 0.7277), XMFLOAT3(1.0345, 0.7848, 1.0931), XMFLOAT4(0.0, 0.0, 0.0, 1.0));

		g_clients[i].xmOOBBTransformed.Transform(g_clients[i].xmOOBB, XMLoadFloat4x4(&g_clients[i].World));
		XMStoreFloat4(&g_clients[i].xmOOBBTransformed.Orientation, XMQuaternionNormalize(XMLoadFloat4(&g_clients[i].xmOOBBTransformed.Orientation)));

	}

	//LoadMap();
	g_mapobjects.reserve(MAX_MAPOBJECT);

	WSADATA   wsadata;
	WSAStartup(MAKEWORD(2, 2), &wsadata);

	g_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);

	cout << "Slash 서버 생성 완료" << endl;
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

	sc_packet_remove_object p;
	p.id = cl;
	p.size = sizeof(p);
	p.type = SC_REMOVE_OBJECT;

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
	state_p.state = g_clients[object_id].State;

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
	put_p.state = g_clients[object_id].State;

	SendPacket(client, &put_p);
}


void SendRemoveObject(int client, int object_id)
{
	sc_packet_remove_object p;
	p.id = object_id;
	p.size = sizeof(sc_packet_remove_object);
	p.type = SC_REMOVE_OBJECT;

	SendPacket(client, &p);
}

void SendObjectHp(int client, int object_id)
{
	sc_packet_hp p;
	p.id = object_id;
	p.size = sizeof(sc_packet_hp);
	p.type = SC_HP;
	p.hp = g_clients[object_id].Hp;

	SendPacket(client, &p);
}


void ProcessPacket(int cl, char *packet)
{
	if (packet[1] <= MOVE_PACKET_END && packet[1] >= MOVE_PACKET_START)
	{
		// LEFT RIGHT 이동 시 룩벡터 3열 부호 다름
		// UP DOWN 이동 시 라이트벡터 3열 부호 다름

		g_clients[cl].State = WALK;

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

		float ceta = acos(cosCeta); // 현재 각도

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

		//cout << "Player Pos << " << g_clients[cl].World._41 << "\t" << g_clients[cl].World._42 << "\t" << g_clients[cl].World._43 << endl;


		// 플레이어 이동시 충돌체크

		for (auto d : g_mapobjects)
		{
			if (false == CanSeeMapObject(cl, d)) continue;

			XMMATRIX world = XMLoadFloat4x4(&d->World);
			XMMATRIX invWorld = XMMatrixInverse(&XMMatrixDeterminant(world), world);
			BoundingOrientedBox mLocalPlayerBounds;
			g_clients[cl].xmOOBB.Transform(mLocalPlayerBounds, invWorld);

			if (mLocalPlayerBounds.Contains(d->Bounds) != DirectX::DISJOINT)
			{

				SlideVector = GetSlideVector(g_clients[cl], d);
				float cosCeta = Vector3::DotProduct(SlideVector, xmf3Shift);
				if (cosCeta < 0)
				{
					auto result = Vector3::MultiplyScalr(SlideVector, Vector3::DotProduct(xmf3Shift, SlideVector));
					XMFLOAT3 sub_xmf3Shift = Vector3::Subtract(xmf3Shift, result);

					g_clients[cl].World._41 += sub_xmf3Shift.x, g_clients[cl].World._42 += sub_xmf3Shift.y, g_clients[cl].World._43 += sub_xmf3Shift.z;
					IsCollision = true;
					break;
				}
			}
		}

		for (int i = 0; i < NUM_OF_NPC; ++i)
		{
			if (i < NPC_START)
			{
				if (false == g_clients[i].in_use) continue;
			}
			else if (i > NPC_START)
			{
				if (false == g_clients[i].is_active) continue;
			}
			if (false == IsClose(i, cl)) continue;
			if (i == cl) continue;

			if (g_clients[cl].xmOOBB.Contains(g_clients[i].xmOOBB))
			{
				//cout << i << "번째 객체와 충돌 Type : " << int(g_clients[i].Type) << endl;
				SlideVector = GetSlideVector(g_clients[cl], g_clients[i]);
				float cosCeta = Vector3::DotProduct(SlideVector, xmf3Shift);
				if (cosCeta < 0)
				{
					auto result = Vector3::MultiplyScalr(SlideVector, Vector3::DotProduct(xmf3Shift, SlideVector));
					XMFLOAT3 sub_xmf3Shift = Vector3::Subtract(xmf3Shift, result);

					g_clients[cl].World._41 += sub_xmf3Shift.x, g_clients[cl].World._42 += sub_xmf3Shift.y, g_clients[cl].World._43 += sub_xmf3Shift.z;
					IsCollision = true;
					break; // 먼저 충돌한 객체만 처리
				}
			}
		}

		if (!IsCollision)
			g_clients[cl].World._41 += xmf3Shift.x, g_clients[cl].World._42 += xmf3Shift.y, g_clients[cl].World._43 += xmf3Shift.z;

		cout << " Pos = X : " << g_clients[cl].World._41 << " Y : " << g_clients[cl].World._42 << " Z : " << g_clients[cl].World._43 << endl;

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

		unordered_set <int> new_view_list; // 새로운 뷰리스트 생성
		for (int i = 0; i < MAX_USER; ++i) {
			if (i == cl) continue;
			if (false == g_clients[i].in_use) continue;
			if (false == CanSee(cl, i)) continue;
			// 새로운 뷰리스트 (나를 기준) // 새로 이동하고 보이는 모든 애들
			new_view_list.insert(i);
		}
		for (int i = NPC_START; i < NUM_OF_NPC; ++i) {
			if (false == CanSee(cl, i)) continue;
			new_view_list.insert(i);
		}

		SendPacket(cl, &sp_pos);
		if (IsRotated)
			SendPacket(cl, &sp_rotate);

		for (auto id : new_view_list) {
			g_clients[cl].vlm.lock();

			WakeUpNPC(id, cl);

			// 나의 기존 뷰리스트에는 없었다 // 즉 새로 들어왔다
			if (0 == g_clients[cl].viewlist.count(id))
			{
				g_clients[cl].viewlist.insert(id);
				g_clients[cl].vlm.unlock();

				if (IsNPC(id))
					SendPutMonster(cl, id);
				else
					SendPutObject(cl, id);
				
			}
			else
				g_clients[cl].vlm.unlock();

			if (true == IsNPC(id)) continue;

			g_clients[id].vlm.lock();
			// 상대방한테 내가 없었다? // 추가
			if (0 == g_clients[id].viewlist.count(cl)) {
				g_clients[id].viewlist.insert(cl);
				g_clients[id].vlm.unlock();
				if (IsNPC(id))
					SendPutMonster(cl, id);
				else
					SendPutObject(cl, id);
			}
			// 상대방한테 내가 있었다? // 위치값만
			else
			{
				g_clients[id].vlm.unlock();
				SendPacket(id, &sp_pos);
				if (IsRotated)
					SendPacket(id, &sp_rotate);
			}
		}
		// 나의 이전 뷰리스트에 있는 애들
		g_clients[cl].vlm.lock();
		unordered_set <int> old_v = g_clients[cl].viewlist;
		g_clients[cl].vlm.unlock();
		for (auto id : old_v) {
			if (0 == new_view_list.count(id)) {
				if (cl == id) continue;
				g_clients[cl].vlm.lock();
				g_clients[cl].viewlist.erase(id); // 계속 락 언락하지 말고 따로 리무브리스트만들고 한번에 지우는게 좋음
				g_clients[cl].vlm.unlock();
				SendRemoveObject(cl, id);

				if (true == IsNPC(id)) continue;
				g_clients[id].vlm.lock();
				// 있을 경우에만 지우게 하자.
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
	else if (packet[1] == CS_STOP)
	{
		//cout << cl << g_clients[cl].LookVector.x << " " << g_clients[cl].LookVector.y << " " << g_clients[cl].LookVector.z << endl;

		g_clients[cl].State = IDLE;
		sc_packet_state sp;
		sp.id = cl;
		sp.size = sizeof(sc_packet_state);
		sp.type = SC_STATE;
		sp.state = g_clients[cl].State;
		for (int i = 0; i < MAX_USER; ++i)
		{
			if (false == g_clients[i].in_use) continue;
			if (false == CanSee(cl, i)) continue;
			SendPacket(i, &sp);
		}

		return;
	}
	else if ((packet[1] == CS_ATTACK1) || (packet[1] == CS_ATTACK2) || (packet[1] == CS_ATTACK3))
	{
		if(packet[1] == CS_ATTACK1)
			g_clients[cl].State = ATTACK1;
		else if (packet[1] == CS_ATTACK2)
			g_clients[cl].State = ATTACK2;
		else if (packet[1] == CS_ATTACK3)
			g_clients[cl].State = ATTACK3;

		for (int i = 0; i < MAX_USER; ++i)
		{
			if (false == g_clients[i].in_use) continue;
			if (false == CanSee(i, cl)) continue;

			SendObjectState(i, cl);

		}

		add_timer(cl, EVT_PLAYER_ATTACK, GetTickCount() + iAttackDelay, 0);

		return;
	}
	else if (packet[1] == CS_MAP_INIT_DATA)
	{
		static int first_client = cl;

		if (first_client != cl)
			return;

		cs_packet_mapinitdata *p = reinterpret_cast<cs_packet_mapinitdata *>(packet);

		MapObject* MapData = new MapObject;

		MapData->World = p->world;
		MapData->Bounds = p->bounds;

		//BoundingBox Bounds = p->bounds;
		//DirectX::BoundingOrientedBox::CreateFromBoundingBox(MapData->xmOOBB, Bounds);
		//DirectX::BoundingOrientedBox::CreateFromBoundingBox(MapData->xmOOBBTransformed, Bounds);


		//cout << p->world._11 << " " << p->world._12 << " " << p->world._13 << " " << p->world._14 << endl;
		//cout << p->world._21 << " " << p->world._22 << " " << p->world._23 << " " << p->world._24 << endl;
		//cout << p->world._31 << " " << p->world._32 << " " << p->world._33 << " " << p->world._34 << endl;
		//cout << p->world._41 << " " << p->world._42 << " " << p->world._43 << " " << p->world._44 << endl << endl;


		//MapData->xmOOBBTransformed.Transform(MapData->xmOOBB, XMLoadFloat4x4(&MapData->World));
		//XMStoreFloat4(&MapData->xmOOBBTransformed.Orientation, XMQuaternionNormalize(XMLoadFloat4(&MapData->xmOOBBTransformed.Orientation)));

		g_mapobjects.emplace_back(MapData);

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
		unsigned long long key; // 64비트 모드에서는 long long으로 32비트에서는 long으로
		WSAOVERLAPPED *p_over;

		BOOL is_success = GetQueuedCompletionStatus(g_iocp,
			&data_size, &key, &p_over, INFINITE);

		//printf("GQCS from client [ %d ] with size [ %d ]\n", key, data_size);

		// 접속종료 처리
		if (0 == data_size) {
			DisconnectPlayer(key);
			continue;
		}
		// 에러 처리
		if (0 == is_success) {
			//printf("Error in GQCS key[ %d ]\n", key);
			DisconnectPlayer(key);
			continue;
		}

		// Send/Recv 처리
		EXOver *o = reinterpret_cast<EXOver *>(p_over);
		if (EVT_RECV == o->event_type) {
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
		ZeroMemory(&g_clients[new_key].exover.wsaover, sizeof(WSAOVERLAPPED)); // 리시브할때마다 클리어 해줘야 한다.

		CreateIoCompletionPort(reinterpret_cast<HANDLE>(new_socket),
			g_iocp, new_key, 0);
		//g_clients[new_key].viewlist.clear();
		g_clients[new_key].in_use = true; // 이거 위치 여기여야만 한다는데 왜그런지 모르겠음 // 멀티쓰레드 떄문이라는데....
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

	for (auto &th : all_threads) th.join(); // 레퍼런스로 받아야함
	WSACleanup();
}