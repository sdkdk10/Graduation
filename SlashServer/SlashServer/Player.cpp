#include "Player.h"

Player::Player()
{
}

Player::~Player()
{
}

void Player::Initialize()
{
	GameObject::Initialize();

	exover_.eventType = EVT_RECV;
	exover_.wsaBuf.buf = exover_.ioBuf;
	exover_.wsaBuf.len = sizeof(exover_.ioBuf);
	packetSize_ = 0;
	prevSize_ = 0;
	rollDir = 0;
	rollCurFrame = 0;
	isWarriorUltimateMode = false;

	XMStoreFloat4x4(&world_, XMMatrixScaling(0.05f, 0.05f, 0.05f)*XMMatrixRotationX(1.7f)*XMMatrixRotationZ(3.14f)*XMMatrixTranslation(0.0f, 0.0f, 0.0f));
	world_._41 = INIT_PLAYER_POS.x, world_._42 = INIT_PLAYER_POS.y, world_._43 = INIT_PLAYER_POS.z;
	hp_ = INIT_PLAYER_HP;
	dmg_ = INIT_PLAYER_DMG;
	objectType_ = TYPE_PLAYER;
	playerType_ = PLAYER_WARRIOR;
	SetOOBB(XMFLOAT3(-7.5f, -5.9f, 28.8f), XMFLOAT3(23.1f, 16.4f, 28.5f), XMFLOAT4(0.f, 0.f, 0.f, 1.f));
}
