#pragma once
#include "Base.h"
#include "GameObject.h"

class Camera;
class CLayer;

class CScene
	: public CBase
{
protected:
	explicit CScene();
	virtual ~CScene();

public:
	Camera*			Get_MainCam() { return m_pMainCam; }
	HRESULT			Set_MainCam(Camera* pCam) { if (nullptr == pCam) return E_FAIL; m_pMainCam = pCam;  return S_OK; }
	HRESULT			Set_CamFrustum(BoundingFrustum* pFrustum) { m_pCamFrustum = pFrustum; return S_OK; }
	BoundingFrustum*	Get_CamFrustum() { return m_pCamFrustum; }
	auto&			Get_MapLayer() { return m_mapLayer; }
	CGameObject*	Find_Object(wchar_t* LayerTag, unsigned int iIdx = 0);


protected:
	Microsoft::WRL::ComPtr<ID3D12Device>	m_d3dDevice;

	unordered_map<wchar_t*, CLayer*>		m_mapLayer;
	Camera*									m_pMainCam;		// ���� ���� ���� ī�޶�
	BoundingFrustum*						m_pCamFrustum;



public:
	virtual HRESULT Initialize();
	virtual bool	Update(const GameTimer& gt);
	virtual void	Render(ID3D12GraphicsCommandList* cmdList);

public:
	HRESULT			Add_Layer(wchar_t* pLayerTag, CLayer* pLayer);
	CLayer*			Find_Layer(wchar_t* pLayerTag);

	HRESULT			Ready_GameObject(wchar_t* pLayerTag, CGameObject* pObj);
	HRESULT			Ready_Layer(wchar_t* pLayerTag);

private:
	virtual void	Free();

};