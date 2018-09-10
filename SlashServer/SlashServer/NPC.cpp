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
	else if(NPC_ROCK_WARRIOR == npcType_)
		XMStoreFloat4x4(&world_, XMMatrixScaling(0.08f, 0.08f, 0.08f)*XMMatrixRotationX(1.7f)*XMMatrixRotationZ(3.14f)*XMMatrixTranslation(0.0f, 0.0f, 0.0f));
	else if(NPC_TREE_GUARD == npcType_)
		XMStoreFloat4x4(&world_, XMMatrixScaling(0.07f, 0.07f, 0.07f)*XMMatrixRotationX(1.7f)*XMMatrixRotationZ(3.14f)*XMMatrixTranslation(0.0f, 0.0f, 0.0f));
	else if(NPC_TURTLE == npcType_)
		XMStoreFloat4x4(&world_, XMMatrixScaling(0.05f, 0.05f, 0.05f)*XMMatrixRotationX(1.7f)*XMMatrixRotationZ(3.14f)*XMMatrixTranslation(0.0f, 0.0f, 0.0f));
	else if (NPC_DRAGON == npcType_)
		XMStoreFloat4x4(&world_, XMMatrixScaling(0.1f, 0.1f, 0.1f)*XMMatrixRotationX(1.7f)*XMMatrixRotationZ(3.14f)*XMMatrixTranslation(0.0f, 0.0f, 0.0f));

	lua_getglobal(L, "LoadMonsterData");
	lua_pushnumber(L, static_cast<int>(npcType_));
	lua_pcall(L, 1, 4, 0);
	world_._41 = static_cast<float>(lua_tonumber(L, -4));
	world_._42 = 0;
	world_._43 = static_cast<float>(lua_tonumber(L, -3));
	hp_ = (int)lua_tonumber(L, -2);
	dmg_ = (int)lua_tonumber(L, -1);
	lua_pop(L, 4);
	L_ = L;

	//타입별로 OOBB 노가다 해야함
	//SetOOBB(XMFLOAT3(Bounds.Center.x , Bounds.Center.y , Bounds.Center.z ), XMFLOAT3(Bounds.Extents.x, Bounds.Extents.y, Bounds.Extents.z), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));

	SetOOBB(XMFLOAT3(0.f, 0.3232f, 0.7277f), XMFLOAT3(1.0345f, 0.7848f, 1.0931f), XMFLOAT4(0.f, 0.f, 0.f, 1.f));

	xmOOBBTransformed_.Transform(xmOOBB_, XMLoadFloat4x4(&world_));
	XMStoreFloat4(&xmOOBBTransformed_.Orientation, XMQuaternionNormalize(XMLoadFloat4(&xmOOBBTransformed_.Orientation)));

	objectType_ = TYPE_MONSTER;
	isActive_ = false;
}

void display_error(lua_State* L, int errnum)
{
	printf("Erorr : %s\n", lua_tostring(L, -1));
	lua_pop(L, 1);
}

