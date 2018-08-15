#pragma once

#include "UI.h"

class ChangeUI : public UI
{

public:
	ChangeUI(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice, ComPtr<ID3D12DescriptorHeap> &srv, UINT srvSize, XMFLOAT2 _move, XMFLOAT2 _scale, float _size, int diffuseSrvHeapIndex, float fZ = 0);
	~ChangeUI();
public:
	static ChangeUI* Create(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice, ComPtr<ID3D12DescriptorHeap> &srv, UINT srvSize, XMFLOAT2 move, XMFLOAT2 scale, float size, int diffuseSrvHeapIndex, float fZ = 0);

	virtual bool			Update(const GameTimer & gt);
	virtual void			SetUI(float size = 0.5f, float moveX = 0.0f, float moveY = 0.0f, float scaleX = 0.0f, float scaleY = 0.0f);
	virtual void			Render(ID3D12GraphicsCommandList* cmdList);
	virtual HRESULT			Initialize(XMFLOAT2 move, XMFLOAT2 scale, float size);

public:
	void		SetisChange(bool _isChange);
	void		SetChangeInfo(XMFLOAT4 _xm, float _time);

private:
	bool				m_IsChange = false;
	bool				m_ChangeValue = true;
	XMFLOAT4			m_xmf4ColorChange;
	XMFLOAT4			m_ChangeColor;
	float				m_fChangeTime = 0.f;
	float				m_fTimeAccc = 0.f;
	float				m_fZ = 0.f;
private:
	virtual void Free();
};

