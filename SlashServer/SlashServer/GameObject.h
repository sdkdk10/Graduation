#pragma once
#include "Functor.h"
#include <mutex>
#include <unordered_set>

class GameObject
{
public:
	explicit GameObject();
	virtual ~GameObject();

public:
	virtual void Initialize();

	void SetOOBB(const XMFLOAT3& xmCenter, const XMFLOAT3& xmExtents, const XMFLOAT4& xmOrientation);

	void SetSkillOOBB(const XMFLOAT3 & xmCenter, const XMFLOAT3 & xmExtents, const XMFLOAT4 & xmOrientation);

	bool CanSeeMapObject(GameObject * b);
	bool CanSee(GameObject * b);
	bool IsClose(GameObject * b);
	bool InWarriorAttack1Range(GameObject * b);
	bool IsInAgroRange(GameObject * b);

public:
	XMFLOAT4X4 world_;
	BoundingOrientedBox xmOOBB_;
	BoundingOrientedBox xmOOBBTransformed_;
	double lookDegree_;
	bool isActive_;
	BYTE state_;
	BYTE objectType_;
	int hp_;
	int dmg_;
	int ID_;
	int skillMoveRange;
	BoundingOrientedBox skillOOBB_{};
	BoundingOrientedBox skillOOBBTransformed_{};
	unsigned int exp_;
	unsigned short level_;
	std::unordered_set<GameObject*> viewList_;
	std::mutex vlm_;
};