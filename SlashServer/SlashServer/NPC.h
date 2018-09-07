#pragma once
#include "GameObject.h"

// For Script
extern "C"
{
#include "include\lua.h"
#include "include\lauxlib.h"
#include "include\lualib.h"
}

class NPC : public GameObject
{
public:
	explicit NPC();
	virtual ~NPC();

public:
	virtual void Initialize();

public:
	lua_State * L_;
	BYTE npcType_;
};