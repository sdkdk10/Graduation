#pragma once

#include "Scene.h"

class CTestScene
	: public CScene
{
private:
	explicit CTestScene(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice, vector<ComPtr<ID3D12DescriptorHeap>> &srv, UINT srvSize);
public:
	virtual ~CTestScene();

public:
	virtual HRESULT Initialize();
	virtual bool	Update(const GameTimer& gt);
	virtual void	Render(ID3D12GraphicsCommandList* cmdList);
	virtual void	CollisionProcess();
private:
	Microsoft::WRL::ComPtr<ID3D12Device>		m_d3dDevice;

	vector<ComPtr<ID3D12DescriptorHeap>> mSrvDescriptorHeap;
	UINT mCbvSrvDescriptorSize = 0;

public:
	static CTestScene* Create(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice, vector<ComPtr<ID3D12DescriptorHeap>> &srv, UINT srvSize);

private:
	virtual void Free();
};