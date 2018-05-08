#pragma once

#include "GameObject.h"

class Player;

class Spider : public CGameObject
{
private:
	const float m_fScale = 2.0f;
private:
	Spider(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice, ComPtr<ID3D12DescriptorHeap> &srv, UINT srvSize);
public:
	virtual ~Spider();
public:
	virtual bool			Update(const GameTimer & gt);
	virtual void			Render(ID3D12GraphicsCommandList* cmdList);
	virtual HRESULT			Initialize();
	virtual void			Animate(const GameTimer & gt);

private:
	Player *				m_pPlayer = nullptr;
public:
	static Spider* Create(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice, ComPtr<ID3D12DescriptorHeap> &srv, UINT srvSize);

};

