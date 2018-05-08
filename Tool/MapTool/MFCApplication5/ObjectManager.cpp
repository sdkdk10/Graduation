#include "stdafx.h"
#include "ObjectManager.h"
#include "Renderer.h"
#include "GameObject.h"

IMPLEMENT_SINGLETON(CObjectManager);

CObjectManager::CObjectManager()
	: m_pRenderer(nullptr)
{

}

CObjectManager::~CObjectManager()
{
}

void CObjectManager::Clear_Object()
{
	size_t iSize = m_vecObject.size();

	for (size_t i = 0; i < iSize; ++i)
		Safe_Release(m_vecObject[i]);
	m_vecObject.clear();

	m_pRenderer->Clear_Renderer();
}

CGameObject * CObjectManager::Find_Object(unsigned int iIdx)
{
	if(iIdx > m_vecObject.size() - 1)
		return nullptr;

	return m_vecObject[iIdx];
}

void CObjectManager::Delete_Object(unsigned int iIdx)
{
	size_t iSize = m_vecObject.size();

	for (size_t i = 0; i < iSize; ++i)
	{
		if (m_vecObject[i]->GetMyID() == iIdx)
		{
			Erase_Vector_Element(m_vecObject, i);
			return;
		}
	}
}

HRESULT CObjectManager::Add_Object(CGameObject * pObj)
{
	if (pObj == nullptr)
		return E_FAIL;

	m_vecObject.push_back(pObj);

	return S_OK;
}

HRESULT CObjectManager::Init_ObjMgr(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice, CRenderer* pRenderer)
{
	m_d3dDevice = d3dDevice;
	m_pRenderer = pRenderer;
	return S_OK;
}

int CObjectManager::Update_ObjMgr(const GameTimer & gt, const FrameResource* pCurRsc)
{
	mCurrFrameResource = const_cast<FrameResource*>(pCurRsc);
	size_t iSize = m_vecObject.size();

	for (size_t i = 0; i < iSize; ++i)
		m_vecObject[i]->Update(gt);
	return 0;
}

void CObjectManager::Render_ObjMgr(ID3D12GraphicsCommandList* cmdList)
{
	m_pRenderer->Render(cmdList);
}

void CObjectManager::Free()
{
	size_t iSize = m_vecObject.size();
	for (size_t i = 0; i < iSize; ++i)
		Safe_Release(m_vecObject[i]);

	m_vecObject.clear();
}
