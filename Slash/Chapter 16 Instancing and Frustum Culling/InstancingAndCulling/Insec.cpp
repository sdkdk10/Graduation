#include "stdafx.h"
#include "Insec.h"
#include "Define.h"
#include "DynamicMesh.h"
#include "InputDevice.h"
#include "BoundingBox.h"



Insec::Insec(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice, ComPtr<ID3D12DescriptorHeap> &srv, UINT srvSize)
	: CGameObject(d3dDevice, srv, srvSize)
{
}

Insec::~Insec()
{
}

bool Insec::Update(const GameTimer & gt)
{
	/*m_pBoundMesh->SetPosition(GetPosition());
	m_pBoundMesh->Update(gt);*/
	//OnPrepareRender(); 수정필요

	//XMFLOAT3 MoveTest = XMFLOAT3(1, 0, 0);
	//Move(MoveTest);

	CGameObject::Update(gt);
	m_pMesh->Update(gt);
	auto currObjectCB = m_pFrameResource->ObjectCB.get();

	
	XMMATRIX world = XMLoadFloat4x4(&World);
	XMMATRIX texTransform = XMLoadFloat4x4(&TexTransform);

	ObjectConstants objConstants;
	XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(world));
	XMStoreFloat4x4(&objConstants.TexTransform, XMMatrixTranspose(texTransform));
	objConstants.MaterialIndex = Mat->MatCBIndex;

	currObjectCB->CopyData(ObjCBIndex, objConstants);

	// Next FrameResource need to be updated too.


	///////////////////////////////////////////////////

	auto currMaterialCB = m_pFrameResource->MaterialCB.get();

	// Only update the cbuffer data if the constants have changed.  If the cbuffer
	// data changes, it needs to be updated for each FrameResource.
	
	
	XMMATRIX matTransform = XMLoadFloat4x4(&Mat->MatTransform);

	MaterialConstants matConstants;
	matConstants.DiffuseAlbedo = Mat->DiffuseAlbedo;
	matConstants.FresnelR0 = Mat->FresnelR0;
	matConstants.Roughness = Mat->Roughness;
	XMStoreFloat4x4(&matConstants.MatTransform, XMMatrixTranspose(matTransform));

	matConstants.DiffuseMapIndex = Mat->DiffuseSrvHeapIndex;


	currMaterialCB->CopyData(Mat->MatCBIndex, matConstants);

	// Next FrameResource need to be updated too.
	//mat->NumFramesDirty--;
	

	if (KeyBoard_Input(DIK_UP) == CInputDevice::INPUT_PRESS)
	{
		KeyInputTest = 1;
		fAngleTestSum = 0;


		/*fAngleTestSum2 += fAngleTest2;

		if (fAngleTestSum2 < 3.14)
		{

			XMFLOAT3 xmf3RotateAxis = XMFLOAT3(0, -1, 0);
			XMMATRIX mtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&xmf3RotateAxis), fAngleTest2);
			XMStoreFloat4x4(&World, XMLoadFloat4x4(&World) * mtxRotate);


		}*/

		World._43 -= gt.DeltaTime() * 10.f;
		World._41 -= gt.DeltaTime() * 10.f;

	}
	else if (KeyBoard_Input(DIK_DOWN) == CInputDevice::INPUT_PRESS)
	{
		//KeyInputTest = 2;
		KeyInputTest = 1;

	/*	fAngleTestSum2 = 0;
		fAngleTestSum += fAngleTest;

		if (fAngleTestSum < 3.14)
		{

			XMFLOAT3 xmf3RotateAxis = XMFLOAT3(0, 1, 0);
			XMMATRIX mtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&xmf3RotateAxis), fAngleTest);
			XMStoreFloat4x4(&World, XMLoadFloat4x4(&World) * mtxRotate);

			
		}*/

		World._43 += gt.DeltaTime() * 10.f;
		World._41 += gt.DeltaTime() * 10.f;
	}

	else if (KeyBoard_Input(DIK_RIGHT) == CInputDevice::INPUT_PRESS)
	{
		KeyInputTest = 1;
		fAngleTestSum = 0;


		/*fAngleTestSum2 += fAngleTest2;

		if (fAngleTestSum2 < 3.14)
		{

		XMFLOAT3 xmf3RotateAxis = XMFLOAT3(0, -1, 0);
		XMMATRIX mtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&xmf3RotateAxis), fAngleTest2);
		XMStoreFloat4x4(&World, XMLoadFloat4x4(&World) * mtxRotate);


		}*/


		World._41 -= gt.DeltaTime() * 10.f;
		World._43 += gt.DeltaTime() * 10.f;
	}
	else if (KeyBoard_Input(DIK_LEFT) == CInputDevice::INPUT_PRESS)
	{
		//KeyInputTest = 2;
		KeyInputTest = 1;

		/*	fAngleTestSum2 = 0;
		fAngleTestSum += fAngleTest;

		if (fAngleTestSum < 3.14)
		{

		XMFLOAT3 xmf3RotateAxis = XMFLOAT3(0, 1, 0);
		XMMATRIX mtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&xmf3RotateAxis), fAngleTest);
		XMStoreFloat4x4(&World, XMLoadFloat4x4(&World) * mtxRotate);


		}*/

		World._41 += gt.DeltaTime() * 10.f;
		World._43 -= gt.DeltaTime() * 10.f;
	}
	else
	{
		KeyInputTest = 0;
	}

	return true;
}

void Insec::Render(ID3D12GraphicsCommandList * cmdList)
{
	//m_pBoundMesh->Render(cmdList);

	if (KeyInputTest == 0)
		iTest = (int)dynamic_cast<DynamicMesh*>(m_pMesh)->m_fAnimationKeyFrameIndex;
	if (KeyInputTest == 1)
		iTest = (int)dynamic_cast<DynamicMesh*>(m_pMesh)->m_fAnimationKeyFrameIndex_Walk;
	if (KeyInputTest == 2)
		iTest = (int)dynamic_cast<DynamicMesh*>(m_pMesh)->m_fAnimationKeyFrameIndex_Back;
	Render_Head(cmdList);
	Render_Body(cmdList);
	Render_Right(cmdList);
	//Render_Left(cmdList);

}

HRESULT Insec::Initialize()
{
	m_pMesh = new DynamicMesh(m_d3dDevice);
	World._43 = 1.f;


	vector<pair<const string, const string>> path;
	path.push_back(make_pair("Idle", "Models/Warrior/Warrior_Idle.ASE"));
	path.push_back(make_pair("Walk", "Models/Warrior/Warrior_Walk.ASE"));
	path.push_back(make_pair("Back", "Models/Warrior/Warrior_Back.ASE"));


	if (FAILED(m_pMesh->Initialize(path)))
		return E_FAIL;

	Mat = new Material;
	Mat->Name = "InsecMat";
	Mat->MatCBIndex = 0;
	Mat->DiffuseSrvHeapIndex = 0;
	Mat->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	Mat->FresnelR0 = XMFLOAT3(0.05f, 0.05f, 0.05f);
	Mat->Roughness = 0.3f;

	XMStoreFloat4x4(&World, XMMatrixScaling(0.05f, 0.05f, 0.05f)*XMMatrixRotationX(1.7f)*XMMatrixRotationZ(3.14f)*XMMatrixTranslation(0.0f, 0.0f, 0.0f));
	TexTransform = MathHelper::Identity4x4();
	ObjCBIndex = 0;
	
	Geo_Head = dynamic_cast<DynamicMesh*>(m_pMesh)->m_Geometry[0].get();
	PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	Element_Head.IndexCount = Geo_Head->DrawArgs[" \"_head\""].IndexCount;
	Element_Head.StartIndexLocation = Geo_Head->DrawArgs[" \"_head\""].StartIndexLocation;
	Element_Head.BaseVertexLocation = Geo_Head->DrawArgs[" \"_head\""].BaseVertexLocation;

	Geo_Body = dynamic_cast<DynamicMesh*>(m_pMesh)->m_Geometry[1].get();
	PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	Element_Body.IndexCount = Geo_Body->DrawArgs[" \"_body\""].IndexCount;
	Element_Body.StartIndexLocation = Geo_Body->DrawArgs[" \"_body\""].StartIndexLocation;
	Element_Body.BaseVertexLocation = Geo_Body->DrawArgs[" \"_body\""].BaseVertexLocation;


	Geo_Right = dynamic_cast<DynamicMesh*>(m_pMesh)->m_Geometry[2].get();
	PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	Element_Right.IndexCount = Geo_Right->DrawArgs[" \"_rh-01\""].IndexCount;
	Element_Right.StartIndexLocation = Geo_Right->DrawArgs[" \"_rh-01\""].StartIndexLocation;
	Element_Right.BaseVertexLocation = Geo_Right->DrawArgs[" \"_rh-01\""].BaseVertexLocation;


	//Geo_Left = dynamic_cast<DynamicMesh*>(m_pMesh)->m_Geometry[3].get();
	//PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	//Element_Left.IndexCount = Geo_Left->DrawArgs["mage_Left"].IndexCount;
	//Element_Left.StartIndexLocation = Geo_Left->DrawArgs["mage_Left"].StartIndexLocation;
	//Element_Left.BaseVertexLocation = Geo_Left->DrawArgs["mage_Left"].BaseVertexLocation;


	//m_pBoundMesh = dynam


	//m_pBoundMesh = CBoundingBox::Create(m_d3dDevice, mSrvDescriptorHeap, mCbvSrvDescriptorSize, GetPosition(), XMFLOAT3(0.5f, 0.5f, 0.5f));
	return S_OK;
}

void Insec::Free()
{
}

void Insec::Render_Head(ID3D12GraphicsCommandList * cmdList)
{



	UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
	UINT matCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(MaterialConstants));

	auto objectCB = m_pFrameResource->ObjectCB->Resource();
	auto matCB = m_pFrameResource->MaterialCB->Resource();

	cmdList->IASetVertexBuffers(0, 1, &Geo_Head->VertexBufferView());
	cmdList->IASetIndexBuffer(&Geo_Head->IndexBufferView());
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



	//cmdList->DrawIndexedInstanced(Element_Head.IndexCount, 1, Element_Head.StartIndexLocation, Element_Head.BaseVertexLocation , 0);
	//	dynamic_cast<DynamicMesh*>(m_pMesh)->m_vecIndexOffset[0].
	cmdList->DrawIndexedInstanced(Element_Head.IndexCount, 1,
		Element_Head.StartIndexLocation + dynamic_cast<DynamicMesh*>(m_pMesh)->m_vecIndexOffset[0][iTest] + dynamic_cast<DynamicMesh*>(m_pMesh)->m_vecIndexAnimOffset[0][KeyInputTest/*dynamic_cast<DynamicMesh*>(m_pMesh)->iAnimframe*/],
		Element_Head.BaseVertexLocation + dynamic_cast<DynamicMesh*>(m_pMesh)->m_vecVertexOffset[0][iTest] + dynamic_cast<DynamicMesh*>(m_pMesh)->m_vecVertexAnimOffset[0][KeyInputTest/*dynamic_cast<DynamicMesh*>(m_pMesh)->iAnimframe*/],
		0);
}
void Insec::Render_Body(ID3D12GraphicsCommandList * cmdList)
{
	

	UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
	UINT matCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(MaterialConstants));

	auto objectCB = m_pFrameResource->ObjectCB->Resource();
	auto matCB = m_pFrameResource->MaterialCB->Resource();

	cmdList->IASetVertexBuffers(0, 1, &Geo_Body->VertexBufferView());
	cmdList->IASetIndexBuffer(&Geo_Body->IndexBufferView());
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



	cmdList->DrawIndexedInstanced(Element_Body.IndexCount, 1,
		Element_Body.StartIndexLocation + dynamic_cast<DynamicMesh*>(m_pMesh)->m_vecIndexOffset[1][iTest] + dynamic_cast<DynamicMesh*>(m_pMesh)->m_vecIndexAnimOffset[1][KeyInputTest/*dynamic_cast<DynamicMesh*>(m_pMesh)->iAnimframe*/],
		Element_Body.BaseVertexLocation + dynamic_cast<DynamicMesh*>(m_pMesh)->m_vecVertexOffset[1][iTest] + dynamic_cast<DynamicMesh*>(m_pMesh)->m_vecVertexAnimOffset[1][KeyInputTest/*dynamic_cast<DynamicMesh*>(m_pMesh)->iAnimframe*/],
		0);

}


void Insec::Render_Right(ID3D12GraphicsCommandList * cmdList)
{

	UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
	UINT matCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(MaterialConstants));

	auto objectCB = m_pFrameResource->ObjectCB->Resource();
	auto matCB = m_pFrameResource->MaterialCB->Resource();

	cmdList->IASetVertexBuffers(0, 1, &Geo_Right->VertexBufferView());
	cmdList->IASetIndexBuffer(&Geo_Right->IndexBufferView());
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



	cmdList->DrawIndexedInstanced(Element_Right.IndexCount, 1, 
		Element_Right.StartIndexLocation + dynamic_cast<DynamicMesh*>(m_pMesh)->m_vecIndexOffset[2][iTest] + dynamic_cast<DynamicMesh*>(m_pMesh)->m_vecIndexAnimOffset[2][KeyInputTest/*dynamic_cast<DynamicMesh*>(m_pMesh)->iAnimframe*/],
		Element_Right.BaseVertexLocation +  dynamic_cast<DynamicMesh*>(m_pMesh)->m_vecVertexOffset[2][iTest] + dynamic_cast<DynamicMesh*>(m_pMesh)->m_vecVertexAnimOffset[2][KeyInputTest/*dynamic_cast<DynamicMesh*>(m_pMesh)->iAnimframe*/],
		0);
}

Insec * Insec::Create(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice, ComPtr<ID3D12DescriptorHeap>& srv, UINT srvSize)
{
	Insec* pInstance = new Insec(d3dDevice, srv, srvSize);

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX(L"Insec Created Failed");
		Safe_Release(pInstance);
	}

	return pInstance;
}

//void Insec::Render_Left(ID3D12GraphicsCommandList * cmdList)
//{
//	UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
//	UINT matCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(MaterialConstants));
//
//	auto objectCB = m_pFrameResource->ObjectCB->Resource();
//	auto matCB = m_pFrameResource->MaterialCB->Resource();
//
//	cmdList->IASetVertexBuffers(0, 1, &Geo_Left->VertexBufferView());
//	cmdList->IASetIndexBuffer(&Geo_Left->IndexBufferView());
//	cmdList->IASetPrimitiveTopology(PrimitiveType);
//
//	CD3DX12_GPU_DESCRIPTOR_HANDLE tex(mSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
//	tex.Offset(Mat->DiffuseSrvHeapIndex, mCbvSrvDescriptorSize);
//
//	Mat->DiffuseSrvHeapIndex;
//	D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + ObjCBIndex * objCBByteSize;
//	D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress() + Mat->MatCBIndex*matCBByteSize;
//
//	cmdList->SetGraphicsRootConstantBufferView(4, objCBAddress);
//	//cmdList->SetGraphicsRootConstantBufferView(5, matCBAddress);
//	cmdList->SetGraphicsRootShaderResourceView(5, matCBAddress);
//
//	cmdList->SetGraphicsRootDescriptorTable(7, tex);
//
//
//	cmdList->DrawIndexedInstanced(Element_Right.IndexCount, 1, 
//		Element_Right.StartIndexLocation + dynamic_cast<DynamicMesh*>(m_pMesh)->m_vecIndexOffset_Left[iTest] + dynamic_cast<DynamicMesh*>(m_pMesh)->m_vecVertexAnimOffset_Left[KeyInputTest/*dynamic_cast<DynamicMesh*>(m_pMesh)->iAnimframe*/],
//		Element_Right.BaseVertexLocation + dynamic_cast<DynamicMesh*>(m_pMesh)->m_vecVertexOffset_Left[iTest] + dynamic_cast<DynamicMesh*>(m_pMesh)->m_vecVertexAnimOffset_Left[KeyInputTest/*dynamic_cast<DynamicMesh*>(m_pMesh)->iAnimframe*/],
//		0);
//
//
//	//cmdList->DrawIndexedInstanced(Element_Left.IndexCount, 1, Element_Left.StartIndexLocation, Element_Left.BaseVertexLocation, 0);
//}

void Insec::OnPrepareRender()
{
	World._11 = m_xmf3Right.x; 
	World._12 = m_xmf3Right.y;
	World._13 = m_xmf3Right.z;

	World._21 = m_xmf3Up.x;
	World._22 = m_xmf3Up.y;
	World._23 = m_xmf3Up.z;

	World._31 = m_xmf3Look.x;
	World._32 = m_xmf3Look.y;
	World._33 = m_xmf3Look.z;

	World._41 = m_xmf3Position.x;
	World._42 = m_xmf3Position.y;
	World._43 = m_xmf3Position.z;
}

void Insec::SetPosition(float x, float y, float z)
{
	World._41 = x;
	World._42 = y;
	World._43 = z;
}

void Insec::SetPosition(XMFLOAT3 xmf3Position)
{
	Move(XMFLOAT3(xmf3Position.x - m_xmf3Position.x, xmf3Position.y - m_xmf3Position.y, xmf3Position.z - m_xmf3Position.z));
}

void Insec::MoveStrafe(float fDistance)
{
}

void Insec::MoveUp(float fDistance)
{
}

void Insec::MoveForward(float fDistance)
{
}

void Insec::Rotate(float fPitch, float fYaw, float fRoll)
{
}

void Insec::Rotate(XMFLOAT3 * pxmf3Axis, float fAngle)
{
}

void Insec::Animate(const GameTimer & gt)
{
}

void Insec::Move(const XMFLOAT3 & xmf3Shift, bool bVelocity)
{
	XMFLOAT3 PosTest = XMFLOAT3(World._41, World._42, World._43);

	m_xmf3Position = Vector3::Add(PosTest, xmf3Shift);
	SetPosition(m_xmf3Position);
}
