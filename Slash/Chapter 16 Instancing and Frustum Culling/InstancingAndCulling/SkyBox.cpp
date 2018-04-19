#include "stdafx.h"

#include "GeometryMesh.h"

#include "SkyBox.h"
#include "Define.h"



SkyBox::SkyBox(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice, ComPtr<ID3D12DescriptorHeap> &srv, UINT srvSize)
	: CGameObject(d3dDevice, srv, srvSize)
{
}



SkyBox::~SkyBox()
{
}

bool SkyBox::Update(const GameTimer & gt)
{
	CGameObject::Update(gt);
	auto currObjectCB = m_pFrameResource->ObjectCB.get();

	/* CB Update */
	XMMATRIX world = XMLoadFloat4x4(&World);
	XMMATRIX texTransform = XMLoadFloat4x4(&TexTransform);

	ObjectConstants objConstants;
	XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(world));
	XMStoreFloat4x4(&objConstants.TexTransform, XMMatrixTranspose(texTransform));
	objConstants.MaterialIndex = Mat->MatCBIndex;

	currObjectCB->CopyData(ObjCBIndex, objConstants);

	//////////////////////////////////////////////////

	/* Material Update */
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

void SkyBox::Render(ID3D12GraphicsCommandList * cmdList)
{
	UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
	UINT matCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(MaterialConstants));

	auto objectCB = m_pFrameResource->ObjectCB->Resource();
	auto matCB = m_pFrameResource->MaterialCB->Resource();

	cmdList->IASetVertexBuffers(0, 1, &Geo->VertexBufferView());
	cmdList->IASetIndexBuffer(&Geo->IndexBufferView());
	cmdList->IASetPrimitiveTopology(PrimitiveType);

	D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + ObjCBIndex * objCBByteSize;
	D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress() + Mat->MatCBIndex*matCBByteSize;

	CD3DX12_GPU_DESCRIPTOR_HANDLE skyTexDescriptor(mSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
	skyTexDescriptor.Offset(Mat->DiffuseSrvHeapIndex /*BuildDescriptorHeap¿¡¼­ SkyBox Index*/, mCbvSrvDescriptorSize);
	mCommandList->SetGraphicsRootDescriptorTable(6, skyTexDescriptor);

	cmdList->SetGraphicsRootConstantBufferView(4, objCBAddress);
	cmdList->SetGraphicsRootShaderResourceView(5, matCBAddress);

	//cmdList->SetGraphicsRootDescriptorTable(7, tex);

	cmdList->DrawIndexedInstanced(IndexCount, 1, StartIndexLocation, BaseVertexLocation, 0);
}

HRESULT SkyBox::Initialize()
{
	m_pMesh = new GeometryMesh(m_d3dDevice);

	if (FAILED(m_pMesh->Initialize()))
		return E_FAIL;


	
	/* Material Build */
	Mat = new Material;
	Mat->Name = "SkyBoxMat";
	Mat->MatCBIndex = 6;
	Mat->DiffuseSrvHeapIndex = 6;
	Mat->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	Mat->FresnelR0 = XMFLOAT3(0.05f, 0.05f, 0.05f);
	Mat->Roughness = 0.3f;

	/* CB(World,TextureTranform...) Build */

	XMStoreFloat4x4(&World, XMMatrixScaling(5.0f, 5.0f, 5.0f));
	TexTransform = MathHelper::Identity4x4();
	ObjCBIndex = 6;

	Geo = dynamic_cast<GeometryMesh*>(m_pMesh)->m_Geometry[0].get();
	PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	IndexCount = Geo->DrawArgs["sphere"].IndexCount;
	StartIndexLocation = Geo->DrawArgs["sphere"].StartIndexLocation;
	BaseVertexLocation = Geo->DrawArgs["sphere"].BaseVertexLocation;

	return S_OK;



}

SkyBox * SkyBox::Create(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice, ComPtr<ID3D12DescriptorHeap>& srv, UINT srvSize)
{
	SkyBox* pInstance = new SkyBox(d3dDevice, srv, srvSize);

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX(L"SkyBox Created Failed");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void SkyBox::Free()
{
}
