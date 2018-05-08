#pragma once

#include "GameObject.h"
class Camera;
class CMesh;

constexpr      unsigned long MAXINSTOBJECTID = 100000;

class CInstancingObject
	: public CGameObject
{
	typedef struct objdrawelement
	{
		UINT IndexCount = 0;
		UINT StartIndexLocation = 0;
		int BaseVertexLocation = 0;
	}DrawElement;


public:
	explicit CInstancingObject(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice, ComPtr<ID3D12DescriptorHeap> &srv, UINT srvSize, wchar_t* pMesh, int iSize);
	virtual ~CInstancingObject();

public:
	void SetTexture(int matIdx, int idx) { mMaterials[matIdx]->DiffuseSrvHeapIndex = idx; }

public:
	virtual HRESULT			Initialize();
	virtual bool			Update(const GameTimer & gt);
	virtual void			Render(ID3D12GraphicsCommandList* cmdList);

public:
	std::vector<InstanceData> GetvecInstances() { return vecInstances; }
	virtual CTransform* GetTransform(int idx = 0) { if (idx > m_iSize) return nullptr; return m_vecTransCom[idx]; }
private:
	UINT						InstanceCount;
	std::vector<InstanceData>	vecInstances;


	//std::unordered_map<std::string, std::unique_ptr<Material>> mMaterials;
	vector<std::unique_ptr<Material>>			mMaterials;

	MeshGeometry*				m_GeoBounds;

	int							m_iSize;

	vector<CTransform*>			m_vecTransCom;

	static unsigned long		m_iAllInstObjectIndex;
	static CInstancingObject*	m_pAllInstObject[MAXINSTOBJECTID];

	unsigned long				m_iMyInstObject;

public:
	static CInstancingObject* Create(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice, ComPtr<ID3D12DescriptorHeap> &srv, UINT srvSize, wchar_t* pMesh, int iSize);


	DrawElement Element_Bounds;
	virtual void			RenderBounds(ID3D12GraphicsCommandList* cmdList);
private:
	virtual void			Free();
};