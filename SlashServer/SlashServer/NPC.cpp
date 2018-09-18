#include "NPC.h"
#include "Defines.h"

NPC::NPC()
{
}

NPC::~NPC()
{
}

void NPC::Initialize()
{
	GameObject::Initialize();


	lua_State* L = luaL_newstate();
	luaL_openlibs(L);
	int error = luaL_loadfile(L, "monster.lua");
	lua_pcall(L, 0, 0, 0);
	lua_getglobal(L, "set_myid");
	lua_pushnumber(L, ID_);
	lua_pcall(L, 1, 0, 0);

	if(NPC_SPIDER == npcType_)
		XMStoreFloat4x4(&world_, XMMatrixScaling(2.0f, 2.0f, 2.0f)*XMMatrixTranslation(0.0f, 0.0f, 0.0f)*XMMatrixTranslation(0.0f, 0.0f, 0.0f));
	else if(NPC_NAGA_GUARD == npcType_)
		XMStoreFloat4x4(&world_, XMMatrixScaling(0.08f, 0.08f, 0.08f)*XMMatrixRotationX(1.7f)*XMMatrixRotationZ(3.14f)*XMMatrixTranslation(0.0f, 0.0f, 0.0f));
	else if(NPC_ROCK_WARRIOR == npcType_) // 보스 렌더 크기만 키웠음
		XMStoreFloat4x4(&world_, XMMatrixScaling(0.08f, 0.08f, 0.08f)*XMMatrixRotationX(1.7f)*XMMatrixRotationZ(3.14f)*XMMatrixTranslation(0.0f, 0.0f, 0.0f));
	else if(NPC_TREE_GUARD == npcType_)
		XMStoreFloat4x4(&world_, XMMatrixScaling(0.07f, 0.07f, 0.07f)*XMMatrixRotationX(1.7f)*XMMatrixRotationZ(3.14f)*XMMatrixTranslation(0.0f, 0.0f, 0.0f));
	else if(NPC_TURTLE == npcType_)
		XMStoreFloat4x4(&world_, XMMatrixScaling(0.05f, 0.05f, 0.05f)*XMMatrixRotationX(1.7f)*XMMatrixRotationZ(3.14f)*XMMatrixTranslation(0.0f, 0.0f, 0.0f));
	else if (NPC_DRAGON == npcType_)
		XMStoreFloat4x4(&world_, XMMatrixScaling(0.1f, 0.1f, 0.1f)*XMMatrixRotationX(1.7f)*XMMatrixRotationZ(3.14f)*XMMatrixTranslation(0.0f, 0.0f, 0.0f));

	lua_getglobal(L, "LoadMonsterData");
	lua_pushnumber(L, static_cast<int>(npcType_));
	lua_pcall(L, 1, 5, 0);
	world_._41 = static_cast<float>(lua_tonumber(L, -5));
	world_._42 = 0;
	world_._43 = static_cast<float>(lua_tonumber(L, -4));
	hp_ = (int)lua_tonumber(L, -3);
	dmg_ = (int)lua_tonumber(L, -2);
	exp_ = (int)lua_tonumber(L, -1);
	lua_pop(L, 5);
	L_ = L;

	//타입별로 OOBB 노가다 해야함
	//SetOOBB(XMFLOAT3(Bounds.Center.x , Bounds.Center.y , Bounds.Center.z ), XMFLOAT3(Bounds.Extents.x, Bounds.Extents.y, Bounds.Extents.z), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
	if (NPC_SPIDER == npcType_)
		SetOOBB(XMFLOAT3(0.f, 0.3f, 0.7f), XMFLOAT3(1.f, 0.8f, 1.1f), XMFLOAT4(0.f, 0.f, 0.f, 1.f));
	else if (NPC_TURTLE == npcType_)
		SetOOBB(XMFLOAT3(0.2f, 11.6f, 18.1f), XMFLOAT3(23.2f, 39.6f, 19.9f), XMFLOAT4(0.f, 0.f, 0.f, 1.f));
	else if (NPC_NAGA_GUARD == npcType_)
		SetOOBB(XMFLOAT3(-10.8f, 6.4f, 43.5f), XMFLOAT3(28.8f, 31.7f, 41.1f), XMFLOAT4(0.f, 0.f, 0.f, 1.f));
	else if (NPC_TREE_GUARD == npcType_)
		SetOOBB(XMFLOAT3(3.2f, 6.8f, 50.8f), XMFLOAT3(47.9f, 30.9f, 50.7f), XMFLOAT4(0.f, 0.f, 0.f, 1.f));
	else if (NPC_ROCK_WARRIOR == npcType_)
		SetOOBB(XMFLOAT3(0.f, -5.2f, 39.8f), XMFLOAT3(46.6f, 22.5f, 39.6f), XMFLOAT4(0.f, 0.f, 0.f, 1.f));

	xmOOBBTransformed_.Transform(xmOOBB_, XMLoadFloat4x4(&world_));
	XMStoreFloat4(&xmOOBBTransformed_.Orientation, XMQuaternionNormalize(XMLoadFloat4(&xmOOBBTransformed_.Orientation)));

	objectType_ = TYPE_MONSTER;
}

void display_error(lua_State* L, int errnum)
{
	printf("Erorr : %s\n", lua_tostring(L, -1));
	lua_pop(L, 1);
}

