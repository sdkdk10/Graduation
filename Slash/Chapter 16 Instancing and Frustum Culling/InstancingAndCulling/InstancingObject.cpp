#include "stdafx.h"
#include "InstancingObject.h"
#include "Define.h"
#include "StaticMesh.h"
#include "Camera.h"
#include "Management.h"
#include "Transform.h"
#include "Component_Manager.h"

CInstancingObject::CInstancingObject(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice, ComPtr<ID3D12DescriptorHeap> &srv, UINT srvSize, wchar_t* pMesh, int iSize)
	: CGameObject(d3dDevice, srv, srvSize)
{
	m_pwstrMeshName = pMesh;
	m_iSize = iSize;
}

CInstancingObject::~CInstancingObject()
{
}

HRESULT CInstancingObject::Initialize()
{
	/*m_pMesh = new StaticMesh(m_d3dDevice);

	vector<pair<const string, const string>> path;
	path.push_back(make_pair("Idle", "Models/StaticMesh/staticMesh.ASE"));

	if (FAILED(m_pMesh->Initialize(path)))
		return E_FAIL;*/

	m_pMesh = dynamic_cast<StaticMesh*>(CComponent_Manager::GetInstance()->Clone_Component(m_pwstrMeshName));

	auto bricks0 = std::make_unique<Material>();
	bricks0->Name = "bricks0";
	bricks0->MatCBIndex = 0;
	bricks0->DiffuseSrvHeapIndex = 0;
	bricks0->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	bricks0->FresnelR0 = XMFLOAT3(0.02f, 0.02f, 0.02f);
	bricks0->Roughness = 0.1f;

	auto stone0 = std::make_unique<Material>();
	stone0->Name = "stone0";
	stone0->MatCBIndex = 1;
	stone0->DiffuseSrvHeapIndex = 1;
	stone0->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	stone0->FresnelR0 = XMFLOAT3(0.05f, 0.05f, 0.05f);
	stone0->Roughness = 0.3f;

	auto tile0 = std::make_unique<Material>();
	tile0->Name = "tile0";
	tile0->MatCBIndex = 2;
	tile0->DiffuseSrvHeapIndex = 2;
	tile0->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	tile0->FresnelR0 = XMFLOAT3(0.02f, 0.02f, 0.02f);
	tile0->Roughness = 0.3f;

	auto crate0 = std::make_unique<Material>();
	crate0->Name = "checkboard0";
	crate0->MatCBIndex = 3;
	crate0->DiffuseSrvHeapIndex = 3;
	crate0->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	crate0->FresnelR0 = XMFLOAT3(0.05f, 0.05f, 0.05f);
	crate0->Roughness = 0.2f;

	auto ice0 = std::make_unique<Material>();
	ice0->Name = "ice0";
	ice0->MatCBIndex = 4;
	ice0->DiffuseSrvHeapIndex = 4;
	ice0->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	ice0->FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
	ice0->Roughness = 0.0f;

	auto grass0 = std::make_unique<Material>();
	grass0->Name = "grass0";
	grass0->MatCBIndex = 5;
	grass0->DiffuseSrvHeapIndex = 5;
	grass0->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	grass0->FresnelR0 = XMFLOAT3(0.05f, 0.05f, 0.05f);
	grass0->Roughness = 0.2f;

	auto skullMat = std::make_unique<Material>();
	skullMat->Name = "skullMat";
	skullMat->MatCBIndex = 6;
	skullMat->DiffuseSrvHeapIndex = 6;
	skullMat->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	skullMat->FresnelR0 = XMFLOAT3(0.05f, 0.05f, 0.05f);
	skullMat->Roughness = 0.5f;

	mMaterials["bricks0"] = std::move(bricks0);
	mMaterials["stone0"] = std::move(stone0);
	mMaterials["tile0"] = std::move(tile0);
	mMaterials["crate0"] = std::move(crate0);
	mMaterials["ice0"] = std::move(ice0);
	mMaterials["grass0"] = std::move(grass0);
	mMaterials["skullMat"] = std::move(skullMat);

	//Mat = new Material;
	//Mat->Name = "instanceMat";
	//Mat->MatCBIndex = 0;
	//Mat->DiffuseSrvHeapIndex = 0;
	//Mat->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	//Mat->FresnelR0 = XMFLOAT3(0.05f, 0.05f, 0.05f);
	//Mat->Roughness = 0.5f;

	World = MathHelper::Identity4x4();
	TexTransform = MathHelper::Identity4x4();
	ObjCBIndex = 0;

	Geo = dynamic_cast<StaticMesh*>(m_pMesh)->m_Geometry[0].get();
	PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	InstanceCount = 0;
	IndexCount = Geo->DrawArgs["Barrel"].IndexCount;
	StartIndexLocation = Geo->DrawArgs["Barrel"].StartIndexLocation;
	BaseVertexLocation = Geo->DrawArgs["Barrel"].BaseVertexLocation;
	Bounds = Geo->DrawArgs["Barrel"].Bounds;

	Geo_Bounds = dynamic_cast<StaticMesh*>(m_pMesh)->m_Geometry[1].get();
	PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	InstanceCount = 0;
	Element_Bounds.IndexCount = Geo_Bounds->DrawArgs["BarrelBounds"].IndexCount;
	Element_Bounds.StartIndexLocation = Geo_Bounds->DrawArgs["BarrelBounds"].StartIndexLocation;
	Element_Bounds.BaseVertexLocation = Geo_Bounds->DrawArgs["BarrelBounds"].BaseVertexLocation;
//	m_GeoBounds = 

	// Generate instance data.
	//const int n = 5;
	//vecInstances.resize(n*n*n);
	//vecInstances.resize(m_iSize *m_iSize *m_iSize);
	//m_vecTransCom.resize(m_iSize *m_iSize *m_iSize);

	//float width = 200.0f;
	//float height = 200.0f;
	//float depth = 200.0f;

	//float x = -0.5f*width;
	//float y = -0.5f*height;
	//float z = -0.5f*depth;
	//float dx = width / (m_iSize - 1);
	//float dy = height / (m_iSize - 1);
	//float dz = depth / (m_iSize - 1);
	//for (int k = 0; k < m_iSize; ++k)
	//{
	//	for (int i = 0; i < m_iSize; ++i)
	//	{
	//		for (int j = 0; j < m_iSize; ++j)
	//		{
	//			int index = k * m_iSize*m_iSize + i *m_iSize + j;
	//			// Position instanced along a 3D grid.
	//			vecInstances[index].World = XMFLOAT4X4(
	//				1.0f, 0.0f, 0.0f, 0.0f,
	//				0.0f, 1.0f, 0.0f, 0.0f,
	//				0.0f, 0.0f, 1.0f, 0.0f,
	//				x + j * dx, y + i * dy, z + k * dz, 1.0f);

	//			CTransform* pCom = CTransform::Create(this);
	//			pCom->GetPosition() = XMFLOAT3(x + j * dx, y + i * dy, z + k * dz);
	//			pCom->GetScale() = XMFLOAT3(1.f, 1.f, 1.f);
	//			//m_vecTransCom.push_back(pCom);
	//			m_vecTransCom[index] = pCom;
	//			
	//			XMStoreFloat4x4(&vecInstances[index].TexTransform, XMMatrixScaling(2.0f, 2.0f, 1.0f));
	//			//vecInstances[index].MaterialIndex = index % (mMaterials.size() - 1);
	//			vecInstances[index].MaterialIndex = index % 6;
	//		}
	//	}
	//}

	vecInstances.resize(m_iSize);
	m_vecTransCom.resize(m_iSize);

	for (int i = 0; i < m_iSize; ++i)
	{
		vecInstances[i].World = XMFLOAT4X4(
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.f, 0.f, 0.f, 1.0f);

		CTransform* pCom = CTransform::Create(this);
		pCom->GetPosition().x = i * 10.f;
		m_vecTransCom[i] = pCom;
		XMStoreFloat4x4(&vecInstances[i].TexTransform, XMMatrixScaling(2.0f, 2.0f, 1.0f));
		//vecInstances[index].MaterialIndex = index % (mMaterials.size() - 1);
		vecInstances[i].MaterialIndex = i % 6;
	}

	return S_OK;
}

bool CInstancingObject::Update(const GameTimer & gt)
{
	CGameObject::Update(gt);
	m_pCamera = CManagement::GetInstance()->Get_MainCam();
	XMMATRIX view = m_pCamera->GetView();
	XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);

	auto currInstanceBuffer = m_pFrameResource->InstanceBuffer.get();
	const auto& instanceData = vecInstances;

	int visibleInstanceCount = 0;

	auto Player = CManagement::GetInstance()->Find_Object(L"Layer_Player", 0);

	for (UINT i = 0; i < (UINT)instanceData.size(); ++i)
	{
		//XMMATRIX world = XMLoadFloat4x4(&instanceData[i].World);
		m_vecTransCom[i]->Update_Component(gt);
		XMMATRIX world = XMLoadFloat4x4(&m_vecTransCom[i]->GetWorld());
		XMMATRIX texTransform = XMLoadFloat4x4(&instanceData[i].TexTransform);

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
			InstanceData data;
			XMStoreFloat4x4(&data.World, XMMatrixTranspose(world));
			XMStoreFloat4x4(&data.TexTransform, XMMatrixTranspose(texTransform));
			data.MaterialIndex = instanceData[i].MaterialIndex;

			// Write the instance data to structured buffer for the visible objects. º¸¿©? ¤¾¤· 
			currInstanceBuffer->CopyData(visibleInstanceCount++, data);
		}
	}

	InstanceCount = visibleInstanceCount;



	/* Material */
	auto currMaterialBuffer = m_pFrameResource->MaterialBuffer.get();

	for (auto& e : mMaterials)
	{
		// Only update the cbuffer data if the constants have changed.  If the cbuffer
		// data changes, it needs to be updated for each FrameResource.
		Material* mat = e.second.get();
		if (mat->NumFramesDirty > 0)
		{
			XMMATRIX matTransform = XMLoadFloat4x4(&mat->MatTransform);

			MaterialData matData;
			matData.DiffuseAlbedo = mat->DiffuseAlbedo;
			matData.FresnelR0 = mat->FresnelR0;
			matData.Roughness = mat->Roughness;
			XMStoreFloat4x4(&matData.MatTransform, XMMatrixTranspose(matTransform));
			matData.DiffuseMapIndex = mat->DiffuseSrvHeapIndex;

			currMaterialBuffer->CopyData(mat->MatCBIndex, matData);

			// Next FrameResource need to be updated too.
			mat->NumFramesDirty--;
		}
	}

	return true;
}

void CInstancingObject::Render(ID3D12GraphicsCommandList * cmdList)
{

	//RenderBounds(cmdList);
	
	mCommandList->SetGraphicsRootDescriptorTable(3, mSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
	//UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
	cmdList->IASetVertexBuffers(0, 1, &Geo->VertexBufferView());
	cmdList->IASetIndexBuffer(&Geo->IndexBufferView());
	cmdList->IASetPrimitiveTopology(PrimitiveType);

	// Set the instance buffer to use for this render-item.  For structured buffers, we can bypass 
	// the heap and set as a root descriptor.

	auto instanceBuffer = m_pFrameResource->InstanceBuffer->Resource();
	mCommandList->SetGraphicsRootShaderResourceView(0, instanceBuffer->GetGPUVirtualAddress());
	
	InstanceCount = vecInstances.size();
	cmdList->DrawIndexedInstanced(IndexCount, InstanceCount, StartIndexLocation, BaseVertexLocation, 0);

	

}

CInstancingObject * CInstancingObject::Create(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice, ComPtr<ID3D12DescriptorHeap>& srv, UINT srvSize, wchar_t* pMesh, int iSize)
{
	CInstancingObject* pInstance = new CInstancingObject(d3dDevice, srv, srvSize, pMesh, iSize);

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX(L"CInstancingObject Created Failed");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CInstancingObject::RenderBounds(ID3D12GraphicsCommandList * cmdList)
{
	//UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
	cmdList->IASetVertexBuffers(0, 1, &Geo_Bounds->VertexBufferView());
	cmdList->IASetIndexBuffer(&Geo_Bounds->IndexBufferView());
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);

	// Set the instance buffer to use for this render-item.  For structured buffers, we can bypass 
	// the heap and set as a root descriptor.

	auto instanceBuffer = m_pFrameResource->InstanceBuffer->Resource();
	mCommandList->SetGraphicsRootShaderResourceView(0, instanceBuffer->GetGPUVirtualAddress());




	cmdList->DrawIndexedInstanced(Element_Bounds.IndexCount, InstanceCount, Element_Bounds.StartIndexLocation, Element_Bounds.BaseVertexLocation, 0);





	////////////////////////////////////////////////////////////
}

void CInstancingObject::Free()
{
	
}
