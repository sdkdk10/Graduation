#include "stdafx.h"
#include "Dragon.h"
#include "Define.h"
#include "DynamicMeshSingle.h"
#include "Camera.h"
#include "Management.h"


Dragon::Dragon(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice, ComPtr<ID3D12DescriptorHeap> &srv, UINT srvSize)
	: CGameObject(d3dDevice, srv, srvSize)
{
}


Dragon::~Dragon()
{
}

bool Dragon::Update(const GameTimer & gt)
{
	CGameObject::Update(gt);
	m_pMesh->Update(gt);
	m_pCamera = CManagement::GetInstance()->Get_MainCam();
	XMMATRIX view = m_pCamera->GetView();
	XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);
	auto currObjectCB = m_pFrameResource->ObjectCB.get();


	World._43 = 50;
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
	if ((localSpaceFrustum.Contains(Bounds /*m_xmOOBB*/)  != DirectX::DISJOINT) || (mFrustumCullingEnabled == false))
	{
		//cout << "보인당!" << endl;
		m_bIsVisiable = true;
		ObjectConstants objConstants;
		XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(world));
		XMStoreFloat4x4(&objConstants.TexTransform, XMMatrixTranspose(texTransform));
		objConstants.MaterialIndex = Mat->MatCBIndex;

		currObjectCB->CopyData(ObjCBIndex, objConstants);
	}
	else
	{
		//cout << "안보인당!" << endl;
		m_bIsVisiable = false;
	}




	//////////////////////////////////////////////////


	// Next FrameResource need to be updated too.
	//NumFramesDirty--;
	auto currMaterialCB = m_pFrameResource->MaterialCB.get();

	XMMATRIX matTransform = XMLoadFloat4x4(&Mat->MatTransform);

	MaterialConstants matConstants;
	matConstants.DiffuseAlbedo = Mat->DiffuseAlbedo;
	matConstants.FresnelR0 = Mat->FresnelR0;
	matConstants.Roughness = Mat->Roughness;
	XMStoreFloat4x4(&matConstants.MatTransform, XMMatrixTranspose(matTransform));

	matConstants.DiffuseMapIndex = Mat->DiffuseSrvHeapIndex;

	currMaterialCB->CopyData(Mat->MatCBIndex, matConstants);
	return true;
}

void Dragon::Render(ID3D12GraphicsCommandList * cmdList)
{
	if (m_bIsVisiable)
	{
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


		int iTest = pMesh->m_fTest;
		cmdList->DrawIndexedInstanced(pMesh->Indexoffset[1], 1,
			pMesh->Indexoffset[iTest] + pMesh->IndexAnimoffset[0] /*+ pMesh->IndexAnimoffset[0]*/,
			pMesh->Vertexoffset[iTest] + pMesh->VertexAnimoffset[0]/*+ pMesh->VertexAnimoffset[0]*/, 0);


	}

}

HRESULT Dragon::Initialize()
{
	m_pMesh = new DynamicMeshSingle(m_d3dDevice);



	vector<pair<const string, const string>> path;
	path.push_back(make_pair("Idle", "Models/Dragon/Dragon_FlyIdle.ASE"));
	//path.push_back(make_pair("Idle", "Models/Spider/Spider_Walk.ASE"));

	//path.push_back(make_pair("Walk", "Models/Warrior/Warrior_Walk.ASE"));
	//path.push_back(make_pair("Back", "Models/Warrior/Warrior_Attack1.ASE"));
	//path.push_back(make_pair("Back", "Models/Warrior/Warrior_Attack2.ASE"));
	//path.push_back(make_pair("Back", "Models/Warrior/Warrior_Attack3.ASE"));
	if (FAILED(m_pMesh->Initialize(path)))
		return E_FAIL;

	Mat = new Material;
	Mat->Name = "SpiderMat";
	Mat->MatCBIndex = 4; 
	Mat->DiffuseSrvHeapIndex = 4;
	Mat->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	Mat->FresnelR0 = XMFLOAT3(0.05f, 0.05f, 0.05f);
	Mat->Roughness = 0.3f;

	XMStoreFloat4x4(&World, XMMatrixScaling(m_fScale, m_fScale, m_fScale)*XMMatrixRotationX(1.7f)*XMMatrixRotationZ(3.14f)*XMMatrixTranslation(0.0f, 0.0f, 0.0f));
	TexTransform = MathHelper::Identity4x4();
	ObjCBIndex = 4; 

	Geo = dynamic_cast<DynamicMeshSingle*>(m_pMesh)->m_Geometry[0].get();
	PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	IndexCount = Geo->DrawArgs["SingleMesh"].IndexCount;
	StartIndexLocation = Geo->DrawArgs["SingleMesh"].StartIndexLocation;
	BaseVertexLocation = Geo->DrawArgs["SingleMesh"].BaseVertexLocation;
	Bounds = Geo->DrawArgs["SingleMesh"].Bounds;



	//SetOOBB(XMFLOAT3(Bounds.Center.x *m_fScale, Bounds.Center.y * m_fScale, Bounds.Center.z *m_fScale), XMFLOAT3(Bounds.Extents.x * m_fScale, Bounds.Extents.y * m_fScale, Bounds.Extents.z * m_fScale), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));

	//SetOOBB(XMFLOAT3(Bounds.Center.x *m_fScale, Bounds.Center.y * m_fScale, Bounds.Center.z *m_fScale), XMFLOAT3(100, 100, 100), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));

	
	//SetOOBB(XMFLOAT3(World._41, World._42, World._43), XMFLOAT3(100, 100, 100), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));


	SetOOBB(XMFLOAT3(Bounds.Center.x, Bounds.Center.y , Bounds.Center.z), XMFLOAT3(Bounds.Extents.x, Bounds.Extents.y, Bounds.Extents.z), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));

	//SetOOBB(XMFLOAT3(Bounds.Center.x, Bounds.Center.y, Bounds.Center.z), XMFLOAT3(100, 100, 100), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));

	BuildOOBBRenderer(m_xmOOBB);

	return S_OK;
}

Dragon * Dragon::Create(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice, ComPtr<ID3D12DescriptorHeap>& srv, UINT srvSize)
{
	Dragon* pInstance = new Dragon(d3dDevice, srv, srvSize);

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX(L"Player Created Failed");
		Safe_Release(pInstance);
	}

	return pInstance;
}
