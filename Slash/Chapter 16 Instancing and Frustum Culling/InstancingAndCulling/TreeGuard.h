#pragma once

#include "GameObject.h"
#include "../../SlashServer/SlashServer/Protocol.h"

class Player;

class TreeGuard : public CGameObject
{
private:
	const float m_fScale = 0.07f;

private:
	TreeGuard(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice, ComPtr<ID3D12DescriptorHeap> &srv, UINT srvSize);
public:
	virtual ~TreeGuard();
public:
	virtual bool			Update(const GameTimer & gt);
	virtual void			Render(ID3D12GraphicsCommandList* cmdList);
	virtual HRESULT			Initialize();
	virtual void			Animate(const GameTimer & gt);
	virtual void			SaveSlidingVector(CGameObject * pobj, CGameObject * pCollobj);

public:
	void SetTexture(SpiderTex _tex);

private:
	Player * m_pPlayer = nullptr;
	string					m_strTexName[SPIDER_END];

public:
	static TreeGuard* Create(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice, ComPtr<ID3D12DescriptorHeap> &srv, UINT srvSize);

};

