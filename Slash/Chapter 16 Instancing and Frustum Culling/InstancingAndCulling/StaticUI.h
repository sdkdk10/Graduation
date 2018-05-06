#pragma once

#include "UI.h"

class StaticUI : public UI
{

public:
	StaticUI(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice, ComPtr<ID3D12DescriptorHeap> &srv, UINT srvSize, XMFLOAT2 _move, XMFLOAT2 _scale, float _size , int diffuseSrvHeapIndex);
	~StaticUI();
public:
	static StaticUI* Create(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice, ComPtr<ID3D12DescriptorHeap> &srv, UINT srvSize, XMFLOAT2 move, XMFLOAT2 scale, float size , int diffuseSrvHeapIndex);

	virtual bool			Update(const GameTimer & gt);
	virtual void			SetUI(float size = 0.5f, float moveX = 0.0f, float moveY = 0.0f, float scaleX = 0.0f, float scaleY = 0.0f);
	virtual void			Render(ID3D12GraphicsCommandList* cmdList);
	virtual HRESULT			Initialize(XMFLOAT2 move, XMFLOAT2 scale, float size);


private:
	virtual void Free();
};

