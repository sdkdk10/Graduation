#include "stdafx.h"
#include "Spider.h"
#include "Define.h"
#include "DynamicMeshSingle.h"
#include "Camera.h"
#include "Management.h"
#include "Component_Manager.h"
#include "Texture_Manager.h"
#include "Player.h"

Spider::Spider(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice, ComPtr<ID3D12DescriptorHeap> &srv, UINT srvSize)
	: CGameObject(d3dDevice, srv, srvSize)
{

}


Spider::~Spider()
{
}

bool Spider::Update(const GameTimer & gt)
{

	if (!m_pPlayer)
		m_pPlayer = dynamic_cast<Player*>(CManagement::GetInstance()->Find_Object(L"Layer_Player"));

	CGameObject::Update(gt);

	Animate(gt);

	
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
	

	/*cout << "World : " << World._41 << "\t" << World._42 << "\t" << World._43 << endl;
	cout << "Bounds Center: " << Bounds.Center.x << "\t" << Bounds.Center.y << "\t" << Bounds.Center.z << endl;*/
 

	//////////////////////////////////////////////////


	// Next FrameResource need to be updated too.
	//NumFramesDirty--;
	
	return true;
}

void Spider::Render(ID3D12GraphicsCommandList * cmdList)
{
	if (m_bIsVisiable)
	{
		AnimStateMachine.SetTimerTrueFalse();

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

		int iTest = AnimStateMachine.GetCurAnimFrame();
		int AnimaState = AnimStateMachine.GetAnimState();

		cmdList->DrawIndexedInstanced(pMesh->Indexoffset[1], 1,
			pMesh->Indexoffset[iTest] + pMesh->IndexAnimoffset[AnimaState] /*+ pMesh->IndexAnimoffset[0]*/,
			pMesh->Vertexoffset[iTest] + pMesh->VertexAnimoffset[AnimaState]/*+ pMesh->VertexAnimoffset[0]*/, 0);

	
	}
	

}

HRESULT Spider::Initialize()
{
	m_pMesh = dynamic_cast<DynamicMeshSingle*>(CComponent_Manager::GetInstance()->Clone_Component(L"Com_Mesh_Spider"));
	if (nullptr == m_pMesh)
		return E_FAIL;



	Texture* tex = CTexture_Manager::GetInstance()->Find_Texture("SpiderTex", CTexture_Manager::TEX_DEFAULT_2D);
	if (nullptr == tex)
		return E_FAIL;

	AnimStateMachine.vecAnimFrame = &(dynamic_cast<DynamicMeshSingle*>(m_pMesh)->vecAnimFrame);

	AnimStateMachine.SetAnimState(AnimStateMachine.IdleState);

	Mat = new Material;
	Mat->Name = "SpiderMat";
	Mat->MatCBIndex = 3;
	Mat->DiffuseSrvHeapIndex = tex->Num;
	Mat->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	Mat->FresnelR0 = XMFLOAT3(0.05f, 0.05f, 0.05f);
	Mat->Roughness = 0.3f;

	XMStoreFloat4x4(&World, XMMatrixScaling(m_fScale, m_fScale, m_fScale)*XMMatrixTranslation(0.0f, 0.0f, 0.0f)*XMMatrixTranslation(0.0f, 0.0f, 0.0f));
	TexTransform = MathHelper::Identity4x4();
	ObjCBIndex = m_iMyObjectID;

	Geo = dynamic_cast<DynamicMeshSingle*>(m_pMesh)->m_Geometry[0].get();
	PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	IndexCount = Geo->DrawArgs["SingleMesh"].IndexCount;
	StartIndexLocation = Geo->DrawArgs["SingleMesh"].StartIndexLocation;
	BaseVertexLocation = Geo->DrawArgs["SingleMesh"].BaseVertexLocation;
	Bounds = Geo->DrawArgs["SingleMesh"].Bounds;

	

	//SetOOBB(XMFLOAT3(Bounds.Center.x *m_fScale, Bounds.Center.y * m_fScale, Bounds.Center.z *m_fScale), XMFLOAT3(Bounds.Extents.x * m_fScale, Bounds.Extents.y * m_fScale, Bounds.Extents.z * m_fScale), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));

	SetOOBB(XMFLOAT3(Bounds.Center.x , Bounds.Center.y , Bounds.Center.z ), XMFLOAT3(Bounds.Extents.x, Bounds.Extents.y, Bounds.Extents.z), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));



	return S_OK;
}

void Spider::Animate(const GameTimer & gt)
{
	XMFLOAT3 playerPos = m_pPlayer->GetPosition();
	XMFLOAT3 Shaft = XMFLOAT3(1, 0, 0);

	XMFLOAT3 dirVector = Vector3::Subtract(playerPos, GetPosition());   // 객체에서 플레이어로 가는 벡터

	dirVector = Vector3::Normalize(dirVector);
	
	float dotproduct = Vector3::DotProduct(Shaft, dirVector);
	float ShafttLength = Vector3::Length(Shaft);
	float dirVectorLength = Vector3::Length(dirVector);

	float cosCeta = (dotproduct / ShafttLength * dirVectorLength);

	float ceta = acos(cosCeta);

	if (playerPos.z < GetPosition().z)
		ceta = 360.f - ceta *51.2958f;// +180.0f;
	else
		ceta = ceta * 51.2958f;

	//cout << ceta << endl;
	float a = Vector3::Length(dirVector);
	XMFLOAT3 Normal = XMFLOAT3(a * cos(ceta), 0, a * sin(ceta));
	Normal = Vector3::Normalize(Normal);

	//cout << Normal.x << "\t" << Normal.y << "\t" << Normal.z << endl;
	/*m_pxmf4WallPlanes[0] = XMFLOAT4(-1.0f, 0.0f, 0.0f, GetPosition().x + Bounds.Extents.x);
	m_pxmf4WallPlanes[1] = XMFLOAT4(1.0f, 0.0f, 0.0f, GetPosition().x + Bounds.Extents.x);
	m_pxmf4WallPlanes[2] = XMFLOAT4(0.0f, 0.0f, -1.0f, GetPosition().z + Bounds.Extents.z);
	m_pxmf4WallPlanes[3] = XMFLOAT4(0.0f, 0.0f, 1.0f, GetPosition().z + Bounds.Extents.z);

	int nPlaneIndex = -1;
	for (int j = 0; j < 4; j++)
	{
		PlaneIntersectionType intersectType = m_pPlayer->m_xmOOBB.Intersects(XMLoadFloat4(&m_pxmf4WallPlanes[j]));
		if (intersectType == INTERSECTING)
		{
			nPlaneIndex = j;
			break;
		}
	}
	if (nPlaneIndex != -1)
	{
		
	}
	cout << nPlaneIndex << endl;*/

	if (Vector3::BetweenVectorLength(m_pPlayer->GetPosition(), GetPosition()) < 5.0f)
	{
		//cout << " 커몬 " << endl;
	}
	//cout << m_pPlayer->GetPosition().x << "\t" << m_pPlayer->GetPosition().y << "\t" << m_pPlayer->GetPosition().z << endl;
	AnimStateMachine.AnimationStateUpdate(gt);

	
	if (m_pPlayer->m_xmOOBB.Intersects(m_xmOOBB))
	{
		//cout << i << "거미 충돌 " << endl;
		
	/*	if (0.0f < ceta && ceta < 45.0f)
		{
			cout << "오른쪽과 충돌 " << endl;
		}
		if (45.0f < ceta && ceta < 135.0f)
		{
			cout << "윗쪽과 충돌 " << endl;
		}
		if (135.0f < ceta && ceta < 225.0f)
		{
			cout << "왼쪽과 충돌 " << endl;
		}
		if (225.0f < ceta && ceta < 360.0f)
		{
			cout << "아랫쪽과 충돌 " << endl;
		}*/
		SetObjectAnimState(2);
	}
	else
	{
		//cout << i << "거미 충돌 아님" << endl;

		SetObjectAnimState(0);

	}

	m_xmOOBBTransformed.Transform(m_xmOOBB, XMLoadFloat4x4(&(GetWorld())));
	XMStoreFloat4(&m_xmOOBBTransformed.Orientation, XMQuaternionNormalize(XMLoadFloat4(&m_xmOOBBTransformed.Orientation)));


}

Spider * Spider::Create(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice, ComPtr<ID3D12DescriptorHeap>& srv, UINT srvSize)
{
	Spider* pInstance = new Spider(d3dDevice, srv, srvSize);

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX(L"Player Created Failed");
		Safe_Release(pInstance);
	}

	return pInstance;
}
