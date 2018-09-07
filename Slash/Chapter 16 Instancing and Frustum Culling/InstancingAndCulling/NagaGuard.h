#pragma once

#include "GameObject.h"
#include "../../SlashServer/SlashServer/Protocol.h"

class Player;

class NagaGuard : public CGameObject
{
private:
	const float m_fScale = 0.08f;
private:
	NagaGuard(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice, ComPtr<ID3D12DescriptorHeap> &srv, UINT srvSize);
public:
	virtual ~NagaGuard();
public:
	virtual bool			Update(const GameTimer & gt);
	virtual void			Render(ID3D12GraphicsCommandList* cmdList);
	virtual HRESULT			Initialize();
	virtual void			Animate(const GameTimer & gt);
	virtual void			SaveSlidingVector(CGameObject * pobj, CGameObject * pCollobj);

public:
	void SetTexture(SpiderType _tex);

private:
	Player * m_pPlayer = nullptr;
	string					m_strTexName[SPIDER_END];

public:
	static NagaGuard* Create(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice, ComPtr<ID3D12DescriptorHeap> &srv, UINT srvSize);

};

