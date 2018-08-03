
#include "stdafx.h"
#include "StaticUI.h"

#include "UIMesh.h"
#include "Management.h"
#include "Renderer.h"
#include "Define.h"

StaticUI::StaticUI(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice, ComPtr<ID3D12DescriptorHeap> &srv, UINT srvSize, XMFLOAT2 _move, XMFLOAT2 _scale, float _size, int diffuseSrvHeapIndex)
	:UI(d3dDevice, srv, srvSize)
{
	move = _move;
	scale = _scale;
	size = _size;

	m_iDiffuseSrvHeapIndex = diffuseSrvHeapIndex;
}


StaticUI::~StaticUI()
{
}

StaticUI * StaticUI::Create(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice, ComPtr<ID3D12DescriptorHeap>& srv, UINT srvSize, XMFLOAT2 move, XMFLOAT2 scale, float size, int diffuseSrvHeapIndex)
{
	StaticUI* pInstance = new StaticUI(d3dDevice, srv, srvSize, move, scale, size, diffuseSrvHeapIndex);

	if (FAILED(pInstance->Initialize(move, scale, size)))
	{
		MSG_BOX(L"Player Created Failed");
		Safe_Release(pInstance);
	}


	return pInstance;
}

bool StaticUI::Update(const GameTimer & gt)
{
	CGameObject::Update(gt);

	CManagement::GetInstance()->GetRenderer()->Add_RenderGroup(CRenderer::RENDER_UI, this);
	return true;
}

void StaticUI::SetUI(float size, float moveX, float moveY, float scaleX, float scaleY)
{
}

void StaticUI::Render(ID3D12GraphicsCommandList * cmdList)
{
	UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
	UINT matCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(MaterialConstants));

	auto objectCB = m_pFrameResource->ObjectCB->Resource();
	auto matCB = m_pFrameResource->MaterialCB->Resource();

	cmdList->IASetVertexBuffers(0, 1, &Geo->VertexBufferView());
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


	cmdList->DrawInstanced(6, 1, 0, 0);
}

HRESULT StaticUI::Initialize(XMFLOAT2 move, XMFLOAT2 scale, float size)
{
	m_pMesh = UIMesh::Create(m_d3dDevice, move, scale, size);

	/* Material Build */
	Mat = new Material;
	Mat->Name = "TerrainMat";
	Mat->MatCBIndex = 2;
	Mat->DiffuseSrvHeapIndex = m_iDiffuseSrvHeapIndex;//7;
	Mat->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	Mat->FresnelR0 = XMFLOAT3(0.05f, 0.05f, 0.05f);
	Mat->Roughness = 0.3f;

	/* CB(World,TextureTranform...) Build */

	TexTransform = MathHelper::Identity4x4();
	ObjCBIndex = m_iMyObjectID;

	Geo = dynamic_cast<UIMesh*>(m_pMesh)->m_Geometry[0].get();
	PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	IndexCount = Geo->DrawArgs["UI"].IndexCount;
	StartIndexLocation = Geo->DrawArgs["UI"].StartIndexLocation;
	BaseVertexLocation = Geo->DrawArgs["UI"].BaseVertexLocation;

	return S_OK;
}

void StaticUI::Free()
{
}
