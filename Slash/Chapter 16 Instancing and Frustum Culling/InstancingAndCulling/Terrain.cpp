#include "stdafx.h"
#include "GeometryMesh.h"
#include "Terrain.h"
#include "Define.h"
#include "Component_Manager.h"
#include "Texture_Manager.h"

Terrain::Terrain(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice, ComPtr<ID3D12DescriptorHeap>& srv, UINT srvSize)
	: CGameObject(d3dDevice, srv, srvSize)
{
}

Terrain::~Terrain()
{
}

bool Terrain::Update(const GameTimer & gt)
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

void Terrain::Render(ID3D12GraphicsCommandList * cmdList)
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

HRESULT Terrain::Initialize()
{
<<<<<<< HEAD
	
=======
>>>>>>> 4dac4ec56cadf5ef87d42169e83ea1d1d355dfc3
	m_pMesh = dynamic_cast<GeometryMesh*>(CComponent_Manager::GetInstance()->Clone_Component(L"Com_Mesh_Geometry"));
	if (nullptr == m_pMesh)
		return E_FAIL;

	Texture* tex = CTexture_Manager::GetInstance()->Find_Texture("stoneTex", CTexture_Manager::TEX_DEFAULT_2D);
	if (nullptr == tex)
		return E_FAIL;


	/* Material Build */
	Mat = new Material;
	Mat->Name = "TerrainMat";
	Mat->MatCBIndex = 2;
	Mat->DiffuseSrvHeapIndex = tex->Num;
	Mat->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	Mat->FresnelR0 = XMFLOAT3(0.05f, 0.05f, 0.05f);
	Mat->Roughness = 0.3f;

	/* CB(World,TextureTranform...) Build */

	XMStoreFloat4x4(&World, XMMatrixScaling(5.0f, 1.0f, 5.0f));
	TexTransform = MathHelper::Identity4x4();
	ObjCBIndex = m_iMyObjectID;

	Geo = dynamic_cast<GeometryMesh*>(m_pMesh)->m_Geometry[0].get();
	PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	IndexCount = Geo->DrawArgs["grid"].IndexCount;
	StartIndexLocation = Geo->DrawArgs["grid"].StartIndexLocation;
	BaseVertexLocation = Geo->DrawArgs["grid"].BaseVertexLocation;

	return S_OK;

}

Terrain * Terrain::Create(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice, ComPtr<ID3D12DescriptorHeap>& srv, UINT srvSize)
{
	Terrain* pInstance = new Terrain(d3dDevice, srv, srvSize);

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX(L"Terrain Created Failed");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void Terrain::Free()
{
}
