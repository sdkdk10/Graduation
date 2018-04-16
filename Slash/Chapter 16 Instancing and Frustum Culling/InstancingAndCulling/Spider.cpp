#include "stdafx.h"
#include "Spider.h"
#include "Define.h"
#include "DynamicMeshSingle.h"
#include "InputDevice.h"
#include "BoundingBox.h"


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
	m_pMesh->Update(gt);
	auto currObjectCB = m_pFrameResource->ObjectCB.get();


	//auto currObjectCB2 = m_pFrameResource->InstanceBuffer.get();

	XMMATRIX world = XMLoadFloat4x4(&World);
	XMMATRIX texTransform = XMLoadFloat4x4(&TexTransform);

	ObjectConstants objConstants;
	XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(world));
	XMStoreFloat4x4(&objConstants.TexTransform, XMMatrixTranspose(texTransform));
	objConstants.MaterialIndex = Mat->MatCBIndex;

	currObjectCB->CopyData(ObjCBIndex, objConstants);


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

void Spider::Render(ID3D12GraphicsCommandList * cmdList)
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
	cout << iTest << endl;
	cmdList->DrawIndexedInstanced(pMesh->Indexoffset[1], 1,
		pMesh->Indexoffset[iTest] + pMesh->IndexAnimoffset[0] /*+ pMesh->IndexAnimoffset[0]*/,
		pMesh->Vertexoffset[iTest] + pMesh->VertexAnimoffset[0]/*+ pMesh->VertexAnimoffset[0]*/,0);

	/*	->Indexoffset[1] + Indexoffset[(int)AnimatinoKeyFrameIndex] + IndexAnimoffset[iAnimationState],
		ri->BaseVertexLocation + Vertexoffset[(int)AnimatinoKeyFrameIndex] + VertexAnimoffset[iAnimationState], 0);
*/


	//cmdList->DrawIndexedInstanced(Element_Head.IndexCount, 1, Element_Head.StartIndexLocation, Element_Head.BaseVertexLocation , 0);
	//	dynamic_cast<DynamicMesh*>(m_pMesh)->m_vecIndexOffset[0].
	//cmdList->DrawIndexedInstanced(IndexCount, 1,
	//	dynamic_cast<DynamicMesh*>(m_pMesh)->Indexoffset[1] + dynamic_cast<DynamicMesh*>(m_pMesh)->IndexAnimoffset[1],
	//	dynamic_cast<DynamicMesh*>(m_pMesh)->Vertexoffset[1] + dynamic_cast<DynamicMesh*>(m_pMesh)->VertexAnimoffset[1],
	//	0);


}

HRESULT Spider::Initialize()
{
	m_pMesh = new DynamicMeshSingle(m_d3dDevice);



	vector<pair<const string, const string>> path;
	path.push_back(make_pair("Idle", "Models/Spider/Spider_Idle.ASE"));
	path.push_back(make_pair("Idle", "Models/Spider/Spider_Walk.ASE"));

	//path.push_back(make_pair("Walk", "Models/Warrior/Warrior_Walk.ASE"));
	//path.push_back(make_pair("Back", "Models/Warrior/Warrior_Attack1.ASE"));
	//path.push_back(make_pair("Back", "Models/Warrior/Warrior_Attack2.ASE"));
	//path.push_back(make_pair("Back", "Models/Warrior/Warrior_Attack3.ASE"));
	if (FAILED(m_pMesh->Initialize(path)))
		return E_FAIL;

	Mat = new Material;
	Mat->Name = "SpiderMat";
	Mat->MatCBIndex = 3;
	Mat->DiffuseSrvHeapIndex = 3;
	Mat->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	Mat->FresnelR0 = XMFLOAT3(0.05f, 0.05f, 0.05f);
	Mat->Roughness = 0.3f;

	XMStoreFloat4x4(&World, XMMatrixScaling(2.0f, 2.0f, 2.0f)*XMMatrixTranslation(0.0f, 1.0f, 0.0f)*XMMatrixTranslation(0.0f, 0.0f, 0.0f));
	TexTransform = MathHelper::Identity4x4();
	ObjCBIndex = 3;

	Geo = dynamic_cast<DynamicMeshSingle*>(m_pMesh)->m_Geometry[0].get();
	PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	IndexCount = Geo->DrawArgs["Spider"].IndexCount;
	StartIndexLocation = Geo->DrawArgs["Spider"].StartIndexLocation;
	BaseVertexLocation = Geo->DrawArgs["Spider"].BaseVertexLocation;
	Bounds = Geo->DrawArgs["Spider"].Bounds;



	SetOOBB(XMFLOAT3(Bounds.Center.x * 0.05f, Bounds.Center.y * 0.05f, Bounds.Center.z * 0.05f), XMFLOAT3(Bounds.Extents.x * 0.05f, Bounds.Extents.y * 0.05f, Bounds.Extents.z * 0.05f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));

	BuildOOBBRenderer(m_xmOOBB);


	return S_OK;
}

Spider * Spider::Create(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice, ComPtr<ID3D12DescriptorHeap>& srv, UINT srvSize)
{
	Spider* pInstance = new Spider(d3dDevice, srv, srvSize);

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX(L"Insec Created Failed");
		Safe_Release(pInstance);
	}

	return pInstance;
}
