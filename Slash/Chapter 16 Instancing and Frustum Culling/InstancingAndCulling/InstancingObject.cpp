#include "stdafx.h"
#include "InstancingObject.h"
#include "Define.h"
#include "StaticMesh.h"
#include "Camera.h"
#include "Management.h"
#include "Transform.h"
#include "Component_Manager.h"

CInstancingObject* CInstancingObject::m_pAllInstObject[MAXINSTOBJECTID] = { nullptr };
unsigned long CInstancingObject::m_iAllInstObjectIndex = 0;

CInstancingObject::CInstancingObject(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice, ComPtr<ID3D12DescriptorHeap> &srv, UINT srvSize, wchar_t* pMesh, int iSize)
	: CGameObject(d3dDevice, srv, srvSize)
{
	//while (m_pAllInstObject[m_iAllInstObjectIndex++])
	//{
	//	m_iAllInstObjectIndex %= MAXOBJECTID;
	//}
	//m_pAllInstObject[m_iAllInstObjectIndex] = this;
	//m_iMyObjectID = m_iAllInstObjectIndex;

	m_iMyInstObject = m_iAllInstObjectIndex;
	m_iAllInstObjectIndex += iSize;
	m_iAllObjectIndex += iSize;

	m_pwstrMeshName = pMesh;
	m_iSize = iSize;
}

CInstancingObject::~CInstancingObject()
{
}

HRESULT CInstancingObject::Initialize()
{
<<<<<<< HEAD

=======
>>>>>>> a549a07b1fbd6cc03621ef7e65224284684e3fd7
	m_pMesh = dynamic_cast<StaticMesh*>(CComponent_Manager::GetInstance()->Clone_Component(m_pwstrMeshName));
	if (nullptr == m_pMesh)
		return E_FAIL;

	auto bricks0 = std::make_unique<Material>();
	bricks0->Name = "bricks0";
	bricks0->MatCBIndex = 0;
	bricks0->DiffuseSrvHeapIndex = 0;
	bricks0->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	bricks0->FresnelR0 = XMFLOAT3(0.02f, 0.02f, 0.02f);
	bricks0->Roughness = 0.1f;
	
	mMaterials.push_back(std::move(bricks0));

	auto stone0 = std::make_unique<Material>();
	stone0->Name = "stone0";
	stone0->MatCBIndex = 1;
	stone0->DiffuseSrvHeapIndex = 1;
	stone0->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	stone0->FresnelR0 = XMFLOAT3(0.05f, 0.05f, 0.05f);
	stone0->Roughness = 0.3f;

	mMaterials.push_back(std::move(stone0));

	auto tile0 = std::make_unique<Material>();
	tile0->Name = "tile0";
	tile0->MatCBIndex = 2;
	tile0->DiffuseSrvHeapIndex = 2;
	tile0->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	tile0->FresnelR0 = XMFLOAT3(0.02f, 0.02f, 0.02f);
	tile0->Roughness = 0.3f;

	mMaterials.push_back(std::move(tile0));

	auto crate0 = std::make_unique<Material>();
	crate0->Name = "checkboard0";
	crate0->MatCBIndex = 3;
	crate0->DiffuseSrvHeapIndex = 3;
	crate0->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	crate0->FresnelR0 = XMFLOAT3(0.05f, 0.05f, 0.05f);
	crate0->Roughness = 0.2f;

	mMaterials.push_back(std::move(crate0));

	auto ice0 = std::make_unique<Material>();
	ice0->Name = "ice0";
	ice0->MatCBIndex = 4;
	ice0->DiffuseSrvHeapIndex = 4;
	ice0->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	ice0->FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
	ice0->Roughness = 0.0f;

	mMaterials.push_back(std::move(ice0));

	auto grass0 = std::make_unique<Material>();
	grass0->Name = "grass0";
	grass0->MatCBIndex = 5;
	grass0->DiffuseSrvHeapIndex = 5;
	grass0->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	grass0->FresnelR0 = XMFLOAT3(0.05f, 0.05f, 0.05f);
	grass0->Roughness = 0.2f;

	mMaterials.push_back(std::move(grass0));

	auto skullMat = std::make_unique<Material>();
	skullMat->Name = "skullMat";
	skullMat->MatCBIndex = 6;
	skullMat->DiffuseSrvHeapIndex = 6;
	skullMat->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	skullMat->FresnelR0 = XMFLOAT3(0.05f, 0.05f, 0.05f);
	skullMat->Roughness = 0.5f;

	mMaterials.push_back(std::move(skullMat));

	//mMaterials["bricks0"] = std::move(bricks0);
	//mMaterials["stone0"] = std::move(stone0);
	//mMaterials["tile0"] = std::move(tile0);
	//mMaterials["crate0"] = std::move(crate0);
	//mMaterials["ice0"] = std::move(ice0);
	//mMaterials["grass0"] = std::move(grass0);
	//mMaterials["skullMat"] = std::move(skullMat);

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
		vecInstances[i].MaterialIndex = i;
	}

	return S_OK;
}

bool CInstancingObject::Update(const GameTimer & gt)
{
<<<<<<< HEAD
	
=======

	
	CGameObject::Update(gt);
>>>>>>> a549a07b1fbd6cc03621ef7e65224284684e3fd7
	m_pCamera = CManagement::GetInstance()->Get_MainCam();
	XMMATRIX view = m_pCamera->GetView();
	XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);

<<<<<<< HEAD
	const auto& instanceData = vecInstances;
=======
	auto currInstanceBuffer = m_pFrameResource->InstanceBuffer.get();
	auto& instanceData = vecInstances;
>>>>>>> a549a07b1fbd6cc03621ef7e65224284684e3fd7

	int visibleInstanceCount = 0;


	int iTest = m_iMyInstObject;
	for (UINT i = 0; i < (UINT)m_iSize; ++i)
	{
		//XMMATRIX world = XMLoadFloat4x4(&instanceData[i].World);
		vecInstances[i].World = m_vecTransCom[i]->GetWorld();
		m_vecTransCom[i]->Update_Component(gt);

		instanceData[i].World = m_vecTransCom[i]->GetWorld();
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
			CGameObject::Update(gt);
			collisionTagTest = i;
			Animate(gt, m_vecTransCom[i]);
			auto currInstanceBuffer = m_pFrameResource->InstanceBuffer.get();

			InstanceData data;
			XMStoreFloat4x4(&data.World, XMMatrixTranspose(world));
			XMStoreFloat4x4(&data.TexTransform, XMMatrixTranspose(texTransform));
			data.MaterialIndex = instanceData[i].MaterialIndex;

			// Write the instance data to structured buffer for the visible objects. 보여? ㅎㅇ 
			currInstanceBuffer->CopyData(iTest++, data);
			visibleInstanceCount++;
		}
	}

	InstanceCount = visibleInstanceCount;
	cout << m_pwstrMeshName << " : " << endl;
	cout << InstanceCount << endl;

	//cout << InstanceCount << endl;


	/* Material */
	auto currMaterialBuffer = m_pFrameResource->MaterialBuffer.get();

	for (auto& e : mMaterials)
	{
		// Only update the cbuffer data if the constants have changed.  If the cbuffer
		// data changes, it needs to be updated for each FrameResource.
		Material* mat = e.get();
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
	D3D12_GPU_VIRTUAL_ADDRESS Address = instanceBuffer->GetGPUVirtualAddress() + m_iMyInstObject * sizeof(InstanceData);
	mCommandList->SetGraphicsRootShaderResourceView(0, Address);
	
	InstanceCount = vecInstances.size();
	cmdList->DrawIndexedInstanced(IndexCount, InstanceCount, StartIndexLocation, BaseVertexLocation, 0);

	

}

void CInstancingObject::Animate(const GameTimer & gt)
{


}

void CInstancingObject::Animate(const GameTimer & gt, CTransform * transform)
{
	auto m_pPlayer = CManagement::GetInstance()->Find_Object(L"Layer_Player", 0);

	XMFLOAT3 playerPos = m_pPlayer->GetPosition();
	XMFLOAT3 Shaft = XMFLOAT3(1, 0, 0);

	XMFLOAT3 dirVector = Vector3::Subtract(playerPos, transform->GetPosition());   // 객체에서 플레이어로 가는 벡터

	dirVector = Vector3::Normalize(dirVector);

	float dotproduct = Vector3::DotProduct(Shaft, dirVector);
	float ShafttLength = Vector3::Length(Shaft);
	float dirVectorLength = Vector3::Length(dirVector);

	float cosCeta = (dotproduct / ShafttLength * dirVectorLength);

	float ceta = acos(cosCeta);

	if (playerPos.z < transform->GetPosition().z)
		ceta = 360.f - ceta * 57.3248f;// +180.0f;
	else
		ceta = ceta * 57.3248f;

	//cout << ceta << endl;
	//cout << ceta << endl;
	float a = Vector3::Length(dirVector);
	XMFLOAT3 Normal = XMFLOAT3(a * cos(ceta), 0, a * sin(ceta));
	Normal = Vector3::Normalize(Normal);



	XMMATRIX world = XMLoadFloat4x4(&(transform->GetWorld()));
	XMMATRIX invWorld = XMMatrixInverse(&XMMatrixDeterminant(world), world);

	BoundingOrientedBox mLocalPlayerBounds;
	m_pPlayer->m_xmOOBB.Transform(mLocalPlayerBounds, invWorld);

	if (mLocalPlayerBounds.Intersects(GetBounds()) != DirectX::DISJOINT)
	{
		/*cout << collisionTagTest << "번째 배럴";
		if (0.0f < ceta && ceta < 45.0f)
		{
			cout << "오른쪽과 충돌 " << endl;
		}
		if (45.0f < ceta && ceta < 135.0f)
		{
			cout << "윗쪽과 충돌 " << endl;
		}
		if (135.0f < ceta && ceta < 225.0f)
		{
			cout << "왼쪽과 충돌 " << endl;
		}
		if (225.0f < ceta && ceta < 360.0f)
		{
			cout << "아랫쪽과 충돌 " << endl;
		}*/
	}
	else
	{

		//cout << "충돌안함" << endl;
	}
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
