#include "stdafx.h"
#include "Management.h"
#include "Renderer.h"
#include "NumUI.h"

IMPLEMENT_SINGLETON(CManagement)

CManagement::CManagement()
	: m_pRenderer(nullptr)
	, m_pCurScene(nullptr)
	, m_pNumUI(nullptr)
{

}

CManagement::~CManagement()
{
}

CLayer* CManagement::Get_Layer(wchar_t* pLayerTag)
{
	// TODO: 여기에 반환 구문을 삽입합니다.
	return m_pCurScene->Get_Layer(pLayerTag);
}

CGameObject * CManagement::Find_Object(wchar_t * LayerTag, unsigned int iIdx)
{
	return m_pCurScene->Find_Object(LayerTag, iIdx);
}

HRESULT CManagement::Add_NumUI(int iNum, XMFLOAT3 f3Pos, bool isCritical)
{
	m_pNumUI->Add(iNum, f3Pos, isCritical);

	return S_OK;
}


HRESULT CManagement::Init_Management(CRenderer* pRenderer, NumUI* pNumUI)
{
	if (pRenderer == nullptr || pNumUI == nullptr)
		return E_FAIL;

	m_pRenderer = pRenderer;
	m_pNumUI = pNumUI;

	return S_OK;
}

bool CManagement::Update(const GameTimer & gt, const FrameResource* pCruRrc)
{
	mCurrFrameResource = const_cast<FrameResource*>(pCruRrc);

	if (false == m_pCurScene->Update(gt))
		return false;

	m_pNumUI->Update(gt);

	return true;

}

void CManagement::Render(ID3D12GraphicsCommandList * cmdList)
{
	m_pRenderer->Render(cmdList);
}

HRESULT CManagement::Change_Scene(CScene* pScene)
{
	if (nullptr == pScene)
		return E_FAIL;

	Safe_Release(m_pCurScene);

	m_pCurScene = pScene;

	m_pCurScene->Get_MainCam()->Set_Object(CManagement::GetInstance()->Get_CurScene()->Find_Object(L"Layer_Player", 0));

	return S_OK;
}

HRESULT CManagement::Set_Sound(CSound * pSound)
{
	if (pSound == nullptr)
		return E_FAIL;
	m_pSound = pSound;

	return S_OK;
}

void CManagement::Free()
{
	Safe_Release(m_pCurScene);
	Safe_Release(m_pRenderer);
	Safe_Release(m_pSound);
}