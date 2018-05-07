#include "stdafx.h"
#include "Spider.h"
#include "Define.h"
#include "DynamicMeshSingle.h"
#include "Camera.h"
#include "Management.h"
#include "Component_Manager.h"
#include "Texture_Manager.h"

Spider::Spider(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice, ComPtr<ID3D12DescriptorHeap> &srv, UINT srvSize)
	: CGameObject(d3dDevice, srv, srvSize)
{
}


Spider::~Spider()
{
}

bool Spider::Update(const GameTimer & gt)
{
	CGameObject::Update(gt);
<<<<<<< HEAD

	Animate(gt);

=======
>>>>>>> 4dac4ec56cadf5ef87d42169e83ea1d1d355dfc3
	m_pCamera = CManagement::GetInstance()->Get_MainCam();
	XMMATRIX view = m_pCamera->GetView();
	XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);
	auto currObjectCB = m_pFrameResource->ObjectCB.get();


	
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

		int iTest = AnimStateMachine.m_iCurAnimFrame;
		int AnimaState = AnimStateMachine.m_iAnimState;
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

	AnimStateMachine.m_iAnimState = AnimStateMachine.IdleState;
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
	AnimStateMachine.AnimationStateUpdate(gt);
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
