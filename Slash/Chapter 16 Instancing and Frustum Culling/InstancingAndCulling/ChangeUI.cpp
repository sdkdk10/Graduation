
#include "stdafx.h"
#include "ChangeUI.h"

#include "UIMesh.h"
#include "Management.h"
#include "Renderer.h"
#include "Define.h"

ChangeUI::ChangeUI(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice, ComPtr<ID3D12DescriptorHeap> &srv, UINT srvSize, XMFLOAT2 _move, XMFLOAT2 _scale, float _size, int diffuseSrvHeapIndex, float fZ)
	:UI(d3dDevice, srv, srvSize)
{
	move = _move;
	scale = _scale;
	size = _size;
	m_fZ = fZ;
	m_iDiffuseSrvHeapIndex = diffuseSrvHeapIndex;
}


ChangeUI::~ChangeUI()
{
}

ChangeUI * ChangeUI::Create(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice, ComPtr<ID3D12DescriptorHeap>& srv, UINT srvSize, XMFLOAT2 move, XMFLOAT2 scale, float size, int diffuseSrvHeapIndex, float fZ)
{
	ChangeUI* pInstance = new ChangeUI(d3dDevice, srv, srvSize, move, scale, size, diffuseSrvHeapIndex, fZ);

	if (FAILED(pInstance->Initialize(move, scale, size)))
	{
		MSG_BOX(L"Player Created Failed");
		Safe_Release(pInstance);
	}


	return pInstance;
}

bool ChangeUI::Update(const GameTimer & gt)
{
	CGameObject::Update(gt);

	CManagement::GetInstance()->GetRenderer()->Add_RenderGroup(CRenderer::RENDER_UICHANGE, this);

	//XMMATRIX world = XMLoadFloat4x4(&f4x4View);

	auto currObjectCB = m_pFrameResource->ObjectCB.get();

	XMMATRIX world = XMLoadFloat4x4(&World);
	XMMATRIX texTransform = XMLoadFloat4x4(&TexTransform);

	ObjectConstants objConstants;
	XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(world));
	XMStoreFloat4x4(&objConstants.TexTransform, XMMatrixTranspose(texTransform));
	objConstants.MaterialIndex = Mat->MatCBIndex;

	currObjectCB->CopyData(ObjCBIndex, objConstants);


	auto currMaterialCB = m_pFrameResource->MaterialCB.get();

	XMMATRIX matTransform = XMLoadFloat4x4(&Mat->MatTransform);

	MaterialConstants matConstants;

	if (m_IsChange)
	{


		float fDivTime = m_fChangeTime / gt.DeltaTime();

		m_ChangeColor = Vector4::Divide(Vector4::Subtract(m_xmf4ColorChange, XMFLOAT4(1, 1, 1, 1)), fDivTime);

		if (m_ChangeValue)
		{
			Mat->DiffuseAlbedo = Vector4::Add(Mat->DiffuseAlbedo, m_ChangeColor);
		}

		else
		{
			Mat->DiffuseAlbedo = Vector4::Subtract(Mat->DiffuseAlbedo, m_ChangeColor);
		}
		m_fTimeAccc += gt.DeltaTime();

		if (m_fTimeAccc > m_fChangeTime)
		{
			m_ChangeValue = !m_ChangeValue;
			m_fTimeAccc = 0.f;
		}
	}
	matConstants.DiffuseAlbedo = Mat->DiffuseAlbedo;// Mat->DiffuseAlbedo;
	matConstants.FresnelR0 = Mat->FresnelR0;
	matConstants.Roughness = Mat->Roughness;
	XMStoreFloat4x4(&matConstants.MatTransform, XMMatrixTranspose(matTransform));

	matConstants.DiffuseMapIndex = Mat->DiffuseSrvHeapIndex;

	currMaterialCB->CopyData(Mat->MatCBIndex, matConstants);



	return true;
}

void ChangeUI::SetUI(float size, float moveX, float moveY, float scaleX, float scaleY)
{
}

void ChangeUI::Render(ID3D12GraphicsCommandList * cmdList)
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

HRESULT ChangeUI::Initialize(XMFLOAT2 move, XMFLOAT2 scale, float size)
{
	m_pMesh = UIMesh::Create(m_d3dDevice, move, scale, size, m_fZ);

	/* Material Build */
	Mat = new Material;
	Mat->Name = "TerrainMat";
	//Mat->MatCBIndex = 2;
	Mat->MatCBIndex = m_iMyObjectID;
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

void ChangeUI::SetChangeInfo(XMFLOAT4 _xm, float _time)
{
	m_fChangeTime = _time;
	m_xmf4ColorChange = _xm;
}

void ChangeUI::SetisChange(bool _isChange)
{
	m_IsChange = _isChange;

	Mat->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_fTimeAccc = 0.f;
}

void ChangeUI::Free()
{
}
