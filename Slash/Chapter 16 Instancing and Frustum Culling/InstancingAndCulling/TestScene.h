#pragma once

#include "Scene.h"

typedef struct uiInfo
{
	float fSize = 0.0f;
	float fMoveX = 0.0f;
	float fMoveY = 0.0f;
	float fScale = 0.0f;
}UIInfo;

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
	void UpdateOOBB();
	void UpdateUI();
	virtual void	Render(ID3D12GraphicsCommandList* cmdList);
	virtual void	CollisionProcess();
	
public:
	int			GetObjectCount() { return m_iObjectCount; }
private:
	int				m_iObjectCount = 0;

private:
	HRESULT Load_Map();

public:
	static CTestScene* Create(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice, vector<ComPtr<ID3D12DescriptorHeap>> &srv, UINT srvSize);

	void UISetting();
private:
	virtual void Free();
private:
	vector<pair<const string, const string>> path;
};