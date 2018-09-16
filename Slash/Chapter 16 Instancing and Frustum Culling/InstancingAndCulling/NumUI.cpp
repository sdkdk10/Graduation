
#include "stdafx.h"
#include "NumUI.h"
#include "Management.h"
#include "Renderer.h"
#include "Component_Manager.h"
#include "UIMesh.h"
#include "Define.h"

NumUI::NumUI(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice, ComPtr<ID3D12DescriptorHeap> &srv, UINT srvSize, XMFLOAT2 _move, XMFLOAT2 _scale, float _size, int diffuseSrvHeapIndex)
	:UI(d3dDevice, srv, srvSize)
{
	move = _move;
	scale = _scale;
	size = _size;

	m_iDiffuseSrvHeapIndex = diffuseSrvHeapIndex;

	m_pAllObject[m_iAllObjectIndex] = this;
	++m_iAllObjectIndex;
}

NumUI::NumUI(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice, ComPtr<ID3D12DescriptorHeap>& srv, UINT srvSize, wchar_t * wstrUIName, int diffuseSrvHeapIndex)
	: UI(d3dDevice, srv, srvSize)
{
	m_iDiffuseSrvHeapIndex = diffuseSrvHeapIndex;

	m_pAllObject[m_iAllObjectIndex] = this;
	++m_iAllObjectIndex;
}


NumUI::~NumUI()
{
}

NumUI * NumUI::Create(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice, ComPtr<ID3D12DescriptorHeap>& srv, UINT srvSize, XMFLOAT2 move, XMFLOAT2 scale, float size, int diffuseSrvHeapIndex)
{
	NumUI* pInstance = new NumUI(d3dDevice, srv, srvSize, move, scale, size, diffuseSrvHeapIndex);

	if (FAILED(pInstance->Initialize(move, scale, size)))
	{
		MSG_BOX(L"Player Created Failed");
		Safe_Release(pInstance);
	}


	return pInstance;
}

NumUI * NumUI::Create(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice, ComPtr<ID3D12DescriptorHeap>& srv, UINT srvSize, wchar_t * wstrUIName, int diffuseSrvHeapIndex)
{
	NumUI* pInstance = new NumUI(d3dDevice, srv, srvSize, wstrUIName, diffuseSrvHeapIndex);

	if (FAILED(pInstance->Initialize(wstrUIName)))
	{
		MSG_BOX(L"Player Created Failed");
		Safe_Release(pInstance);
	}


	return pInstance;
}

bool NumUI::Update(const GameTimer & gt)
{
	CGameObject::Update(gt);
	
	for (auto& elem : m_vNum)
	{
		auto currObjectCB = m_pFrameResource->ObjectCB.get();

		XMMATRIX world = XMLoadFloat4x4(&World);
		XMMATRIX texTransform = XMLoadFloat4x4(&TexTransform);

		ObjectConstants objConstants;
		XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(world));
		XMStoreFloat4x4(&objConstants.TexTransform, XMMatrixTranspose(texTransform));
		objConstants.MaterialIndex = Mat->MatCBIndex;

		currObjectCB->CopyData(ObjCBIndex, objConstants);


		auto currMaterialCB = m_pFrameResource->MaterialCB.get();

		Mat->MatTransform._41 = elem.instData.TexTransform._41;
		XMMATRIX matTransform = XMLoadFloat4x4(&Mat->MatTransform);

		MaterialConstants matConstants;
		matConstants.DiffuseAlbedo = Mat->DiffuseAlbedo;
		matConstants.FresnelR0 = Mat->FresnelR0;
		matConstants.Roughness = Mat->Roughness;
		XMStoreFloat4x4(&matConstants.MatTransform, XMMatrixTranspose(matTransform));


		matConstants.DiffuseMapIndex = Mat->DiffuseSrvHeapIndex;

		currMaterialCB->CopyData(Mat->MatCBIndex, matConstants);
	}


	CManagement::GetInstance()->GetRenderer()->Add_RenderGroup(CRenderer::RENDER_UICHANGE, this);
	return true;
}

void NumUI::SetUI(float size, float moveX, float moveY, float scaleX, float scaleY)
{
}

void NumUI::Render(ID3D12GraphicsCommandList * cmdList)
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

HRESULT NumUI::Initialize(XMFLOAT2 move, XMFLOAT2 scale, float size)
{
	//m_pMesh = UIMesh::Create(m_d3dDevice, move, scale, size);
	//Com_Mesh_WarriorUI
	m_pMesh = dynamic_cast<UIMesh*>(CComponent_Manager::GetInstance()->Clone_Component(L"Com_Mesh_WarriorUI"));
	if (m_pMesh == nullptr)
		return E_FAIL;
	/* Material Build */
	Mat = new Material;
	Mat->Name = "TerrainMat";
	Mat->MatCBIndex = m_iMyObjectID;
	Mat->DiffuseSrvHeapIndex = m_iDiffuseSrvHeapIndex;//7;
	Mat->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.7f);
	Mat->FresnelR0 = XMFLOAT3(0.05f, 0.05f, 0.05f);
	Mat->Roughness = 0.3f;

	/* CB(World,TextureTranform...) Build */

	Mat->MatTransform(0, 0) = 0.1f;
	Mat->MatTransform(1, 1) = 1.f;

	TexTransform = MathHelper::Identity4x4();
	ObjCBIndex = m_iMyObjectID;

	Geo = dynamic_cast<UIMesh*>(m_pMesh)->m_Geometry[0].get();
	PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	IndexCount = Geo->DrawArgs["UI"].IndexCount;
	StartIndexLocation = Geo->DrawArgs["UI"].StartIndexLocation;
	BaseVertexLocation = Geo->DrawArgs["UI"].BaseVertexLocation;

	return S_OK;
}

HRESULT NumUI::Initialize(wchar_t* uiName)
{
	m_pMesh = dynamic_cast<UIMesh*>(CComponent_Manager::GetInstance()->Clone_Component(uiName));
	if (m_pMesh == nullptr)
		return E_FAIL;
	/* Material Build */
	Mat = new Material;
	Mat->Name = "TerrainMat";
	Mat->MatCBIndex = m_iMyObjectID;
	Mat->DiffuseSrvHeapIndex = m_iDiffuseSrvHeapIndex;//7;
	Mat->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	Mat->FresnelR0 = XMFLOAT3(0.05f, 0.05f, 0.05f);
	Mat->Roughness = 0.3f;

	Mat->MatTransform(0, 0) = 0.1f;
	Mat->MatTransform(1, 1) = 1.f;

	/* CB(World,TextureTranform...) Build */

	TexTransform = MathHelper::Identity4x4();
	ObjCBIndex = m_iMyObjectID;
	World = MathHelper::Identity4x4();

	Geo = dynamic_cast<UIMesh*>(m_pMesh)->m_Geometry[0].get();
	PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	IndexCount = Geo->DrawArgs["UI"].IndexCount;
	StartIndexLocation = Geo->DrawArgs["UI"].StartIndexLocation;
	BaseVertexLocation = Geo->DrawArgs["UI"].BaseVertexLocation;

	SetNum(1);

	return S_OK;
}

void NumUI::SetColor(float r, float g, float b, float a)
{
	Mat->DiffuseAlbedo.x = r;
	Mat->DiffuseAlbedo.y = g;
	Mat->DiffuseAlbedo.z = b;
	Mat->DiffuseAlbedo.w = a;
}

void NumUI::SetNum(int iNum)
{
	m_vNum.clear();
	int iCnt = 0;
	for (int i = 4; i >= 0; --i)
	{
		if (iNum / int(pow(10, i)) > 0)
		{
			iCnt = i + 1;
			break;
		}
	}
	float fNext = 1.f;
	//float fX = f3Pos.x;
	float fZ = 0.001f;
	for (int i = 0; i < iCnt; ++i)
	{
		tagNumUI tNum;
		int iTexNum = (iNum / int(pow(10, iCnt - 1 - i)));

		iNum = iNum % int(pow(10, iCnt - 1 - i));

		tNum.instData.TexTransform._41 = float(iTexNum) / 10.f;
		tNum.instData.World = World;
		//tNum.instData.World._41 = fX;
		//fX += fNext;
	//	tNum.instData.World._42 = f3Pos.y;
		//tNum.instData.World._43 = f3Pos.z + (fZ*i);

		m_vNum.push_back(tNum);
	}
}

void NumUI::Free()
{
}
