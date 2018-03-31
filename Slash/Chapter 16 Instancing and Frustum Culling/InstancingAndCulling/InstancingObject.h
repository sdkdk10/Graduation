#pragma once

#include "GameObject.h"
class Camera;

class CInstancingObject
	: public CGameObject
{
public:
	explicit CInstancingObject(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice, ComPtr<ID3D12DescriptorHeap> &srv, UINT srvSize);
	virtual ~CInstancingObject();

public:
	virtual HRESULT			Initialize();
	virtual bool			Update(const GameTimer & gt);
	virtual void			Render(ID3D12GraphicsCommandList* cmdList);

public:
	void					SetCamera(Camera* pCam) { m_pCamera = pCam; }
	void					SetCamFrustum(BoundingFrustum frustum) { mCamFrustum = frustum; }

private:
	UINT						InstanceCount;
	std::vector<InstanceData>	vecInstances;
	BoundingBox					Bounds;

	Camera*						m_pCamera;
	BoundingFrustum				mCamFrustum;
	bool						mFrustumCullingEnabled = true;

	std::unordered_map<std::string, std::unique_ptr<Material>> mMaterials;

public:
	static CInstancingObject* Create(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice, ComPtr<ID3D12DescriptorHeap> &srv, UINT srvSize);

private:
	virtual void			Free();
};