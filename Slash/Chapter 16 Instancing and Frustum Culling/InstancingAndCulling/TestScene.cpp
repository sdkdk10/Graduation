#include "stdafx.h"
#include "TestScene.h"
#include "GameObject.h"
#include "Player.h"
#include "Define.h"
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
#include "Skeleton.h"
#include "Network.h"
#include "UI.h"
#include "HPBar.h"
#include "StaticUI.h"
#include "Mesh.h"
#include "MapObject.h"
#include "Texture_Manager.h"
#include "Transform.h"
#include "SkillEffect.h"
#include "Effect_Manager.h"
#include "Network.h"
#include "d3dApp.h"

CTestScene::CTestScene(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice, vector<ComPtr<ID3D12DescriptorHeap>> &srv, UINT srvSize)
	: CScene(d3dDevice, srv, srvSize)
{
}

CTestScene::~CTestScene()
{
}

HRESULT CTestScene::Initialize()
{
	CNetwork::GetInstance()->InitSock(D3DApp::GetApp()->MainWnd());
	//CManagement::GetInstance()->GetSound()->PlayBGM(L"Sound", L"village");

	CGameObject* pObject = SkyBox::Create(m_d3dDevice, mSrvDescriptorHeap[HEAP_DEFAULT], mCbvSrvDescriptorSize);
	Ready_GameObject(L"Layer_SkyBox", pObject);
	//CManagement::GetInstance()->GetRenderer()->Add_RenderGroup(CRenderer::RENDER_PRIORITY, pObject);

	pObject = Player::Create(m_d3dDevice, mSrvDescriptorHeap[HEAP_DEFAULT], mCbvSrvDescriptorSize);
	pObject->SetCamera(Get_MainCam());
	Ready_GameObject(L"Layer_Player", pObject);
	//CManagement::GetInstance()->GetRenderer()->Add_RenderGroup(CRenderer::RENDER_NONALPHA_FORWARD, pObject);

	for(int i = 0; i < MAX_USER; ++i)
	{
		pObject = CSkeleton::Create(m_d3dDevice, mSrvDescriptorHeap[HEAP_DEFAULT], mCbvSrvDescriptorSize, L"Com_Mesh_Mage");
		pObject->SetCamera(Get_MainCam());
		Ready_GameObject(L"Layer_Skeleton", pObject);
		//CManagement::GetInstance()->GetRenderer()->Add_RenderGroup(CRenderer::RENDER_NONALPHA_FORWARD, pObject);
	}
	
	for (int i = NPC_START; i < NUM_OF_NPC; ++i)
	{
		pObject = Spider::Create(m_d3dDevice, mSrvDescriptorHeap[HEAP_DEFAULT], mCbvSrvDescriptorSize);
		pObject->SetCamera(Get_MainCam());
		//pObject->SetPosition(i * 20 /*+ 100*/ -1000, 0, k * 20  + 200/*+ 100*/);
		pObject->SetPosition(0, 0, 5 + i);

		Ready_GameObject(L"Layer_Spider", pObject);
		//CManagement::GetInstance()->GetRenderer()->Add_RenderGroup(CRenderer::RENDER_NONALPHA_FORWARD, pObject);
	}


//	pObject = Spider::Create(m_d3dDevice, mSrvDescriptorHeap[HEAP_DEFAULT], mCbvSrvDescriptorSize);
//	pObject->SetCamera(Get_MainCam());
//	pObject->SetPosition(0,0,0);
////	pObject->SetOOBB(XMFLOAT3(pObject->GetBounds().Center.x + pObject->GetWorld()._41, pObject->GetBounds().Center.y + pObject->GetWorld()._42, pObject->GetBounds().Center.z +	pObject->GetWorld()._43), XMFLOAT3(pObject->GetBounds().Extents.x, pObject->GetBounds().Extents.y, pObject->GetBounds().Extents.z), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
//	
//	Ready_GameObject(L"Layer_Spider", pObject);
//	CManagement::GetInstance()->GetRenderer()->Add_RenderGroup(CRenderer::RENDER_NONALPHA_FORWARD, pObject);
//	
//	pObject = Spider::Create(m_d3dDevice, mSrvDescriptorHeap[HEAP_DEFAULT], mCbvSrvDescriptorSize);
//	pObject->SetCamera(Get_MainCam());
//	pObject->SetPosition(20,0,0);
////	pObject->SetOOBB(XMFLOAT3(pObject->GetBounds().Center.x + pObject->GetWorld()._41, pObject->GetBounds().Center.y + pObject->GetWorld()._42, pObject->GetBounds().Center.z +	pObject->GetWorld()._43), XMFLOAT3(pObject->GetBounds().Extents.x, pObject->GetBounds().Extents.y, pObject->GetBounds().Extents.z), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
//	
//	Ready_GameObject(L"Layer_Spider", pObject);
//	CManagement::GetInstance()->GetRenderer()->Add_RenderGroup(CRenderer::RENDER_NONALPHA_FORWARD, pObject);
//	
////	pObject = Spider::Create(m_d3dDevice, mSrvDescriptorHeap[HEAP_DEFAULT], mCbvSrvDescriptorSize);
////	pObject->SetCamera(Get_MainCam());
////	pObject->SetPosition(-15, 0, 0);
//////	pObject->SetOOBB(XMFLOAT3(pObject->GetBounds().Center.x + pObject->GetWorld()._41, pObject->GetBounds().Center.y + pObject->GetWorld()._42, pObject->GetBounds().Center.z + pObject->GetWorld()._43), XMFLOAT3(pObject->GetBounds().Extents.x, pObject->GetBounds().Extents.y, pObject->GetBounds().Extents.z), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
////
//
//	Ready_GameObject(L"Layer_Spider", pObject);
//	CManagement::GetInstance()->GetRenderer()->Add_RenderGroup(CRenderer::RENDER_NONALPHA_FORWARD, pObject);

	
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
				pObject->SetPosition(i * 20, j * 20, k * 20);

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


	//pObject = Dragon::Create(m_d3dDevice, mSrvDescriptorHeap[HEAP_DEFAULT], mCbvSrvDescriptorSize);
	//pObject->SetCamera(Get_MainCam());
	//Ready_GameObject(L"Layer_Dragon", pObject);
	//CManagement::GetInstance()->GetRenderer()->Add_RenderGroup(CRenderer::RENDER_NONALPHA_FORWARD, pObject);
	
	//pObject = Barrel::Create(m_d3dDevice, mSrvDescriptorHeap[HEAP_DEFAULT], mCbvSrvDescriptorSize);
	//pObject->SetCamera(Get_MainCam());
	//Ready_GameObject(L"Layer_Barrel", pObject);
	//CManagement::GetInstance()->GetRenderer()->Add_RenderGroup(CRenderer::RENDER_NONALPHA_FORWARD, pObject);

	pObject = Terrain::Create(m_d3dDevice, mSrvDescriptorHeap[HEAP_DEFAULT], mCbvSrvDescriptorSize);
	pObject->SetCamera(Get_MainCam());
	Ready_GameObject(L"Layer_Terrain", pObject);
	//CManagement::GetInstance()->GetRenderer()->Add_RenderGroup(CRenderer::RENDER_NONALPHA_FORWARD, pObject);

	//for (int i = 0; i < 30; ++i)
	//{
	//	
	//		for (int k = 0; k < 30; ++k)
	//		{
	//			pObject = Terrain::Create(m_d3dDevice, mSrvDescriptorHeap[HEAP_DEFAULT], mCbvSrvDescriptorSize);
	//			pObject->SetCamera(Get_MainCam());
	//			pObject->SetPosition(i * 50 -750, 0, k * 50 - 750);

	//			Ready_GameObject(L"Layer_Terrain", pObject);
	//		}
	//	

	//}

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
	auto * m_pPlayer = CManagement::GetInstance()->Find_Object(L"Layer_Player");

	auto spiderList = CManagement::GetInstance()->Get_Layer(L"Layer_Spider")->Get_ObjectList();
	m_pPlayer->m_pCollider = NULL;

	for (int i = 0; i < spiderList.size(); ++i)
	{
		spiderList[i]->m_pCollider = NULL;
	}
	CScene::Update(gt);

	CollisionProcess();
	UpdateOOBB();
	UpdateUI();

	m_pPlayer->m_pCollider;

	//CollisionProcess();
	if (m_pPlayer->m_pCollider == NULL)
	{
		m_pPlayer->m_MovingRefletVector = XMFLOAT3(0, 0, 0);
		//cout << "�浹����" << endl;
	}
	else
	{
		m_pPlayer->m_pCollider->SaveSlidingVector(m_pPlayer,m_pPlayer->m_pCollider);
		
	}


	for (int i = 0; i < spiderList.size(); ++i)
	{
		if (spiderList[i]->m_pCollider == NULL)
		{
			spiderList[i]->m_MovingRefletVector = XMFLOAT3(0, 0, 0);
		}
		else
		{
			spiderList[i]->m_pCollider->SaveSlidingVector(spiderList[i], spiderList[i]->m_pCollider);

		}
	}
	return true;
}

void CTestScene::UpdateOOBB()
{
	auto * m_pPlayer = CManagement::GetInstance()->Find_Object(L"Layer_Player");
	auto * m_pSpider = CManagement::GetInstance()->Find_Object(L"Layer_Spider");

	/*cout << " ------------------------------" << endl;
	cout << "�÷��̾�" << m_pPlayer->m_xmOOBB.Orientation.x << "\t" << m_pPlayer->m_xmOOBB.Orientation.y << "\t" << m_pPlayer->m_xmOOBB.Orientation.z << "\t" << m_pPlayer->m_xmOOBB.Orientation.w << endl;
	cout << "�Ź�" << m_pSpider->m_xmOOBB.Orientation.x << "\t" << m_pSpider->m_xmOOBB.Orientation.y << "\t" << m_pSpider->m_xmOOBB.Orientation.z << "\t" << m_pSpider->m_xmOOBB.Orientation.w << endl;
	cout << " ------------------------------" << endl;*/
	//cout << "�Ź�" << m_pSpider->m_xmOOBB.Orientation.x << "\t" << m_pSpider->m_xmOOBB.Orientation.y << "\t" << m_pSpider->m_xmOOBB.Orientation.z << "\t" << m_pSpider->m_xmOOBB.Orientation.w << endl;

	//float ceta = acos(m_pSpider->m_xmOOBB.Orientation.w) * 2.0f;


	//cout << "�Ź�" << "\t" << asin(m_pSpider->m_xmOOBB.Orientation.x) / sin(ceta/2) << "\t" << m_pSpider->m_xmOOBB.Orientation.y / sin(ceta / 2) << "\t" << m_pSpider->m_xmOOBB.Orientation.z / sin(ceta / 2) << "\t" << ceta * 57.3248f << endl;


	//Find_Object �ѹ��� �ϵ��� �ٲܰ�
	/*auto * m_pPlayer = CManagement::GetInstance()->Find_Object(L"Layer_Player");
	auto * m_pBarrel = CManagement::GetInstance()->Find_Object(L"Layer_Barrel");
	auto * m_pInstance = CManagement::GetInstance()->Find_Object(L"Layer_Map");
	auto * m_pSpider = CManagement::GetInstance()->Find_Object(L"Layer_Spider");
	auto * m_pSpider2 = CManagement::GetInstance()->Find_Object(L"Layer_Spider",1);*/
	//auto * m_pDragon = CManagement::GetInstance()->Find_Object(L"Layer_Dragon");

	//�÷��̾� ���� ����� �ּ�
	//m_pPlayer->m_xmOOBBTransformed.Transform(m_pPlayer->m_xmOOBB, XMLoadFloat4x4(&(m_pPlayer->GetWorld())));
	//XMStoreFloat4(&m_pPlayer->m_xmOOBBTransformed.Orientation, XMQuaternionNormalize(XMLoadFloat4(&m_pPlayer->m_xmOOBBTransformed.Orientation)));


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


	//cout << m_pPlayer->m_MovingRefletVector.x << "\t" << m_pPlayer->m_MovingRefletVector.y << "\t" << m_pPlayer->m_MovingRefletVector.z << endl;

}
void CTestScene::Render(ID3D12GraphicsCommandList * cmdList)
{
}

void CTestScene::CollisionProcess()
{
	

	//auto * m_pPlayer = CManagement::GetInstance()->Find_Object(L"Layer_Player");
	//auto * m_pBarrel = CManagement::GetInstance()->Find_Object(L"Layer_Barrel");

	//auto * m_pInstance = CManagement::GetInstance()->Find_Object(L"Layer_Map");
	////auto * m_pSpider = CManagement::GetInstance()->Find_Object(L"Layer_Spider");

	//vector<CGameObject *> m_pSpider;
	//for (int i = 0; i < 2; ++i)
	//{
	//	m_pSpider.push_back(CManagement::GetInstance()->Find_Object(L"Layer_Spider", i));
	//}

	////auto * m_pDragon = CManagement::GetInstance()->Find_Object(L"Layer_Dragon");


	//auto instanceData = dynamic_cast<CInstancingObject*>(m_pInstance)->GetvecInstances();


	//auto m_pCamera = CManagement::GetInstance()->Get_MainCam();


	//for (int i = 0; i < instanceData.size(); ++i)
	//{
	//	XMMATRIX world = XMLoadFloat4x4(&(instanceData[i].World));

	//	XMMATRIX invWorld = XMMatrixInverse(&XMMatrixDeterminant(world), world);

	//	// View space to the object's local space.

	//	BoundingOrientedBox mLocalPlayerBounds;	
	//	// Transform the camera frustum from view space to the object's local space.
	//	m_pPlayer->m_xmOOBB.Transform(mLocalPlayerBounds, invWorld);

	//	// Perform the box/frustum intersection test in local space.
	//	if (mLocalPlayerBounds.Intersects(m_pInstance->GetBounds()) != DirectX::DISJOINT)
	//	{

	//		//cout << i << "��° �ν��Ͻ� ������Ʈ�� �浹" << endl;
	//	}

	//}


	////if (m_pPlayer->m_xmOOBB.Intersects(m_pBarrel->m_xmOOBB))
	////{
	////	//cout << " Barrel �浹 " << endl;
	////}
	////else
	////{
	////	//cout << " Barrel �浹 �ƴ�" << endl;
	////}

	////cout << m_pSpider->m_xmOOBB.Extents.x << "\t" << m_pSpider->m_xmOOBB.Extents.y <<"\t"<< m_pSpider->m_xmOOBB.Extents.z << endl;
	////for (int i = 0; i < 2; ++i)
	////{

	////	if (m_pPlayer->m_xmOOBB.Intersects(m_pSpider[i]->m_xmOOBB))
	////	{
	////		//cout << i << "�Ź� �浹 " << endl;
	////		
	////		m_pSpider[i]->SetObjectAnimState(2);
	////	}
	////	else
	////	{
	////		//cout << i << "�Ź� �浹 �ƴ�" << endl;

	////		m_pSpider[i]->SetObjectAnimState(0);

	////	}
	////}
	//for (int i = 0; i < 2; ++i)
	//{

	//	if (m_pPlayer->m_xmOOBB.Intersects(m_pSpider[i]->m_xmOOBB))
	//	{
	//		//cout << i << "�Ź� �浹 " << endl;
	//		
	//		m_pSpider[i]->SetObjectAnimState(1);
	//	}
	//	else
	//	{
	//		//cout << i << "�Ź� �浹 �ƴ�" << endl;

	//		m_pSpider[i]->SetObjectAnimState(0);

	//	}
	//}


	////if (m_pPlayer->m_xmOOBB.Intersects(m_pDragon->m_xmOOBB))
	////{
	////	//cout << "�巡�� �浹 " << endl;
	////}
	////else
	////{
	////	//cout << "�巡�� �浹 �ƴ�" << endl;
	////}


}

HRESULT CTestScene::Load_Map()
{
	ifstream fin("Assets/Data/MapData.txt");
	if (!fin)
	{
		MSG_BOX(L"MapData ������ ���ص��� ���ص�~~~~");
		return E_FAIL;
	}
	string ignore;
	/*
	while (!fin.eof())
	{
	fin >> ignore;      // > MeshType

	Mesh::MESHTYPE eType = Mesh::MESHTYPE(std::stoi(ignore));

	fin >> ignore;      // > Com_Mesh_�̸�
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
	*/

	while (!fin.eof())
	{
		if (m_iObjectCount == 0)
			fin >> m_iObjectCount;

		fin >> ignore;      // > Com_Mesh_�̸�

		wstring wstrFileName;
		wstring meshName = wstrFileName.assign(ignore.begin(), ignore.end());

		CGameObject* pObject;
		CRenderer::RenderType eRenderType;

		int iSize = 1;
		fin >> iSize;         // > Instancing Size

							  //pObject = CMapObject::Create(m_d3dDevice, mSrvDescriptorHeap[HEAP_DEFAULT], mCbvSrvDescriptorSize, const_cast<wchar_t*>(meshName.c_str()));
							  //eRenderType = CRenderer::RenderType::RENDER_NONALPHA_FORWARD;
		pObject = CInstancingObject::Create(m_d3dDevice, mSrvDescriptorHeap[HEAP_INSTANCING], mCbvSrvDescriptorSize, const_cast<wchar_t*>(meshName.c_str()), iSize);
		eRenderType = CRenderer::RenderType::RENDER_NONALPHA_INSTANCING;
		//eRenderType = CRenderer::RenderType::RENDER_ALPHA_INST;




		pObject->SetCamera(Get_MainCam());

		std::string::size_type sz;

		string Value[9];
		float fValue[9];
		for (int i = 0; i < iSize; ++i)
		{
			int Alpha;
			fin >> Alpha;
			if (Alpha)
				eRenderType = CRenderer::RenderType::RENDER_ALPHA_INST;
			else
				eRenderType = CRenderer::RenderType::RENDER_NONALPHA_INSTANCING;
			fin >> ignore;         // > Tex_Name
			Texture* tex = CTexture_Manager::GetInstance()->Find_Texture(ignore, CTexture_Manager::TEX_INST_2D);      // > �ν��Ͻ��� �� �ٲ���
																													  //dynamic_cast<CInstancingObject*>(pObject)->SetTexture(i, tex->Num);

			for (int j = 0; j < 9; ++j)
			{
				fin >> Value[j];
				fValue[j] = std::stof(Value[j], &sz);
			}

			dynamic_cast<CInstancingObject*>(pObject)->GetTransform(i)->Translation(fValue[0], fValue[1], fValue[2]);
			dynamic_cast<CInstancingObject*>(pObject)->GetTransform(i)->Scaling(fValue[3], fValue[4], fValue[5]);
			dynamic_cast<CInstancingObject*>(pObject)->GetTransform(i)->Rotation(fValue[6], fValue[7], fValue[8]);
			dynamic_cast<CInstancingObject*>(pObject)->SetRenderType(eRenderType);
		}
		Ready_GameObject(L"Layer_Map", pObject);
		//CManagement::GetInstance()->GetRenderer()->Add_RenderGroup(eRenderType, pObject);
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
	//CManagement::GetInstance()->GetRenderer()->Add_RenderGroup(CRenderer::RENDER_UI, pObject);

	move.x = -0.82f;
	move.y = 0.75f;

	scale.x = 1.0f;
	scale.y = 1.0f;

	size = 0.125f;

	tex = CTexture_Manager::GetInstance()->Find_Texture("WarriorUITex", CTexture_Manager::TEX_DEFAULT_2D);

	//pObject = StaticUI::Create(m_d3dDevice, mSrvDescriptorHeap[HEAP_DEFAULT], mCbvSrvDescriptorSize, move, scale, size, tex->Num);//, "Models/StaticMesh/staticMesh.ASE", 10);
	pObject = StaticUI::Create(m_d3dDevice, mSrvDescriptorHeap[HEAP_DEFAULT], mCbvSrvDescriptorSize, L"Com_Mesh_WarriorUI", tex->Num);
	pObject->SetCamera(Get_MainCam());
	//dynamic_cast<CInstancingObject*>(pObject)->SetCamFrustum(mCamFrustum);
	Ready_GameObject(L"Layer_PlayerStateUI", pObject);
//	CManagement::GetInstance()->GetRenderer()->Add_RenderGroup(CRenderer::RENDER_UI, pObject);

	move.x = -0.5f;
	move.y = 1.45f;

	scale.x = 1.0f;
	scale.y = 0.5f;

	size = 0.5f;

	tex = CTexture_Manager::GetInstance()->Find_Texture("HeartTex", CTexture_Manager::TEX_DEFAULT_2D);

	//pObject = StaticUI::Create(m_d3dDevice, mSrvDescriptorHeap[HEAP_DEFAULT], mCbvSrvDescriptorSize, move, scale, size, tex->Num);//, "Models/StaticMesh/staticMesh.ASE", 10);
	pObject = StaticUI::Create(m_d3dDevice, mSrvDescriptorHeap[HEAP_DEFAULT], mCbvSrvDescriptorSize, L"Com_Mesh_PlayerHPState", tex->Num);
	pObject->SetCamera(Get_MainCam());
	//dynamic_cast<CInstancingObject*>(pObject)->SetCamFrustum(mCamFrustum);
	Ready_GameObject(L"Layer_PlayerHPStateUI", pObject);
//	CManagement::GetInstance()->GetRenderer()->Add_RenderGroup(CRenderer::RENDER_UI, pObject);
}

void CTestScene::Free()
{
	CScene::Free();
}
