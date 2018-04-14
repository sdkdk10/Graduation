#include "stdafx.h"
#include "TestScene.h"
#include "Define.h"
#include "GameObject.h"
#include "Insec.h"
#include "Barrel.h"
#include "SkyBox.h"
#include "InstancingObject.h"
#include "Layer.h"
#include "Management.h"
#include "Renderer.h"
#include "Terrain.h"
#include "Collision_Manager.h"



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

	pObject = Insec::Create(m_d3dDevice, mSrvDescriptorHeap[HEAP_DEFAULT], mCbvSrvDescriptorSize);
	Ready_GameObject(L"Layer_Player", pObject);
	CManagement::GetInstance()->GetRenderer()->Add_RenderGroup(CRenderer::RENDER_NONALPHA_FORWARD, pObject);
	
	pObject = Barrel::Create(m_d3dDevice, mSrvDescriptorHeap[HEAP_DEFAULT], mCbvSrvDescriptorSize);
	Ready_GameObject(L"Layer_Barrel", pObject);
	CManagement::GetInstance()->GetRenderer()->Add_RenderGroup(CRenderer::RENDER_NONALPHA_FORWARD, pObject);

	pObject = Terrain::Create(m_d3dDevice, mSrvDescriptorHeap[HEAP_DEFAULT], mCbvSrvDescriptorSize);
	Ready_GameObject(L"Layer_Terrain", pObject);
	CManagement::GetInstance()->GetRenderer()->Add_RenderGroup(CRenderer::RENDER_NONALPHA_FORWARD, pObject);

	pObject = CInstancingObject::Create(m_d3dDevice, mSrvDescriptorHeap[HEAP_INSTANCING], mCbvSrvDescriptorSize);//, "Models/StaticMesh/staticMesh.ASE", 10);

	dynamic_cast<CInstancingObject*>(pObject)->SetCamera(Get_MainCam());
	//dynamic_cast<CInstancingObject*>(pObject)->SetCamFrustum(mCamFrustum);
	Ready_GameObject(L"Layer_Instance", pObject);
	CManagement::GetInstance()->GetRenderer()->Add_RenderGroup(CRenderer::RENDER_NONALPHA_INSTANCING, pObject);

	return S_OK;
}

bool CTestScene::Update(const GameTimer & gt)
{
	CScene::Update(gt);

	auto * m_pPlayer = CManagement::GetInstance()->Find_Object(L"Layer_Player");
	auto * m_pBarrel = CManagement::GetInstance()->Find_Object(L"Layer_Barrel");
	auto * m_pInstance = CManagement::GetInstance()->Find_Object(L"Layer_Instance");

	auto instanceData = dynamic_cast<CInstancingObject*>(m_pInstance)->GetvecInstances();
	

	auto m_pCamera = CManagement::GetInstance()->Get_MainCam();

	for (int i = 0; i < instanceData.size(); ++i)
	{
		XMMATRIX world = XMLoadFloat4x4(&instanceData[i].World);

		XMMATRIX invWorld = XMMatrixInverse(&XMMatrixDeterminant(world), world);

		// View space to the object's local space.

		BoundingOrientedBox mLocalPlayerBounds ;
		// Transform the camera frustum from view space to the object's local space.
		m_pPlayer->m_xmOOBB.Transform(mLocalPlayerBounds, invWorld);

		// Perform the box/frustum intersection test in local space.
		if (mLocalPlayerBounds.Contains(m_pInstance->GetBounds()) != DirectX::DISJOINT)
		{
			cout << "인스턴싱 오브젝트랑 충돌" << endl;
		}
		
	}
	//cout << m_pPlayer->GetPosition().x << "\t" << m_pPlayer->GetPosition().y << m_pPlayer->GetPosition().z << endl;

	m_pPlayer->m_xmOOBBTransformed.Transform(m_pPlayer->m_xmOOBB, XMLoadFloat4x4(&XMFLOAT4X4(m_pPlayer->GetRight().x, m_pPlayer->GetRight().y, m_pPlayer->GetRight().z, 0, m_pPlayer->GetUp().x, m_pPlayer->GetUp().y, m_pPlayer->GetUp().z, 0, m_pPlayer->GetLook().x, m_pPlayer->GetLook().y, m_pPlayer->GetLook().z, 0, m_pPlayer->GetPosition().x, m_pPlayer->GetPosition().y, m_pPlayer->GetPosition().z, 1)));
	
	XMStoreFloat4(&m_pPlayer->m_xmOOBBTransformed.Orientation, XMQuaternionNormalize(XMLoadFloat4(&m_pPlayer->m_xmOOBBTransformed.Orientation)));
	
	/*cout << m_pPlayer->m_xmOOBB.Center.x << "\t" << m_pPlayer->m_xmOOBB.Center.y << "\t" << m_pPlayer->m_xmOOBB.Center.z << endl;
	cout << m_pPlayer->m_xmOOBB.Extents.x << "\t" << m_pPlayer->m_xmOOBB.Extents.y << "\t" << m_pPlayer->m_xmOOBB.Extents.z << endl;*/

	//cout << "---------------------------------------------------------" << endl;
	//cout << " Barrel Center : " << m_pBarrel->m_xmOOBB.Center.x << "\t" << m_pBarrel->m_xmOOBB.Center.y << "\t" << m_pBarrel->m_xmOOBB.Center.z << endl;
	//cout << " Barrel Extent : " << m_pBarrel->m_xmOOBB.Extents.x << "\t" << m_pBarrel->m_xmOOBB.Extents.y << "\t" << m_pBarrel->m_xmOOBB.Extents.z << endl;
	//cout << " Barrel Pos : " << m_pBarrel->GetPosition().x << "\t" << m_pBarrel->GetPosition().y << "\t" << m_pBarrel->GetPosition().z << endl;
	//cout << "---------------------------------------------------------" << endl;

	//cout << "---------------------------------------------------------" << endl;
	//cout << " Player Center : " << m_pPlayer->m_xmOOBB.Center.x << "\t" << m_pPlayer->m_xmOOBB.Center.y << "\t" << m_pPlayer->m_xmOOBB.Center.z << endl;
	//cout << " Player Extent : " << m_pPlayer->m_xmOOBB.Extents.x << "\t" << m_pPlayer->m_xmOOBB.Extents.y << "\t" << m_pPlayer->m_xmOOBB.Extents.z << endl;
	//cout << " Player Pos : " << m_pPlayer->GetPosition().x << "\t" << m_pPlayer->GetPosition().y << "\t" << m_pPlayer->GetPosition().z << endl;

	//cout << "---------------------------------------------------------" << endl;

	//cout << m_pPlayer->GetBounds().Center.x << "\t" << m_pPlayer->GetBounds().Center.y << "\t" << m_pPlayer->GetBounds().Center.z << endl;
	//cout << m_pPlayer->GetBounds().Extents.x << "\t" << m_pPlayer->GetBounds().Extents.y << "\t" << m_pPlayer->GetBounds().Extents.z << endl;


	//cout << "---------------------------------------------------------" << endl;
	//cout << m_pBarrel->GetBounds().Center.x << "\t" << m_pBarrel->GetBounds().Center.y << "\t" << m_pBarrel->GetBounds().Center.z << endl;
	//cout << m_pBarrel->GetBounds().Extents.x << "\t" << m_pBarrel->GetBounds().Extents.y << "\t" << m_pBarrel->GetBounds().Extents.z << endl;
	//cout << "---------------------------------------------------------" << endl;

	if (m_pPlayer->m_xmOOBB.Contains(m_pBarrel->m_xmOOBB))
	{
		cout << "충돌 " << endl;
	}
	else
	{
		cout << "충돌 아님" << endl;
	}
	//Collision_Manager::CollisionDetect();
	return true;
}

void CTestScene::Render(ID3D12GraphicsCommandList * cmdList)
{
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

void CTestScene::Free()
{
	CScene::Free();
}
