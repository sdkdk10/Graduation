#include "stdafx.h"
#include "Barrel.h"
#include "Define.h"
#include "StaticMesh.h"


Barrel::Barrel(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice, ComPtr<ID3D12DescriptorHeap> &srv, UINT srvSize) 
	: CGameObject(d3dDevice, srv, srvSize)
{
}


Barrel::~Barrel()
{
}

bool Barrel::Update(const GameTimer & gt)
{
	CGameObject::Update(gt);
	m_pMesh->Update(gt);
	auto currObjectCB = m_pFrameResource->ObjectCB.get();


	//auto currObjectCB2 = m_pFrameResource->InstanceBuffer.get();

	World._43 = 1.0f;
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

void Barrel::Render(ID3D12GraphicsCommandList * cmdList)
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

	cmdList->DrawIndexedInstanced(IndexCount, 1, StartIndexLocation, BaseVertexLocation, 0);


}

HRESULT Barrel::Initialize()
{
	m_pMesh = new StaticMesh(m_d3dDevice);

	/*if (FAILED(m_pMesh->Initialize(L"Idle", "Models/StaticMesh/staticMesh.ASE")))
		return E_FAIL;*/


	vector<pair<const string, const string>> path;
	path.push_back(make_pair("Idle", "Models/StaticMesh/staticMesh.ASE"));


	if (FAILED(m_pMesh->Initialize(path)))
		return E_FAIL;


	Mat = new Material;
	Mat->Name = "BarrelMat";
	Mat->MatCBIndex = 1;
	Mat->DiffuseSrvHeapIndex = 1;
	Mat->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	Mat->FresnelR0 = XMFLOAT3(0.05f, 0.05f, 0.05f);
	Mat->Roughness = 0.3f;

	XMStoreFloat4x4(&World, XMMatrixScaling(0.1f, 0.1f, 0.1f)*XMMatrixTranslation(0.0f, 1.0f, 0.0f));
	TexTransform = MathHelper::Identity4x4();
	ObjCBIndex = 1;

	Geo = dynamic_cast<StaticMesh*>(m_pMesh)->m_Geometry.get();
	PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	IndexCount = Geo->DrawArgs["Barrel"].IndexCount;
	StartIndexLocation = Geo->DrawArgs["Barrel"].StartIndexLocation;
	BaseVertexLocation = Geo->DrawArgs["Barrel"].BaseVertexLocation;

	return S_OK;
}

Barrel * Barrel::Create(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice, ComPtr<ID3D12DescriptorHeap>& srv, UINT srvSize)
{
	Barrel* pInstance = new Barrel(d3dDevice, srv, srvSize);
	{
		if (FAILED(pInstance->Initialize()))
		{
			MSG_BOX(L"Barrel Created Failed");
			Safe_Release(pInstance);
		}
	}
	return pInstance;
}

void Barrel::Free()
{
}
