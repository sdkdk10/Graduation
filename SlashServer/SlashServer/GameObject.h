#pragma once
#include "Functor.h"

class GameObject
{
public:
	explicit GameObject();
	virtual ~GameObject();

public:
	void SetOOBB(const XMFLOAT3& xmCenter, const XMFLOAT3& xmExtents, const XMFLOAT4& xmOrientation);
	virtual void Initialize();

	bool CanSeeMapObject(GameObject * b);
	bool CanSee(GameObject * b);
	bool IsClose(GameObject * b);
	bool IsAttackRange(GameObject * b);
	bool IsInAgroRange(GameObject * b);

public:
	XMFLOAT4X4 world_;
	BoundingOrientedBox xmOOBB_;
	BoundingOrientedBox xmOOBBTransformed_;
	float lookDegree_;
	bool isActive_;
	BYTE state_;
	BYTE objectType_;
	int hp_;
	int dmg_;
	int ID_;
};