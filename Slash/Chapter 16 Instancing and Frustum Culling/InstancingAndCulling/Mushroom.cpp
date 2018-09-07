#include "stdafx.h"
#include "Mushroom.h"
#include "Define.h"
#include "DynamicMeshSingle.h"
#include "Camera.h"
#include "Management.h"
#include "Renderer.h"
#include "Component_Manager.h"
#include "Texture_Manager.h"
#include "Player.h"
#include "Layer.h"


Mushroom::Mushroom(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice, ComPtr<ID3D12DescriptorHeap> &srv, UINT srvSize)
	: CGameObject(d3dDevice, srv, srvSize)
{
	m_fMoveSpeed = 2.0f;

	m_fRotateSpeed = 4.0f;
}


Mushroom::~Mushroom()
{
}

bool Mushroom::Update(const GameTimer & gt)
{



	if (!m_pPlayer)
		m_pPlayer = dynamic_cast<Player*>(CManagement::GetInstance()->Find_Object(L"Layer_Player"));

	CGameObject::Update(gt);




	m_pCamera = CManagement::GetInstance()->Get_MainCam();
	XMMATRIX view = m_pCamera->GetView();
	XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);



	//auto currObjectCB2 = m_pFrameResource->InstanceBuffer.get();

	XMMATRIX world = XMLoadFloat4x4(&World);
	XMMATRIX texTransform = XMLoadFloat4x4(&TexTransform);

	XMMATRIX invWorld = XMMatrixInverse(&XMMatrixDeterminant(world), world);

	// View space to the object's local space.
	XMMATRIX viewToLocal = XMMatrixMultiply(invView, invWorld);

	// Transform the camera frustum from view space to the object's local space.
	BoundingFrustum localSpaceFrustum;
	mCamFrustum = *CManagement::GetInstance()->Get_CurScene()->Get_CamFrustum();
	mCamFrustum.Transform(localSpaceFrustum, viewToLocal);


	// Perform the box/frustum intersection test in local space.
	if ((localSpaceFrustum.Contains(Bounds) != DirectX::DISJOINT) || (mFrustumCullingEnabled == false))
	{
		//cout << "보인당!" << endl;
		m_bIsVisiable = true;
		Animate(gt);

		auto currObjectCB = m_pFrameResource->ObjectCB.get();

		ObjectConstants objConstants;
		XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(world));
		XMStoreFloat4x4(&objConstants.TexTransform, XMMatrixTranspose(texTransform));
		objConstants.MaterialIndex = Mat->MatCBIndex;

		currObjectCB->CopyData(ObjCBIndex, objConstants);

		auto currMaterialCB = m_pFrameResource->MaterialCB.get();

		XMMATRIX matTransform = XMLoadFloat4x4(&Mat->MatTransform);

		MaterialConstants matConstants;
		matConstants.DiffuseAlbedo = Mat->DiffuseAlbedo;
		matConstants.FresnelR0 = Mat->FresnelR0;
		matConstants.Roughness = Mat->Roughness;
		XMStoreFloat4x4(&matConstants.MatTransform, XMMatrixTranspose(matTransform));

		matConstants.DiffuseMapIndex = Mat->DiffuseSrvHeapIndex;

		currMaterialCB->CopyData(Mat->MatCBIndex, matConstants);
	}
	else
	{
		//cout << "안보인당!" << endl;
		m_bIsVisiable = false;
	}

	CManagement::GetInstance()->GetRenderer()->Add_RenderGroup(CRenderer::RENDER_NONALPHA_FORWARD, this);
	return true;
}

void Mushroom::Render(ID3D12GraphicsCommandList * cmdList)
{
	if (m_bIsVisiable /*&& m_bIsConnected*/)
	{
		AnimStateMachine->SetTimerTrueFalse();

		UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
		UINT matCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(MaterialConstants));

		auto objectCB = m_pFrameResource->ObjectCB->Resource();
		auto matCB = m_pFrameResource->MaterialCB->Resource();

		cmdList->IASetVertexBuffers(0, 1, &Geo->VertexBufferView());
		cmdList->IASetIndexBuffer(&Geo->IndexBufferView());
		cmdList->IASetPrimitiveTopology(PrimitiveType);

		CD3DX12_GPU_DESCRIPTOR_HANDLE tex(mSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
		tex.Offset(Mat->DiffuseSrvHeapIndex, mCbvSrvDescriptorSize);

		Mat->DiffuseSrvHeapIndex;
		D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + ObjCBIndex * objCBByteSize;
		D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress() + Mat->MatCBIndex*matCBByteSize;

		cmdList->SetGraphicsRootConstantBufferView(4, objCBAddress);
		//cmdList->SetGraphicsRootConstantBufferView(5, matCBAddress);
		cmdList->SetGraphicsRootShaderResourceView(5, matCBAddress);

		cmdList->SetGraphicsRootDescriptorTable(7, tex);

		//	auto indexcnt = dynamic_cast<DynamicMesh*>(m_pMesh)->Indexoffset[0];

		auto pMesh = dynamic_cast<DynamicMeshSingle*>(m_pMesh);


		//int iTest = (int)pMesh->m_fTest;

		int iTest = AnimStateMachine->GetCurAnimFrame();
		int AnimaState = AnimStateMachine->GetAnimState();

		cmdList->DrawIndexedInstanced(pMesh->Indexoffset[1], 1,
			pMesh->Indexoffset[iTest] + pMesh->IndexAnimoffset[AnimaState] /*+ pMesh->IndexAnimoffset[0]*/,
			pMesh->Vertexoffset[iTest] + pMesh->VertexAnimoffset[AnimaState]/*+ pMesh->VertexAnimoffset[0]*/, 0);


		//if (m_bLODState == true)
		//{
		//	cmdList->DrawIndexedInstanced(1200, 1,
		//		0 /*+ pMesh->IndexAnimoffset[0]*/,
		//		0/*+ pMesh->VertexAnimoffset[0]*/, 0);
		//}
		//else
		//{
		//	cmdList->DrawIndexedInstanced(pMesh->Indexoffset[1], 1,
		//		pMesh->Indexoffset[iTest] + pMesh->IndexAnimoffset[AnimaState] /*+ pMesh->IndexAnimoffset[0]*/,
		//		pMesh->Vertexoffset[iTest] + pMesh->VertexAnimoffset[AnimaState]/*+ pMesh->VertexAnimoffset[0]*/, 0);

		//}



	}


}

HRESULT Mushroom::Initialize()
{
	m_pMesh = dynamic_cast<DynamicMeshSingle*>(CComponent_Manager::GetInstance()->Clone_Component(L"Com_Mesh_Mushroom"));
	if (nullptr == m_pMesh)
		return E_FAIL;



	Texture* tex = CTexture_Manager::GetInstance()->Find_Texture("MushroomTex", CTexture_Manager::TEX_DEFAULT_2D);
	if (nullptr == tex)
		return E_FAIL;

	m_strTexName[SPIDER_BRICK] = "bricksTex";
	m_strTexName[SPIDER_STONE] = "stoneTex";
	m_strTexName[SPIDER_TILE] = "tileTex";
	m_strTexName[SPIDER_ICE] = "iceTex";


	AnimStateMachine = new AnimateStateMachine;

	AnimStateMachine->vecAnimFrame = &(dynamic_cast<DynamicMeshSingle*>(m_pMesh)->vecAnimFrame);

	AnimStateMachine->SetAnimState(State::STATE_IDLE);

	Mat = new Material;
	Mat->Name = "SpiderMat";
	Mat->MatCBIndex = m_iMyObjectID;
	Mat->DiffuseSrvHeapIndex = tex->Num;
	Mat->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	Mat->FresnelR0 = XMFLOAT3(0.05f, 0.05f, 0.05f);
	Mat->Roughness = 0.3f;

	XMStoreFloat4x4(&World, XMMatrixScaling(m_fScale, m_fScale, m_fScale)*XMMatrixRotationX(1.7f)*XMMatrixRotationZ(3.14f)*XMMatrixTranslation(0.0f, 0.0f, 0.0f));


	//XMStoreFloat4x4(&World, XMMatrixScaling(m_fScale, m_fScale, m_fScale)*XMMatrixTranslation(0.0f, 0.0f, 0.0f)*XMMatrixTranslation(0.0f, 0.0f, 0.0f));
	TexTransform = MathHelper::Identity4x4();
	ObjCBIndex = m_iMyObjectID;

	Geo = dynamic_cast<DynamicMeshSingle*>(m_pMesh)->m_Geometry[0].get();
	PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	IndexCount = Geo->DrawArgs["SingleMesh"].IndexCount;
	StartIndexLocation = Geo->DrawArgs["SingleMesh"].StartIndexLocation;
	BaseVertexLocation = Geo->DrawArgs["SingleMesh"].BaseVertexLocation;
	Bounds = Geo->DrawArgs["SingleMesh"].Bounds;



	m_xmf3Scale = XMFLOAT3(2.0f, 2.0f, 2.0f);
	m_xmf3Rot = XMFLOAT3(0.0f, 0.0f, 0.0f);

	return S_OK;
}

void Mushroom::Animate(const GameTimer & gt)
{
	AnimStateMachine->AnimationStateUpdate(gt);

}

void Mushroom::SaveSlidingVector(CGameObject * pobj, CGameObject * pCollobj)
{


	//XMFLOAT3 pCollObjPos2= pCollobj->m_pTransCom->m_f3Position;
	XMFLOAT3 Center = pCollobj->GetPosition();
	XMFLOAT3 Player = pobj->GetPosition();
	XMFLOAT3 dirVector = Vector3::Subtract(Player, Center);   // 충돌 객체에서 플레이어로 가는 벡터

	float tanceta = dirVector.z / dirVector.x;
	float ceta = atan(tanceta) * 57.3248f;

	float extenttanceta = pCollobj->m_xmOOBB.Extents.z / pCollobj->m_xmOOBB.Extents.x;


	float extentceta = atan(extenttanceta) * 57.3248f;


	dirVector = Vector3::Normalize(dirVector);
	//cout << "Coll obj Look" << "\t" << look.x << "\t" << look.y << "\t" << look.z << endl;
	//cout << "dirVector : " << "\t" << dirVector.x << "\t" << dirVector.y << "\t" << dirVector.z << endl;
	//cout << "Ceta : " << ceta << endl;
	//cout << "ExtentCeta : " << extentceta << endl;

	if (Player.x > Center.x && 0 < ceta && ceta< extentceta) // 1
	{
		//cout << " 오른쪽 충돌" << endl;
		pobj->m_MovingRefletVector = XMFLOAT3(1, 0, 0);


	}
	if (Player.z > Center.z && extentceta <ceta && ceta < 90) // 2
	{
		//cout << "윗쪽 충돌" << endl;
		pobj->m_MovingRefletVector = XMFLOAT3(0, 0, 1);
	}
	if (Player.z > Center.z && -90 < ceta && ceta < -extentceta) // 3
	{
		//cout << "윗쪽 충돌" << endl;
		//pobj->m_MovingRefletVector = XMFLOAT3(-1, 0, 0);
		pobj->m_MovingRefletVector = XMFLOAT3(0, 0, 1);

	}
	if (Player.x < Center.x && -extentceta < ceta && ceta < 0)// 4
	{
		//cout << "왼쪽 충돌" << endl;
		//pobj->m_MovingRefletVector = XMFLOAT3(-1, 0, 0);
		pobj->m_MovingRefletVector = XMFLOAT3(-1, 0, 0);

	}
	if (Player.x < Center.x && 0 < ceta && ceta < extentceta) // 5
	{
		//cout << "왼쪽 충돌" << endl;
		//pobj->m_MovingRefletVector = XMFLOAT3(-1, 0, 0);
		pobj->m_MovingRefletVector = XMFLOAT3(-1, 0, 0);

	}
	if (Player.z < Center.z && extentceta < ceta && ceta < 90) // 6
	{
		//cout << "아래쪽 충돌" << endl;
		//pobj->m_MovingRefletVector = XMFLOAT3(-1, 0, 0);
		pobj->m_MovingRefletVector = XMFLOAT3(0, 0, -1);

	}
	if (Player.z < Center.z && -90 < ceta && ceta < -extentceta) // 7
	{
		//cout << "아래쪽 충돌" << endl;
		//pobj->m_MovingRefletVector = XMFLOAT3(-1, 0, 0);
		pobj->m_MovingRefletVector = XMFLOAT3(0, 0, -1);

	}
	if (Player.x > Center.x && -extentceta < ceta && ceta < 0) // 8
	{
		//cout << "오른쪽 충돌" << endl;
		//pobj->m_MovingRefletVector = XMFLOAT3(-1, 0, 0);
		pobj->m_MovingRefletVector = XMFLOAT3(1, 0, 0);

	}
}

//void NagaGuard::SetTexture(SpiderTex _tex)
//{
//	Texture* tex = CTexture_Manager::GetInstance()->Find_Texture(m_strTexName[_tex], CTexture_Manager::TEX_DEFAULT_2D);
//	if (nullptr == tex)
//		return;
//	Mat->DiffuseSrvHeapIndex = tex->Num;
//
//}

Mushroom * Mushroom::Create(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice, ComPtr<ID3D12DescriptorHeap>& srv, UINT srvSize)
{
	Mushroom* pInstance = new Mushroom(d3dDevice, srv, srvSize);

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX(L"Player Created Failed");
		Safe_Release(pInstance);
	}

	return pInstance;
}
