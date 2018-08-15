#include "stdafx.h"
#include "SelectScene.h"
#include "Management.h"
#include "Renderer.h"
#include "Layer.h"
#include "Terrain.h"
#include "Transform.h"
#include "Sound.h"
#include "InputDevice.h"
#include "ChangeUI.h"
#include "Player.h"
#include "Texture_Manager.h"
#include "TestScene.h"

CSelectScene::CSelectScene(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice, vector<ComPtr<ID3D12DescriptorHeap>> &srv, UINT srvSize)
	: CScene(d3dDevice, srv, srvSize)
{

}

CSelectScene::~CSelectScene()
{
}

HRESULT CSelectScene::Initialize()
{
	XMFLOAT2 move;
	XMFLOAT2 scale;
	float size;

	move.x = -0.6f;
	move.y = 0.15f;

	scale.x = 0.7f;
	scale.y = 1.5f;

	size = 0.5f;

	Texture* tex = CTexture_Manager::GetInstance()->Find_Texture("SelectUI", CTexture_Manager::TEX_DEFAULT_2D);
	CGameObject* pObject = ChangeUI::Create(m_d3dDevice, mSrvDescriptorHeap[HEAP_DEFAULT], mCbvSrvDescriptorSize, move, scale, size, tex->Num, 0.01f);
	pObject->SetCamera(Get_MainCam());
	dynamic_cast<ChangeUI*>(pObject)->SetisChange(true);
	dynamic_cast<ChangeUI*>(pObject)->SetChangeInfo(XMFLOAT4(0.4f, 0.4f, 0.4f, 1.f), 1.5f);
	//dynamic_cast<CInstancingObject*>(pObject)->SetCamFrustum(mCamFrustum);
	Ready_GameObject(L"Layer_SelectSceneUI", pObject);
	CManagement::GetInstance()->GetRenderer()->Add_RenderGroup(CRenderer::RENDER_UICHANGE, pObject);
	m_vecSelect.push_back(dynamic_cast<ChangeUI*>(pObject));


	tex = CTexture_Manager::GetInstance()->Find_Texture("WarriorUITex", CTexture_Manager::TEX_DEFAULT_2D);
	move.x = -0.8f;
	move.y = 0.25f;
	scale.x = 0.5f;
	scale.y = 0.5f;
	pObject = ChangeUI::Create(m_d3dDevice, mSrvDescriptorHeap[HEAP_DEFAULT], mCbvSrvDescriptorSize, move, scale, size, tex->Num);
	pObject->SetCamera(Get_MainCam());
	//dynamic_cast<CInstancingObject*>(pObject)->SetCamFrustum(mCamFrustum);
	Ready_GameObject(L"Layer_SelectSceneUI", pObject);
	CManagement::GetInstance()->GetRenderer()->Add_RenderGroup(CRenderer::RENDER_UICHANGE, pObject);


	move.x = 0.6f;
	move.y = 0.15f;
	scale.x = 0.7f;
	scale.y = 1.5f;
	tex = CTexture_Manager::GetInstance()->Find_Texture("SelectUI", CTexture_Manager::TEX_DEFAULT_2D);
	pObject = ChangeUI::Create(m_d3dDevice, mSrvDescriptorHeap[HEAP_DEFAULT], mCbvSrvDescriptorSize, move, scale, size, tex->Num, 0.01f);
	pObject->SetCamera(Get_MainCam());
	dynamic_cast<ChangeUI*>(pObject)->SetChangeInfo(XMFLOAT4(0.4f, 0.4f, 0.4f, 1.f), 1.5f);
	//dynamic_cast<CInstancingObject*>(pObject)->SetCamFrustum(mCamFrustum);
	Ready_GameObject(L"Layer_SelectSceneUI", pObject);
	CManagement::GetInstance()->GetRenderer()->Add_RenderGroup(CRenderer::RENDER_UICHANGE, pObject);
	m_vecSelect.push_back(dynamic_cast<ChangeUI*>(pObject));

	tex = CTexture_Manager::GetInstance()->Find_Texture("WarriorUITex", CTexture_Manager::TEX_DEFAULT_2D);
	move.x = 0.8f;
	move.y = 0.25f;
	scale.x = 0.5f;
	scale.y = 0.5f;
	pObject = ChangeUI::Create(m_d3dDevice, mSrvDescriptorHeap[HEAP_DEFAULT], mCbvSrvDescriptorSize, move, scale, size, tex->Num);
	pObject->SetCamera(Get_MainCam());
	//dynamic_cast<CInstancingObject*>(pObject)->SetCamFrustum(mCamFrustum);
	Ready_GameObject(L"Layer_SelectSceneUI", pObject);
	CManagement::GetInstance()->GetRenderer()->Add_RenderGroup(CRenderer::RENDER_UICHANGE, pObject);


	tex = CTexture_Manager::GetInstance()->Find_Texture("PressEnter", CTexture_Manager::TEX_DEFAULT_2D);
	move.x = 0.f;
	move.y = -1.5f;
	scale.x = 0.7f;
	scale.y = 0.5f;
	pObject = ChangeUI::Create(m_d3dDevice, mSrvDescriptorHeap[HEAP_DEFAULT], mCbvSrvDescriptorSize, move, scale, size, tex->Num);
	pObject->SetCamera(Get_MainCam());
	dynamic_cast<ChangeUI*>(pObject)->SetisChange(true);
	dynamic_cast<ChangeUI*>(pObject)->SetChangeInfo(XMFLOAT4(1.f, 0.4f, 0.4f, 1.f), 1.5f);
	//dynamic_cast<CInstancingObject*>(pObject)->SetCamFrustum(mCamFrustum);
	Ready_GameObject(L"Layer_SelectSceneUI", pObject);
	CManagement::GetInstance()->GetRenderer()->Add_RenderGroup(CRenderer::RENDER_UICHANGE, pObject);


	//pObject = Player::Create(m_d3dDevice, mSrvDescriptorHeap[HEAP_DEFAULT], mCbvSrvDescriptorSize);
	//pObject->SetCamera(Get_MainCam());
	//Ready_GameObject(L"Layer_Player", pObject);
	//CManagement::GetInstance()->GetRenderer()->Add_RenderGroup(CRenderer::RENDER_NONALPHA_FORWARD, pObject);

	return S_OK;
}

bool CSelectScene::Update(const GameTimer & gt)
{
	CScene::Update(gt);

	if (KeyBoard_Input(DIK_X) == CInputDevice::INPUT_DOWN)
		CManagement::GetInstance()->GetSound()->PlayEffect(L"Sound", L"sfx_alice_death_fall01");

	if (KeyBoard_Input(DIK_RIGHT) == CInputDevice::INPUT_PRESS)
	{
		m_isLeft = false;
		m_vecSelect[0]->SetisChange(false);
		m_vecSelect[1]->SetisChange(true);
	}

	if (KeyBoard_Input(DIK_LEFT) == CInputDevice::INPUT_PRESS)
	{
		m_isLeft = true;
		m_vecSelect[0]->SetisChange(true);
		m_vecSelect[1]->SetisChange(false);
	}

	if (KeyBoard_Input(DIK_RETURN) == CInputDevice::INPUT_PRESS)
	{
		CScene* pScene = CTestScene::Create(m_d3dDevice, mSrvDescriptorHeap, mCbvSrvDescriptorSize);

		pScene->Set_MainCam(m_pMainCam);
		pScene->Set_CamFrustum(m_pCamFrustum);
		m_pMainCam->Set_Dynamic(false);
		CManagement::GetInstance()->Change_Scene(pScene);
	}
	return true;
}

void CSelectScene::Render(ID3D12GraphicsCommandList * cmdList)
{
}

CSelectScene * CSelectScene::Create(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice, vector<ComPtr<ID3D12DescriptorHeap>>& srv, UINT srvSize)
{
	CSelectScene* pInstance = new CSelectScene(d3dDevice, srv, srvSize);
	if (FAILED(pInstance->Initialize()))
	{
		Safe_Release(pInstance);
		MSG_BOX(L"CSelectScene Creatd Failed");
	}

	return pInstance;
}

void CSelectScene::Free()
{
	CScene::Free();
}
