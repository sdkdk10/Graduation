#include "GameObject.h"
#include "Protocol.h"

GameObject::GameObject()
{
}

GameObject::~GameObject()
{
}

void GameObject::SetOOBB(const XMFLOAT3 & xmCenter, const XMFLOAT3 & xmExtents, const XMFLOAT4 & xmOrientation)
{
	xmOOBBTransformed_ = xmOOBB_ = BoundingOrientedBox(xmCenter, xmExtents, xmOrientation);
}

void GameObject::SetSkillOOBB(const XMFLOAT3 & xmCenter, const XMFLOAT3 & xmExtents, const XMFLOAT4 & xmOrientation)
{
	skillOOBBTransformed_ = skillOOBB_ = BoundingOrientedBox(xmCenter, xmExtents, xmOrientation);
}

void GameObject::Initialize()
{
	lookDegree_ = 0;
	state_ = STATE_IDLE;
	isActive_ = false;
	level_ = 1;
	exp_ = 0;
}

bool GameObject::CanSeeMapObject(GameObject* b)
{
	float dist_sq = (world_._41 - b->world_._41) * (world_._41 - b->world_._41)
		+ (world_._43 - b->world_._43) * (world_._43 - b->world_._43);
	return (dist_sq <= MAPOBJECT_RADIUS * MAPOBJECT_RADIUS);
}

bool GameObject::CanSee(GameObject* b)
{
	float dist_sq = (world_._41 - b->world_._41) * (world_._41 - b->world_._41)
		+ (world_._43 - b->world_._43) * (world_._43 - b->world_._43);
	return (dist_sq <= VIEW_RADIUS * VIEW_RADIUS);
}

bool GameObject::IsClose(GameObject* b)
{
	float dist_sq = (world_._41 - b->world_._41) * (world_._41 - b->world_._41)
		+ (world_._43 - b->world_._43) * (world_._43 - b->world_._43);
	return (dist_sq <= CLOSE_RADIUS * CLOSE_RADIUS);
}

bool GameObject::InWarriorAttack1Range(GameObject* b)
{
	float dist_sq = (world_._41 - b->world_._41) * (world_._41 - b->world_._41)
		+ (world_._43 - b->world_._43) * (world_._43 - b->world_._43);
	return (dist_sq <= PLAYER_ATTACK_RADIUS * PLAYER_ATTACK_RADIUS);
}
bool GameObject::IsInAgroRange(GameObject* b)
{
	float dist_sq = (world_._41 - b->world_._41) * (world_._41 - b->world_._41)
		+ (world_._43 - b->world_._43) * (world_._43 - b->world_._43);
	return (dist_sq <= AGRO_RADIUS * AGRO_RADIUS);
}