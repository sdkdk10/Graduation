#include "stdafx.h"
#include "TestScene.h"
#include "Define.h"
#include "GameObject.h"
#include "Player.h"
#include "Barrel.h"
#include "SkyBox.h"
#include "InstancingObject.h"
#include "Spider.h"
#include "Layer.h"
#include "Management.h"
#include "Renderer.h"
#include "Terrain.h"
//#include "Collision_Manager.h"
#include "Dragon.h"
#include "Npc.h"
#include "Network.h"
#include "UI.h"
#include "HPBar.h"
#include "StaticUI.h"
#include "Mesh.h"
#include "MapObject.h"
#include "Texture_Manager.h"
#include "Transform.h"


CTestScene::CTestScene(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice, vector<ComPtr<ID3D12DescriptorHeap>> &srv, UINT srvSize)
	: m_d3dDevice(d3dDevice)
	, mSrvDescriptorHeap(srv)
	, mCbvSrvDescriptorSize(srvSize)
{

}

CTestScene::~CTestScene()
{
}

HRESULT CTestScene::Initialize()
{
	CGameObject* pObject = SkyBox::Create(m_d3dDevice, mSrvDescriptorHeap[HEAP_DEFAULT], mCbvSrvDescriptorSize);
	Ready_GameObject(L"Layer_SkyBox", pObject);
	CManagement::GetInstance()->GetRenderer()->Add_RenderGroup(CRenderer::RENDER_PRIORITY, pObject);

	// 서버 실행시 주석시작

	pObject = Player::Create(m_d3dDevice, mSrvDescriptorHeap[HEAP_DEFAULT], mCbvSrvDescriptorSize);
	pObject->SetCamera(Get_MainCam());
	Ready_GameObject(L"Layer_Player", pObject);
	CManagement::GetInstance()->GetRenderer()->Add_RenderGroup(CRenderer::RENDER_NONALPHA_FORWARD, pObject);

	pObject = CNpc::Create(m_d3dDevice, mSrvDescriptorHeap[HEAP_DEFAULT], mCbvSrvDescriptorSize, L"Com_Mesh_Mage");
	pObject->SetCamera(Get_MainCam());
	Ready_GameObject(L"Layer_NPC", pObject);
	CManagement::GetInstance()->GetRenderer()->Add_RenderGroup(CRenderer::RENDER_NONALPHA_FORWARD, pObject);
	
	// 주석종료


//	pObject = Spider::Create(m_d3dDevice, mSrvDescriptorHeap[HEAP_DEFAULT], mCbvSrvDescriptorSize);
//	pObject->SetCamera(Get_MainCam());
//	pObject->SetPosition(0,0,0);
////	pObject->SetOOBB(XMFLOAT3(pObject->GetBounds().Center.x + pObject->GetWorld()._41, pObject->GetBounds().Center.y + pObject->GetWorld()._42, pObject->GetBounds().Center.z +	pObject->GetWorld()._43), XMFLOAT3(pObject->GetBounds().Extents.x, pObject->GetBounds().Extents.y, pObject->GetBounds().Extents.z), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
//	
//	Ready_GameObject(L"Layer_Spider", pObject);
//	CManagement::GetInstance()->GetRenderer()->Add_RenderGroup(CRenderer::RENDER_NONALPHA_FORWARD, pObject);
//	
	pObject = Spider::Create(m_d3dDevice, mSrvDescriptorHeap[HEAP_DEFAULT], mCbvSrvDescriptorSize);
	pObject->SetCamera(Get_MainCam());
	pObject->SetPosition(0, 0, 10);
//	pObject->SetOOBB(XMFLOAT3(pObject->GetBounds().Center.x + pObject->GetWorld()._41, pObject->GetBounds().Center.y + pObject->GetWorld()._42, pObject->GetBounds().Center.z + pObject->GetWorld()._43), XMFLOAT3(pObject->GetBounds().Extents.x, pObject->GetBounds().Extents.y, pObject->GetBounds().Extents.z), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));

	Ready_GameObject(L"Layer_Spider", pObject);
	CManagement::GetInstance()->GetRenderer()->Add_RenderGroup(CRenderer::RENDER_NONALPHA_FORWARD, pObject);

	
	/*for (int i = 0; i < 20; ++i)
	{
		for (int j = 0; j < 20; ++j)
		{
			for (int k = 0; k < 20; ++k)
			{
				Ready_GameObject(L"Layer_Spider", pObject);
				CManagement::GetInstance()->GetRenderer()->Add_RenderGroup(CRenderer::RENDER_NONALPHA_FORWARD, pObject);

				pObject = Spider::Create(m_d3dDevice, mSrvDescriptorHeap[HEAP_DEFAULT], mCbvSrvDescriptorSize);
				pObject->SetCamera(Get_MainCam());
				pObject->SetPosition(i * 50, j * 50, k * 50);

				Ready_GameObject(L"Layer_Spider", pObject);
				CManagement::GetInstance()->GetRenderer()->Add_RenderGroup(CRenderer::RENDER_NONALPHA_FORWARD, pObject);
			}
		}
		
	}*/


	/*pObject = Spider::Create(m_d3dDevice, mSrvDescriptorHeap[HEAP_DEFAULT], mCbvSrvDescriptorSize);
	pObject->SetCamera(Get_MainCam());
	pObject->SetPosition(0, 0, -50);
	Ready_GameObject(L"Layer_Spider", pObject);
	CManagement::GetInstance()->GetRenderer()->Add_RenderGroup(CRenderer::RENDER_NONALPHA_FORWARD, pObject);*/

	/*pObject = Dragon::Create(m_d3dDevice, mSrvDescriptorHeap[HEAP_DEFAULT], mCbvSrvDescriptorSize);
	pObject->SetCamera(Get_MainCam());
	Ready_GameObject(L"Layer_Dragon", pObject);
	CManagement::GetInstance()->GetRenderer()->Add_RenderGroup(CRenderer::RENDER_NONALPHA_FORWARD, pObject);
*/
	
	pObject = Barrel::Create(m_d3dDevice, mSrvDescriptorHeap[HEAP_DEFAULT], mCbvSrvDescriptorSize);
	pObject->SetCamera(Get_MainCam());
	Ready_GameObject(L"Layer_Barrel", pObject);
	CManagement::GetInstance()->GetRenderer()->Add_RenderGroup(CRenderer::RENDER_NONALPHA_FORWARD, pObject);

	pObject = Terrain::Create(m_d3dDevice, mSrvDescriptorHeap[HEAP_DEFAULT], mCbvSrvDescriptorSize);
	pObject->SetCamera(Get_MainCam());
	Ready_GameObject(L"Layer_Terrain", pObject);
	CManagement::GetInstance()->GetRenderer()->Add_RenderGroup(CRenderer::RENDER_NONALPHA_FORWARD, pObject);

	//pObject = CInstancingObject::Create(m_d3dDevice, mSrvDescriptorHeap[HEAP_INSTANCING], mCbvSrvDescriptorSize, L"Com_Mesh_Barrel", 5);//, "Models/StaticMesh/staticMesh.ASE", 10);
	//pObject->SetCamera(Get_MainCam());
	////dynamic_cast<CInstancingObject*>(pObject)->SetCamFrustum(mCamFrustum);
	//Ready_GameObject(L"Layer_Instance", pObject);
	//CManagement::GetInstance()->GetRenderer()->Add_RenderGroup(CRenderer::RENDER_NONALPHA_INSTANCING, pObject);

	UISetting();

	
	if (FAILED(Load_Map()))
		return E_FAIL;

	return S_OK;
}

bool CTestScene::Update(const GameTimer & gt)
{
	CScene::Update(gt);

	CollisionProcess();
	UpdateOOBB();
	UpdateUI();

	return true;
}

void CTestScene::UpdateOOBB()
{
	auto * m_pPlayer = CManagement::GetInstance()->Find_Object(L"Layer_Player");
	auto * m_pBarrel = CManagement::GetInstance()->Find_Object(L"Layer_Barrel");
	auto * m_pInstance = CManagement::GetInstance()->Find_Object(L"Layer_Map");
	auto * m_pSpider = CManagement::GetInstance()->Find_Object(L"Layer_Spider");
	auto * m_pSpider2 = CManagement::GetInstance()->Find_Object(L"Layer_Spider",1);
	//auto * m_pDragon = CManagement::GetInstance()->Find_Object(L"Layer_Dragon");

	//플레이어 없는 관계로 주석
	m_pPlayer->m_xmOOBBTransformed.Transform(m_pPlayer->m_xmOOBB, XMLoadFloat4x4(&(m_pPlayer->GetWorld())));
	XMStoreFloat4(&m_pPlayer->m_xmOOBBTransformed.Orientation, XMQuaternionNormalize(XMLoadFloat4(&m_pPlayer->m_xmOOBBTransformed.Orientation)));


	//cout << m_pSpider->m_xmOOBB.Center.x << "\t" << m_pSpider->m_xmOOBB.Center.y << "\t" <<  m_pSpider->m_xmOOBB.Center.z << endl;

	/*auto mat1 = m_pSpider->GetWorld();
	m_pSpider->m_xmOOBBTransformed.Transform(m_pSpider->m_xmOOBB, XMLoadFloat4x4(&(m_pSpider->GetWorld())));
	XMStoreFloat4(&m_pSpider->m_xmOOBBTransformed.Orientation, XMQuaternionNormalize(XMLoadFloat4(&m_pSpider->m_xmOOBBTransformed.Orientation)));

	auto mat2 = m_pSpider2->GetWorld();

	m_pSpider2->m_xmOOBBTransformed.Transform(m_pSpider2->m_xmOOBB, XMLoadFloat4x4(&(m_pSpider2->GetWorld())));
	XMStoreFloat4(&m_pSpider2->m_xmOOBBTransformed.Orientation, XMQuaternionNormalize(XMLoadFloat4(&m_pSpider2->m_xmOOBBTransformed.Orientation)));*/


	/*m_pDragon->m_xmOOBBTransformed.Transform(m_pDragon->m_xmOOBB, XMLoadFloat4x4(&(m_pDragon->GetWorld())));
	XMStoreFloat4(&m_pDragon->m_xmOOBBTransformed.Orientation, XMQuaternionNormalize(XMLoadFloat4(&m_pDragon->m_xmOOBBTransformed.Orientation)));*/


}
void CTestScene::UpdateUI()
{
	auto * m_pPlayer = CManagement::GetInstance()->Find_Object(L"Layer_Player");
	auto * m_pHPBar = CManagement::GetInstance()->Find_Object(L"Layer_HPBar");

	m_pHPBar->SetHp(m_pPlayer->GetHp());

}
void CTestScene::Render(ID3D12GraphicsCommandList * cmdList)
{
}

void CTestScene::CollisionProcess()
{

	auto * m_pPlayer = CManagement::GetInstance()->Find_Object(L"Layer_Player");
	auto * m_pBarrel = CManagement::GetInstance()->Find_Object(L"Layer_Barrel");

	auto * m_pInstance = CManagement::GetInstance()->Find_Object(L"Layer_Map");
	//auto * m_pSpider = CManagement::GetInstance()->Find_Object(L"Layer_Spider");

	vector<CGameObject *> m_pSpider;
	for (int i = 0; i < 2; ++i)
	{
		m_pSpider.push_back(CManagement::GetInstance()->Find_Object(L"Layer_Spider", i));
	}

	//auto * m_pDragon = CManagement::GetInstance()->Find_Object(L"Layer_Dragon");


	auto instanceData = dynamic_cast<CInstancingObject*>(m_pInstance)->GetvecInstances();


	auto m_pCamera = CManagement::GetInstance()->Get_MainCam();


	for (int i = 0; i < instanceData.size(); ++i)
	{
		XMMATRIX world = XMLoadFloat4x4(&(instanceData[i].World));

		XMMATRIX invWorld = XMMatrixInverse(&XMMatrixDeterminant(world), world);

		// View space to the object's local space.

		BoundingOrientedBox mLocalPlayerBounds;	
		// Transform the camera frustum from view space to the object's local space.
		m_pPlayer->m_xmOOBB.Transform(mLocalPlayerBounds, invWorld);

		// Perform the box/frustum intersection test in local space.
		if (mLocalPlayerBounds.Intersects(m_pInstance->GetBounds()) != DirectX::DISJOINT)
		{

			cout << i << "번째 인스턴싱 오브젝트랑 충돌" << endl;
		}

	}


	//if (m_pPlayer->m_xmOOBB.Intersects(m_pBarrel->m_xmOOBB))
	//{
	//	//cout << " Barrel 충돌 " << endl;
	//}
	//else
	//{
	//	//cout << " Barrel 충돌 아님" << endl;
	//}

	//cout << m_pSpider->m_xmOOBB.Extents.x << "\t" << m_pSpider->m_xmOOBB.Extents.y <<"\t"<< m_pSpider->m_xmOOBB.Extents.z << endl;
	//for (int i = 0; i < 2; ++i)
	//{

	//	if (m_pPlayer->m_xmOOBB.Intersects(m_pSpider[i]->m_xmOOBB))
	//	{
	//		//cout << i << "거미 충돌 " << endl;
	//		
	//		m_pSpider[i]->SetObjectAnimState(2);
	//	}
	//	else
	//	{
	//		//cout << i << "거미 충돌 아님" << endl;

	//		m_pSpider[i]->SetObjectAnimState(0);

	//	}
	//}


	//if (m_pPlayer->m_xmOOBB.Intersects(m_pDragon->m_xmOOBB))
	//{
	//	//cout << "드래곤 충돌 " << endl;
	//}
	//else
	//{
	//	//cout << "드래곤 충돌 아님" << endl;
	//}


}

HRESULT CTestScene::Load_Map()
{
	ifstream fin("../../Data/MapData.txt");
	if (!fin)
	{
		MSG_BOX(L"MapData 못읽음 망해따리 망해따~~~~");
		return E_FAIL;
	}
	string ignore;
	while (!fin.eof())
	{
		fin >> ignore;		// > MeshType

		Mesh::MESHTYPE eType = Mesh::MESHTYPE(std::stoi(ignore));

		fin >> ignore;		// > Com_Mesh_이름
		wstring wstrFileName;
		wstring meshName = wstrFileName.assign(ignore.begin(), ignore.end());
		CGameObject* pObject;
		int iSize = 1;
		fin >> iSize;
		CRenderer::RenderType eRenderType;
		switch (eType)
		{
		case Mesh::MESH_STATIC:
			pObject = CMapObject::Create(m_d3dDevice, mSrvDescriptorHeap[HEAP_DEFAULT], mCbvSrvDescriptorSize, const_cast<wchar_t*>(meshName.c_str()));
			eRenderType = CRenderer::RenderType::RENDER_NONALPHA_FORWARD;
			break;
		case Mesh::MESH_STATIC_INST:
			pObject = CInstancingObject::Create(m_d3dDevice, mSrvDescriptorHeap[HEAP_INSTANCING], mCbvSrvDescriptorSize, const_cast<wchar_t*>(meshName.c_str()), iSize);
			eRenderType = CRenderer::RenderType::RENDER_NONALPHA_INSTANCING;
			break;
		}


		pObject->SetCamera(Get_MainCam());
		Ready_GameObject(L"Layer_Map", pObject);
		CManagement::GetInstance()->GetRenderer()->Add_RenderGroup(eRenderType, pObject);

		std::string::size_type sz;

		string Value[9];
		float fValue[9];
		for (int i = 0; i < iSize; ++i)
		{
			for (int j = 0; j < 9; ++j)
			{
				fin >> Value[j];
				fValue[j] = std::stof(Value[j], &sz);
			}
			if (eType == (Mesh::MESH_STATIC_INST))
			{
				pObject->GetTransform(i)->Translation(fValue[0], fValue[1], fValue[2]);
				pObject->GetTransform(i)->Scaling(fValue[3], fValue[4], fValue[5]);
				pObject->GetTransform(i)->Rotation(fValue[6], fValue[7], fValue[8]);
			}
			else
			{
				pObject->GetTransform()->Translation(fValue[0], fValue[1], fValue[2]);
				pObject->GetTransform()->Scaling(fValue[3], fValue[4], fValue[5]);
				pObject->GetTransform()->Rotation(fValue[6], fValue[7], fValue[8]);
			}
		}
	}

	return S_OK;
}

CTestScene * CTestScene::Create(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice, vector<ComPtr<ID3D12DescriptorHeap>> &srv, UINT srvSize)
{
	CTestScene* pInstance = new CTestScene(d3dDevice, srv, srvSize);

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX(L"CTestScene Created Failed");
		Safe_Release(pInstance);
	}
	return pInstance;
}


void CTestScene::Put_Player(const float& x, const float& y, const float& z, const int& id)
{
	CGameObject* pObject = nullptr;

	if (MYPLAYERID == id)
	{
		pObject = Player::Create(m_d3dDevice, mSrvDescriptorHeap[HEAP_DEFAULT], mCbvSrvDescriptorSize);
		pObject->SetCamera(Get_MainCam());
		Ready_GameObject(L"Layer_Player", pObject);
		CManagement::GetInstance()->GetRenderer()->Add_RenderGroup(CRenderer::RENDER_NONALPHA_FORWARD, pObject);
		pObject->SetPosition(x, y, z);
		m_pMainCam->Set_Object(pObject);
	}
	else // 서버는 접속순서대로 클라에 ID를 부여함 // 클라이언트는 서버에서 보내주는대로 레이어 백터에 푸시백함 (맨 처음은 무조건 클라이언트 왜? 처음오는 패킷을 내 아이디로 쓰니까)
	{
		pObject = CNpc::Create(m_d3dDevice, mSrvDescriptorHeap[HEAP_DEFAULT], mCbvSrvDescriptorSize, L"Com_Mesh_Mage");
		Ready_GameObject(L"Layer_Player", pObject);
		CManagement::GetInstance()->GetRenderer()->Add_RenderGroup(CRenderer::RENDER_NONALPHA_FORWARD, pObject);
		pObject->SetPosition(x, y, z);
	}
}

void CTestScene::UISetting()
{
	XMFLOAT2 move = XMFLOAT2(-0.3f, 7.3f);

	move.x = -0.3f;
	move.y = 7.3f;

	XMFLOAT2 scale = XMFLOAT2(1.2f, 0.125f);
	scale.x = 1.2f;
	scale.y = 0.125f;
	float size = 0.25f;

	Texture* tex = CTexture_Manager::GetInstance()->Find_Texture("BloodTex", CTexture_Manager::TEX_DEFAULT_2D);

	CGameObject* pObject = HPBar::Create(m_d3dDevice, mSrvDescriptorHeap[HEAP_DEFAULT], mCbvSrvDescriptorSize, move, scale, size, tex->Num);
	pObject->SetCamera(Get_MainCam());
	Ready_GameObject(L"Layer_HPBar", pObject);
	CManagement::GetInstance()->GetRenderer()->Add_RenderGroup(CRenderer::RENDER_UI, pObject);

	move.x = -0.82f;
	move.y = 0.75f;

	scale.x = 1.0f;
	scale.y = 1.0f;

	size = 0.125f;

	tex = CTexture_Manager::GetInstance()->Find_Texture("WarriorUITex", CTexture_Manager::TEX_DEFAULT_2D);

	pObject = StaticUI::Create(m_d3dDevice, mSrvDescriptorHeap[HEAP_DEFAULT], mCbvSrvDescriptorSize, move, scale, size, tex->Num);//, "Models/StaticMesh/staticMesh.ASE", 10);
	pObject->SetCamera(Get_MainCam());
	//dynamic_cast<CInstancingObject*>(pObject)->SetCamFrustum(mCamFrustum);
	Ready_GameObject(L"Layer_PlayerStateUI", pObject);
	CManagement::GetInstance()->GetRenderer()->Add_RenderGroup(CRenderer::RENDER_UI, pObject);

	move.x = -0.5f;
	move.y = 1.45f;

	scale.x = 1.0f;
	scale.y = 0.5f;

	size = 0.5f;

	tex = CTexture_Manager::GetInstance()->Find_Texture("HeartTex", CTexture_Manager::TEX_DEFAULT_2D);

	pObject = StaticUI::Create(m_d3dDevice, mSrvDescriptorHeap[HEAP_DEFAULT], mCbvSrvDescriptorSize, move, scale, size, tex->Num);//, "Models/StaticMesh/staticMesh.ASE", 10);
	pObject->SetCamera(Get_MainCam());
	//dynamic_cast<CInstancingObject*>(pObject)->SetCamFrustum(mCamFrustum);
	Ready_GameObject(L"Layer_PlayerHPStateUI", pObject);
	CManagement::GetInstance()->GetRenderer()->Add_RenderGroup(CRenderer::RENDER_UI, pObject);
}

void CTestScene::Free()
{
	CScene::Free();
}
