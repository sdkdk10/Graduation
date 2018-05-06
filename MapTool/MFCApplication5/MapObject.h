#pragma once
#include "GameObject.h"

class CMapObject
	: public CGameObject
{
private:
	explicit CMapObject(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice, ComPtr<ID3D12DescriptorHeap>& srv, UINT srvSize, wchar_t* meshName);
public:
	virtual ~CMapObject();

public:
	//wchar_t* Get_MeshName() { return m_pwstrMeshName; }
	string Get_TexName() { return m_strTexName; }

public:
	virtual HRESULT Initialize();
	virtual bool	Update(const GameTimer& gt);
	virtual void	Render(ID3D12GraphicsCommandList* cmdList);

public:
	void SetMesh(wchar_t* meshName);
	void SetTexture(Texture* tex);
	void SetTexture(string texName);
	void SetTexture(int idx) { Mat->DiffuseSrvHeapIndex = idx; }
public:
	static CMapObject* Create(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice, ComPtr<ID3D12DescriptorHeap>& srv, UINT srvSize, wchar_t* meshName);

//private:
//	wchar_t*				m_wstrMeshName;
private:
	string					m_strTexName;

public:
	virtual void Free();
};