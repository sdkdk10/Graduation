#pragma once

#include "Base.h"
#include "Define.h"
#include "GameTimer.h"
#include "Camera.h"
#include "Scene.h"

class CRenderer;

class CManagement
	: public CBase
{
	enum SCENETYPE{SCENE_LOADING, SCENE_LOGIN, SCENE_LOGO, SCENE_STAGE, SCENE_END};

	DECLARE_SINGLETON(CManagement)

private:
	explicit CManagement();
	virtual ~CManagement();

public:
	ComPtr<ID3D12DescriptorHeap>&	GetSrvDescriptorHeap() { return mSrvDescriptorHeap; }
	UINT&							GetCbvSrvDescriptorSize() { return mCbvSrvDescriptorSize; }
	FrameResource*					GetCurFrameResource() { return mCurrFrameResource; }
	CRenderer*						GetRenderer() { return m_pRenderer; }

public:
	CScene*			Get_CurScene() { return m_pCurScene; }
	Camera*			Get_MainCam() { return m_pCurScene->Get_MainCam(); }
	HRESULT			Set_MainCam(Camera* pCam) { return m_pCurScene->Set_MainCam(pCam); }
	HRESULT			Set_CamFrustum(DirectX::BoundingFrustum* pFrustum) { return m_pCurScene->Set_CamFrustum(pFrustum); }
	DirectX::BoundingFrustum* Get_CamFrustum() { return m_pCurScene->Get_CamFrustum(); }

	auto&			Get_MapLayer() { return m_pCurScene->Get_MapLayer(); }
	CGameObject*	Find_Object(wchar_t* LayerTag, unsigned int iIdx = 0);

private:
	CScene*				m_pCurScene;
	CRenderer*			m_pRenderer;

	ComPtr<ID3D12DescriptorHeap> mSrvDescriptorHeap = nullptr;
	UINT mCbvSrvDescriptorSize = 0;

	FrameResource* mCurrFrameResource = nullptr;

public:
	void		Init_Management(CRenderer* pRenderer);
	bool		Update(const GameTimer& gt, const FrameResource* pCruRrc);
	void		Render(ID3D12GraphicsCommandList* cmdList);

public:
	HRESULT		Change_Scene(CScene* pScene);

private:
	virtual void Free();

};