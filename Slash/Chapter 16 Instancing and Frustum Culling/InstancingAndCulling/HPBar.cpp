#include "stdafx.h"
#include "HPBar.h"

#include "UIMesh.h"
#include "Management.h"
#include "Renderer.h"
#include "Define.h"

HPBar::HPBar(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice, ComPtr<ID3D12DescriptorHeap> &srv, UINT srvSize, XMFLOAT2 _move, XMFLOAT2 _scale, float _size, int diffuseSrvHeapIndex)
	:UI(d3dDevice, srv, srvSize)
{
	move = _move;
	scale = _scale;
	size = _size;
	m_iDiffuseSrvHeapIndex = diffuseSrvHeapIndex;
}


HPBar::~HPBar()
{
}

HPBar * HPBar::Create(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice, ComPtr<ID3D12DescriptorHeap>& srv, UINT srvSize, XMFLOAT2 move, XMFLOAT2 scale, float size,int diffuseSrvHeapIndex)
{
	HPBar* pInstance = new HPBar(d3dDevice, srv, srvSize, move, scale, size,diffuseSrvHeapIndex);

	if (FAILED(pInstance->Initialize(move, scale, size)))
	{
		MSG_BOX(L"Player Created Failed");
		Safe_Release(pInstance);
	}


	return pInstance;
}

bool HPBar::Update(const GameTimer & gt)
{

	CGameObject::Update(gt);

	auto currVB = m_pFrameResource->VB.get();

	//scale.x -= 0.0001f;
	Vertex v;
	
	//»ï°¢Çü 1 - 0 1 2
	//»ï°¢Çü 2 - 3(0) 4 5(1)

	v.Pos = XMFLOAT3((-size + move.x), (size + move.y) * scale.y, 0.0f); // 0 
	v.TexC = XMFLOAT2(0.0f, 0.0f);

	currVB->CopyData(0, v);

	v.Pos = XMFLOAT3((-size + move.x) + 2 * size * GetHp()/200.0f, (-size + move.y) * scale.y, 0.0f); //1
	v.TexC = XMFLOAT2(1.0f + (size + move.x)* (GetHp() / (float)200), 1.0f);

	currVB->CopyData(1, v);

	v.Pos = XMFLOAT3((-size + move.x), (-size + move.y) * scale.y, 0.0f); //2
	v.TexC = XMFLOAT2(0.0f, 1.0f);

	currVB->CopyData(2, v);


	v.Pos = XMFLOAT3((-size + move.x), (size + move.y) * scale.y, 0.0f); //3
	v.TexC = XMFLOAT2(0.0f, 0.0f);

	currVB->CopyData(3, v);

	v.Pos = XMFLOAT3((-size + move.x) + 2 * size * GetHp() / 200.0f, (size + move.y) * scale.y, 0.0f); //4
	v.TexC = XMFLOAT2(1.0f + (size + move.x)* (GetHp() / (float)200) + 0.05, 0.0f);


	currVB->CopyData(4, v);

	v.Pos = XMFLOAT3((-size + move.x) + 2 * size * GetHp() / 200.0f, (-size + move.y) * scale.y, 0.0f); //5
	v.TexC = XMFLOAT2(1.0f + (size + move.x)* (GetHp() / (float)200), 1.0f);

	
	currVB->CopyData(5, v);

	
	Geo->VertexBufferGPU = currVB->Resource();

	CManagement::GetInstance()->GetRenderer()->Add_RenderGroup(CRenderer::RENDER_UI, this);

	return true;
}

void HPBar::SetUI(float size, float moveX, float moveY, float scaleX, float scaleY)
{
}

void HPBar::Render(ID3D12GraphicsCommandList * cmdList)
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

HRESULT HPBar::Initialize(XMFLOAT2 move, XMFLOAT2 scale, float size)
{
	/*m_pMesh = new UIMesh(m_d3dDevice);

	if (FAILED(m_pMesh->Initialize()))
	return E_FAIL;*/

	m_pMesh = UIMesh::Create(m_d3dDevice, move, scale, size);

	/* Material Build */
	Mat = new Material;
	Mat->Name = "TerrainMat";
	Mat->MatCBIndex = m_iMyObjectID;
	Mat->DiffuseSrvHeapIndex = m_iDiffuseSrvHeapIndex;// 6;
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


void HPBar::Free()
{
}
