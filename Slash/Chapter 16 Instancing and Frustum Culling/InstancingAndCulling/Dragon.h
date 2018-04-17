#pragma once

#include "GameObject.h"

class Dragon : public CGameObject
{
private:
	const float m_fScale = 0.1f;
private:
	Dragon(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice, ComPtr<ID3D12DescriptorHeap> &srv, UINT srvSize);
public:
	virtual ~Dragon();
public:
	virtual bool			Update(const GameTimer & gt);
	virtual void			Render(ID3D12GraphicsCommandList* cmdList);
	virtual HRESULT			Initialize();

public:
	static Dragon* Create(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice, ComPtr<ID3D12DescriptorHeap> &srv, UINT srvSize);

};

