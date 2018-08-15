#include "stdafx.h"
#include "InstancingObject.h"
#include "Define.h"
#include "StaticMesh.h"
#include "Camera.h"
#include "Management.h"
#include "Transform.h"
#include "Component_Manager.h"
#include "Texture_Manager.h"
#include "Layer.h"
#include "TestScene.h"

CInstancingObject* CInstancingObject::m_pAllInstObject[MAXINSTOBJECTID] = { nullptr };
unsigned long CInstancingObject::m_iAllInstObjectIndex = 0;
vector<std::unique_ptr<Material>>			CInstancingObject::mMaterials;


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

void CInstancingObject::SaveSlidingVector(CGameObject * pobj, CGameObject * pCollobj)
{
	// 슬라이딩 벡터는 일단 보류....
	{
		////float ceta = acos(pCollobj->m_xmOOBB.Orientation.w) * 2.0f;


		////XMFLOAT4 Qurtenion = XMFLOAT4(asin(pCollobj->m_xmOOBB.Orientation.x) / sin(ceta / 2), pCollobj->m_xmOOBB.Orientation.y / sin(ceta / 2), pCollobj->m_xmOOBB.Orientation.z / sin(ceta / 2), ceta);
		////cout << "\t" << asin(pCollobj->m_xmOOBB.Orientation.x) / sin(ceta/2) << "\t" << pCollobj->m_xmOOBB.Orientation.y / sin(ceta / 2) << "\t" << pCollobj->m_xmOOBB.Orientation.z / sin(ceta / 2) << "\t" << ceta * 57.3248f << endl;


		////cout << pCollobj->m_xmOOBB.Orientation.x << "\t" << m_xmOOBB.Orientation.y << "\t" << m_xmOOBB.Orientation.z << "\t" << m_xmOOBB.Orientation.w << endl;

		//////////
		////BoundingOrientedBox OOBBTest;
		////OOBBTest = pCollobj->m_xmOOBB;
		////OOBBTest.Transform(OOBBTest, XMLoadFloat4x4(&(pCollobj->GetTransform()->GetWorld())));

		////float ceta = acos(OOBBTest.Orientation.w) * 2.0f;

		////cout << "-----------------------------" << endl;
		////cout << "Player Center : " << pobj->m_xmOOBB.Center.x << "\t" << pobj->m_xmOOBB.Center.y << "\t" << pobj->m_xmOOBB.Center.z << endl;
		////cout << "OOBBTest Center : " << OOBBTest.Center.x << "\t" << OOBBTest.Center.y << "\t" << OOBBTest.Center.z << endl;
		////cout << "OOBBTest Extents : " << OOBBTest.Extents.x << "\t" << OOBBTest.Extents.y << "\t" << OOBBTest.Extents.z << endl;
		////cout << "OOBBTest Qurternion : " << asin(OOBBTest.Orientation.x) / sin(ceta/2.0f) << "\t" << asin(OOBBTest.Orientation.y) / sin(ceta/2.0f) << "\t" << asin(OOBBTest.Orientation.z) / sin(ceta/2.0f) << "\t" << ceta *  57.3248f << endl;
		////cout << "-----------------------------" << endl;

		//////////
		//XMMATRIX world = XMLoadFloat4x4(&(pCollobj->GetTransform()->GetWorld()));


		//XMMATRIX invWorld = XMMatrixInverse(&XMMatrixDeterminant(world), world);

		//// View space to the object's local space.

		//// Transform the camera frustum from view space to the object's local space.

		//BoundingOrientedBox mLocalPlayerBounds;
		//// Transform the camera frustum from view space to the object's local space.
		//pobj->m_xmOOBB.Transform(mLocalPlayerBounds, invWorld);

		//BoundingBox WorldBounds = GetBounds();
		//// Transform the camera frustum from view space to the object's local space.
		//WorldBounds.Transform(WorldBounds, XMLoadFloat4x4(&(pCollobj->GetTransform()->GetWorld())));

		////XMFLOAT3 pCollObjPos2= pCollobj->m_pTransCom->m_f3Position;
		//XMFLOAT3 pCollObjPos = pCollobj->GetTransform()->GetPosition();


		//float MinX = pCollObjPos.x - WorldBounds.Extents.x;
		//float MaxX = pCollObjPos.x + WorldBounds.Extents.x;

		//float MinZ = pCollObjPos.z - WorldBounds.Extents.z;
		//float MaxZ = pCollObjPos.z + WorldBounds.Extents.z;

		////cout << mLocalPlayerBounds.Orientation.x << "\t" << mLocalPlayerBounds.Orientation.y << "\t" << mLocalPlayerBounds.Orientation.z << "\t" << mLocalPlayerBounds.Orientation.w << endl;
		//if (mLocalPlayerBounds.Contains(GetBounds()))
		//{
		//	//cout << "인스턴싱 충돌 " << endl;
		//
		//	//else // 안으로 파들어져 왔을 경우
		//	//{
		//		/*cout << "-----------------------------------" << endl;
		//		cout << "Player Pos : " << pobj->GetPosition().x << "\t" << pobj->GetPosition().y << "\t" << pobj->GetPosition().z << endl;
		//		cout << "Instancing Object Pos : " << pCollObjPos.x << "\t" << pCollObjPos.y << "\t" << pCollObjPos.z << endl;
		//		cout << "Instancing Extents : " << WorldBounds.Extents.x << "\t" << WorldBounds.Extents.y << "\t" << WorldBounds.Extents.z << endl;
		//		cout << "-----------------------------------" << endl;*/

		//		//m_MovingRefletVector = XMFLOAT3(0, 0, -1);
		//		//cout << "-------------------------" << endl;



		//	//cout << "거미" << "\t" << asin(m_pSpider->m_xmOOBB.Orientation.x) / sin(ceta / 2) << "\t" << m_pSpider->m_xmOOBB.Orientation.y / sin(ceta / 2) << "\t" << m_pSpider->m_xmOOBB.Orientation.z / sin(ceta / 2) << "\t" << ceta * 57.3248f << endl;


		//		//cout << "인스턴싱 안으로 파들어와짐" << endl;
		//		//cout << planeCollision << endl;

		//		//cout << "-------------------------" << endl;
		//			switch (planeCollision)
		//			{
		//			case 0:
		//				break;
		//			case 1:
		//				//cout << "오른쪽면 충돌" << endl;
		//				pobj->m_MovingRefletVector = XMFLOAT3(1, 0, 0);
		//				break;
		//			case 2:
		//				//cout << "왼쪽면 충돌" << endl;

		//				pobj->m_MovingRefletVector = XMFLOAT3(-1, 0, 0);

		//				break;
		//			case 3:
		//				//cout << "아래쪽면 충돌" << endl;

		//				pobj->m_MovingRefletVector = XMFLOAT3(0, 0, -1);

		//				break;
		//			case 4:
		//				//cout << "위쪽면 충돌" << endl;

		//				pobj->m_MovingRefletVector = XMFLOAT3(0, 0, 1);

		//				break;
		//			}


		//	

		//	//}

		//	//cout << i << "번째 인스턴싱 오브젝트랑 충돌" << endl;
		//}

	}

	////////
	XMMATRIX world = XMLoadFloat4x4(&(pCollobj->GetTransform()->GetWorld()));


	XMMATRIX invWorld = XMMatrixInverse(&XMMatrixDeterminant(world), world);

	// View space to the object's local space.

	// Transform the camera frustum from view space to the object's local space.

	BoundingOrientedBox mLocalPlayerBounds;
	// Transform the camera frustum from view space to the object's local space.
	pobj->m_xmOOBB.Transform(mLocalPlayerBounds, invWorld);

	BoundingBox WorldBounds = GetBounds();
	// Transform the camera frustum from view space to the object's local space.
	WorldBounds.Transform(WorldBounds, XMLoadFloat4x4(&(pCollobj->GetTransform()->GetWorld())));


	//XMFLOAT3 pCollObjPos2= pCollobj->m_pTransCom->m_f3Position;
	XMFLOAT3 Center = pCollobj->GetTransform()->GetPosition();
	XMFLOAT3 Player = pobj->GetPosition();
	XMFLOAT3 look = XMFLOAT3(pCollobj->GetTransform()->GetWorld()._21, pCollobj->GetTransform()->GetWorld()._22, pCollobj->GetTransform()->GetWorld()._23);
	XMFLOAT3 dirVector = Vector3::Subtract(Player, Center);   // 충돌 객체에서 플레이어로 가는 벡터

	float tanceta = dirVector.z / dirVector.x;
	float ceta = atan(tanceta) * 57.3248f;

	float extenttanceta = WorldBounds.Extents.z / WorldBounds.Extents.x;


	float extentceta = atan(extenttanceta) * 57.3248f;


	look = Vector3::Normalize(look);
	dirVector = Vector3::Normalize(dirVector);
	//cout << "Coll obj Look" << "\t" << look.x << "\t" << look.y << "\t" << look.z << endl;
	//cout << "dirVector : " << "\t" << dirVector.x << "\t" << dirVector.y << "\t" << dirVector.z << endl;
	//cout << "Ceta : " << ceta << endl;
	//cout << "ExtentCeta : " << extentceta << endl;

	if (Player.x > Center.x && 0 < ceta && ceta< extentceta) // 1
	{
		//cout << " 오른쪽 충돌" << endl;
		pobj->m_MovingRefletVector = XMFLOAT3(1, 0, 0);


	}
	if (Player.z > Center.z && extentceta <ceta && ceta < 90) // 2
	{
		//cout << "윗쪽 충돌" << endl;
		pobj->m_MovingRefletVector = XMFLOAT3(0, 0, 1);
	}
	if (Player.z > Center.z && -90 < ceta && ceta < -extentceta) // 3
	{
		//cout << "윗쪽 충돌" << endl;
		//pobj->m_MovingRefletVector = XMFLOAT3(-1, 0, 0);
		pobj->m_MovingRefletVector = XMFLOAT3(0, 0, 1);

	}
	if (Player.x < Center.x && -extentceta < ceta && ceta < 0)// 4
	{
		//cout << "왼쪽 충돌" << endl;
		//pobj->m_MovingRefletVector = XMFLOAT3(-1, 0, 0);
		pobj->m_MovingRefletVector = XMFLOAT3(-1, 0, 0);

	}
	if (Player.x < Center.x && 0 < ceta && ceta < extentceta) // 5
	{
		//cout << "왼쪽 충돌" << endl;
		//pobj->m_MovingRefletVector = XMFLOAT3(-1, 0, 0);
		pobj->m_MovingRefletVector = XMFLOAT3(-1, 0, 0);

	}
	if (Player.z < Center.z && extentceta < ceta && ceta < 90) // 6
	{
		//cout << "아래쪽 충돌" << endl;
		//pobj->m_MovingRefletVector = XMFLOAT3(-1, 0, 0);
		pobj->m_MovingRefletVector = XMFLOAT3(0, 0,-1);

	}
	if (Player.z < Center.z && -90 < ceta && ceta < -extentceta) // 7
	{
		//cout << "아래쪽 충돌" << endl;
		//pobj->m_MovingRefletVector = XMFLOAT3(-1, 0, 0);
		pobj->m_MovingRefletVector = XMFLOAT3(0, 0, -1);

	}
	if (Player.x > Center.x && -extentceta < ceta && ceta < 0) // 8
	{
		//cout << "오른쪽 충돌" << endl;
		//pobj->m_MovingRefletVector = XMFLOAT3(-1, 0, 0);
		pobj->m_MovingRefletVector = XMFLOAT3(1, 0, 0);

	}

	//cout << "ExtentCeta1 : " << extentceta1 << endl;
	//cout << "ExtentCeta2: " << extentceta2 << endl;
	//cout << "ExtentCeta3 : " << extentceta3 << endl;

	//if (ceta < extentceta)
	//{
	//	cout << "오른쪽 면 충돌" << endl;
	//}
	//if( extentceta < ceta < )
	/*float MinX = pCollObjPos.x - WorldBounds.Extents.x;
	float MaxX = pCollObjPos.x + WorldBounds.Extents.x;

	float MinZ = pCollObjPos.z - WorldBounds.Extents.z;
	float MaxZ = pCollObjPos.z + WorldBounds.Extents.z;*/

	//pobj->m_MovingRefletVector = XMFLOAT3(look.x, look.y, look.z);

	//if (pobj->GetPosition().x >  MaxX && pobj->GetPosition().z >  MinZ &&  pobj->GetPosition().z <  MaxZ)
	//{
	//	//cout << " 오른쪽 면  충돌" << endl;
	//	planeCollision = 1;
	//}

	//if (pobj->GetPosition().x <  MinX && pobj->GetPosition().z >  MinZ &&  pobj->GetPosition().z <  MaxZ)
	//{
	//	//cout << " 왼쪽 면  충돌" << endl;

	//	planeCollision = 2;
	//}

	//if (pobj->GetPosition().z <  MinZ && pobj->GetPosition().x >  MinX &&  pobj->GetPosition().x <  MaxX)
	//{
	//	//cout << "아랫면 충돌" << endl;
	//	planeCollision = 3;
	//}

	//if (pobj->GetPosition().z >  MaxZ && pobj->GetPosition().x >  MinX &&  pobj->GetPosition().x <  MaxX)
	//{
	//	//cout << "윗면 충돌" << endl;

	//	planeCollision = 4;
	//}
	//else
	//{
	//	/*cout << "Player Pos : " << pobj->GetPosition().x << "\t" << pobj->GetPosition().y << "\t" << pobj->GetPosition().z << endl;
	//	cout << "Min X  : " << MinX << "\t" << "Max X : " << MaxX << endl;
	//	cout << "Min Z  : " << MinZ << "\t" << "Max Z : " << MaxZ << endl;*/

	//	//cout << " 일단 인스턴싱 충돌" << endl;
	//	//cout << pobj->m_MovingRefletVector.x << "\t" << pobj->m_MovingRefletVector.y << "\t" << pobj->m_MovingRefletVector.z << endl;
	//	switch (planeCollision)
	//	{
	//	case 0:
	//		break;
	//	case 1:
	//		//cout << "오른쪽면 충돌" << endl;
	//		pobj->m_MovingRefletVector = XMFLOAT3(1, 0, 0);
	//		break;
	//	case 2:
	//		//cout << "왼쪽면 충돌" << endl;

	//		pobj->m_MovingRefletVector = XMFLOAT3(-1, 0, 0);

	//		break;
	//	case 3:
	//		//cout << "아래쪽면 충돌" << endl;

	//		pobj->m_MovingRefletVector = XMFLOAT3(0, 0, -1);

	//		break;
	//	case 4:
	//		//cout << "위쪽면 충돌" << endl;

	//		pobj->m_MovingRefletVector = XMFLOAT3(0, 0, 1);

	//		break;
	//	}
	//}
	
	//cout << mLocalPlayerBounds.Orientation.x << "\t" << mLocalPlayerBounds.Orientation.y << "\t" << mLocalPlayerBounds.Orientation.z << "\t" << mLocalPlayerBounds.Orientation.w << endl;
	{
		//if (mLocalPlayerBounds.Contains(GetBounds()))
		//{
		//	//cout << "인스턴싱 충돌 " << endl;

		//	//else // 안으로 파들어져 왔을 경우
		//	//{
		//	/*cout << "-----------------------------------" << endl;
		//	cout << "Player Pos : " << pobj->GetPosition().x << "\t" << pobj->GetPosition().y << "\t" << pobj->GetPosition().z << endl;
		//	cout << "Instancing Object Pos : " << pCollObjPos.x << "\t" << pCollObjPos.y << "\t" << pCollObjPos.z << endl;
		//	cout << "Instancing Extents : " << WorldBounds.Extents.x << "\t" << WorldBounds.Extents.y << "\t" << WorldBounds.Extents.z << endl;
		//	cout << "-----------------------------------" << endl;*/

		//	//m_MovingRefletVector = XMFLOAT3(0, 0, -1);
		//	//cout << "-------------------------" << endl;



		//	//cout << "거미" << "\t" << asin(m_pSpider->m_xmOOBB.Orientation.x) / sin(ceta / 2) << "\t" << m_pSpider->m_xmOOBB.Orientation.y / sin(ceta / 2) << "\t" << m_pSpider->m_xmOOBB.Orientation.z / sin(ceta / 2) << "\t" << ceta * 57.3248f << endl;


		//	//cout << "인스턴싱 안으로 파들어와짐" << endl;
		//	//cout << planeCollision << endl;

		//	//cout << "-------------------------" << endl;
		//	//switch (planeCollision)
		//	//{
		//	//case 0:
		//	//	break;
		//	//case 1:
		//	//	//cout << "오른쪽면 충돌" << endl;
		//	//	pobj->m_MovingRefletVector = XMFLOAT3(1, 0, 0);
		//	//	break;
		//	//case 2:
		//	//	//cout << "왼쪽면 충돌" << endl;

		//	//	pobj->m_MovingRefletVector = XMFLOAT3(-1, 0, 0);

		//	//	break;
		//	//case 3:
		//	//	//cout << "아래쪽면 충돌" << endl;

		//	//	pobj->m_MovingRefletVector = XMFLOAT3(0, 0, -1);

		//	//	break;
		//	//case 4:
		//	//	//cout << "위쪽면 충돌" << endl;

		//	//	pobj->m_MovingRefletVector = XMFLOAT3(0, 0, 1);

		//	//	break;
		//	//}




		//	//}

		//	//cout << i << "번째 인스턴싱 오브젝트랑 충돌" << endl;
		//}
	}
	


	XMFLOAT3 moveVec = Vector3::Normalize(Vector3::Subtract(pCollobj->GetPosition(), pobj->GetPosition()));
	//moveVec.x = moveVec.x / 1.25f;
	//moveVec.y = moveVec.z / 1.25f;
	//moveVec.z = moveVec.y / 1.25f;

	//pobj->Move(moveVec, true);

}

HRESULT CInstancingObject::Initialize()
{

	m_pMesh = dynamic_cast<StaticMesh*>(CComponent_Manager::GetInstance()->Clone_Component(m_pwstrMeshName));
	if (nullptr == m_pMesh)
		return E_FAIL;
	
	string TexName = m_pMesh->Get_TexName();

	auto tex = CTexture_Manager::GetInstance()->Find_Texture(TexName, CTexture_Manager::TEX_INST_2D);
	if (nullptr == tex)
		return E_FAIL;


	/*
	auto bricks0 = std::make_unique<Material>();
	bricks0->Name = "bricks0";
	bricks0->MatCBIndex = 0;
	bricks0->DiffuseSrvHeapIndex = tex->Num;
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
	*/
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
		//XMStoreFloat4x4(&vecInstances[i].TexTransform, XMMatrixScaling(2.0f, 2.0f, 1.0f));
		vecInstances[i].TexTransform = TexTransform;
		//vecInstances[index].MaterialIndex = index % (mMaterials.size() - 1);
		//m_iMyInstObject;

		auto material = std::make_unique<Material>();
		material->Name = "material";
		material->MatCBIndex = m_iMyInstObject + i;
		material->DiffuseSrvHeapIndex = tex->Num;
		material->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		material->FresnelR0 = XMFLOAT3(0.05f, 0.05f, 0.05f);
		material->Roughness = 0.5f;

		mMaterials.push_back(std::move(material));


		vecInstances[i].MaterialIndex = m_iMyInstObject + i;
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
	auto& instanceData = vecInstances;

	int visibleInstanceCount = 0;


	int iTest = m_iMyInstObject;

	

	for (UINT i = 0; i < (UINT)m_iSize; ++i)
	{
	
		Animate(gt, m_vecTransCom[i]);
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
			//cout << " 보인당" << endl;
			auto currInstanceBuffer = m_pFrameResource->InstanceBuffer.get();


			InstanceData data;
			XMStoreFloat4x4(&data.World, XMMatrixTranspose(world));
			XMStoreFloat4x4(&data.TexTransform, XMMatrixTranspose(texTransform));
			data.MaterialIndex = instanceData[i].MaterialIndex;

			// Write the instance data to structured buffer for the visible objects. 보여? ㅎㅇ 
			currInstanceBuffer->CopyData(iTest++, data);
			visibleInstanceCount++;
		}
		else
		{
			//cout << " 안보인당 " << endl;
		}
	}

	//InstanceCount = visibleInstanceCount;

	InstanceCount = visibleInstanceCount;
	
	//cout << m_pwstrMeshName << " : " << endl;
	//cout << InstanceCount << endl;

	//cout << InstanceCount << endl;


	/* Material */
	auto currMaterialBuffer = m_pFrameResource->MaterialBuffer.get();


	for (int i = 0; i < m_iSize; ++i)
	{
		Material* mat = mMaterials[m_iMyInstObject + i].get();

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

	//for (auto& e : mMaterials)
	//{
	//	// Only update the cbuffer data if the constants have changed.  If the cbuffer
	//	// data changes, it needs to be updated for each FrameResource.
	//	Material* mat = e.get();
	//	if (mat->NumFramesDirty > 0)
	//	{
	//		XMMATRIX matTransform = XMLoadFloat4x4(&mat->MatTransform);

	//		MaterialData matData;
	//		matData.DiffuseAlbedo = mat->DiffuseAlbedo;
	//		matData.FresnelR0 = mat->FresnelR0;
	//		matData.Roughness = mat->Roughness;
	//		XMStoreFloat4x4(&matData.MatTransform, XMMatrixTranspose(matTransform));
	//		matData.DiffuseMapIndex = mat->DiffuseSrvHeapIndex;

	//		currMaterialBuffer->CopyData(mat->MatCBIndex, matData);

	//		// Next FrameResource need to be updated too.
	//		mat->NumFramesDirty--;
	//	}
	//}

	CManagement::GetInstance()->GetRenderer()->Add_RenderGroup(m_eMyRenderType, this);
	return true;
}

void CInstancingObject::Render(ID3D12GraphicsCommandList * cmdList)
{

	//RenderBounds(cmdList);
	
	cmdList->SetGraphicsRootDescriptorTable(3, mSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
	//UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
	cmdList->IASetVertexBuffers(0, 1, &Geo->VertexBufferView());
	cmdList->IASetIndexBuffer(&Geo->IndexBufferView());
	cmdList->IASetPrimitiveTopology(PrimitiveType);

	// Set the instance buffer to use for this render-item.  For structured buffers, we can bypass 
	// the heap and set as a root descriptor.

	auto instanceBuffer = m_pFrameResource->InstanceBuffer->Resource();
	D3D12_GPU_VIRTUAL_ADDRESS Address = instanceBuffer->GetGPUVirtualAddress() + m_iMyInstObject * sizeof(InstanceData);
	cmdList->SetGraphicsRootShaderResourceView(0, Address);
	
	//InstanceCount = vecInstances.size();
	cmdList->DrawIndexedInstanced(IndexCount, InstanceCount, StartIndexLocation, BaseVertexLocation, 0);

	

}

void CInstancingObject::Animate(const GameTimer & gt)
{


}

void CInstancingObject::Animate(const GameTimer & gt, CTransform * transform)
{
	

	
	auto m_pPlayer = CManagement::GetInstance()->Find_Object(L"Layer_Player", 0); //이거 한번만 하게 바꿀껏


	m_pCamera = CManagement::GetInstance()->Get_MainCam();
	XMMATRIX view = m_pCamera->GetView();
	XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);

	XMMATRIX world = XMLoadFloat4x4(&transform->GetWorld());


	XMMATRIX invWorld = XMMatrixInverse(&XMMatrixDeterminant(world), world);

	// View space to the object's local space.

	// Transform the camera frustum from view space to the object's local space.

	BoundingOrientedBox mLocalPlayerBounds;
	// Transform the camera frustum from view space to the object's local space.
	m_pPlayer->m_xmOOBB.Transform(mLocalPlayerBounds, invWorld);

	BoundingBox WorldBounds = GetBounds();
	// Transform the camera frustum from view space to the object's local space.
	WorldBounds.Transform(WorldBounds, XMLoadFloat4x4(&(transform->GetWorld())));

	CTestScene* pScene = dynamic_cast<CTestScene*>(CManagement::GetInstance()->Get_CurScene());
	if (pScene == nullptr)
		return;
	pScene->GetObjectCount();

	
	if (mLocalPlayerBounds.Contains(GetBounds()) != DirectX::DISJOINT)
	{


		m_pPlayer->m_pCollider = this;
		m_pPlayer->m_pCollider->m_pTransCom = transform;


	}

	m_xmOOBBTransformed.Transform(m_xmOOBB, XMLoadFloat4x4(&(transform->GetWorld())));
	XMStoreFloat4(&m_xmOOBBTransformed.Orientation, XMQuaternionNormalize(XMLoadFloat4(&m_xmOOBBTransformed.Orientation)));


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
