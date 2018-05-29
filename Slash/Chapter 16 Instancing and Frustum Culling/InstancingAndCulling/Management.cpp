#include "stdafx.h"
#include "Management.h"
#include "Renderer.h"

IMPLEMENT_SINGLETON(CManagement)

CManagement::CManagement()
	: m_pRenderer(nullptr)
	, m_pCurScene(nullptr)
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


void CManagement::Init_Management(CRenderer* pRenderer)
{
	m_pRenderer = pRenderer;
}

bool CManagement::Update(const GameTimer & gt, const FrameResource* pCruRrc)
{
	mCurrFrameResource = const_cast<FrameResource*>(pCruRrc);

	if (false == m_pCurScene->Update(gt))
		return false;

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

	return S_OK;
}

HRESULT CManagement::Set_Sound(CSound * pSound)
{
	if (pSound == nullptr)
		return E_FAIL;
	m_pSound = pSound;
}

void CManagement::Free()
{
	Safe_Release(m_pCurScene);
	Safe_Release(m_pRenderer);
	Safe_Release(m_pSound);
}