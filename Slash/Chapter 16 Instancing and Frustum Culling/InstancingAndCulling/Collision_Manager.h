#pragma once

#include "Component.h"
#include "Define.h"


class CGameObject;

class Collision_Manager
	: public CComponent
{
public:
	virtual CComponent* Clone(void) = 0;
	virtual void Update_Component(const CGameTimer& gt);

	void CollisionDetect(CGameObject* pObj1, CGameObject* pObj2);
public:
	explicit Collision_Manager();

	~Collision_Manager();
};

