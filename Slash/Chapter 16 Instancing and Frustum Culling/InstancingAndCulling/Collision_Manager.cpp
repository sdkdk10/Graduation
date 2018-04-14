#include "stdafx.h"
#include "Collision_Manager.h"
#include "GameObject.h"

void Collision_Manager::Update_Component(const CGameTimer & gt)
{
}

void Collision_Manager::CollisionDetect(CGameObject* pObj1, CGameObject* pObj2)
{
	BoundingBox pBound1 = pObj1->GetBounds();
	BoundingBox pBound2 = pObj2->GetBounds();

	
	
	
}

Collision_Manager::Collision_Manager()
{
}


Collision_Manager::~Collision_Manager()
{
}
