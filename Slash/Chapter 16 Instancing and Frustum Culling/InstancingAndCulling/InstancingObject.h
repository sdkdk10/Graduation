#pragma once

#include "GameObject.h"
class Camera;
class CMesh;


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
	explicit CInstancingObject(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice, ComPtr<ID3D12DescriptorHeap> &srv, UINT srvSize);
	virtual ~CInstancingObject();

public:
	virtual HRESULT			Initialize();
	virtual bool			Update(const GameTimer & gt);
	virtual void			Render(ID3D12GraphicsCommandList* cmdList);

public:
	std::vector<InstanceData> GetvecInstances() { return vecInstances; }
private:
	UINT						InstanceCount;
	std::vector<InstanceData>	vecInstances;


	std::unordered_map<std::string, std::unique_ptr<Material>> mMaterials;

	MeshGeometry*				m_GeoBounds;
	

public:
	static CInstancingObject* Create(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice, ComPtr<ID3D12DescriptorHeap> &srv, UINT srvSize);


	DrawElement Element_Bounds;
	virtual void			RenderBounds(ID3D12GraphicsCommandList* cmdList);
private:
	virtual void			Free();
};