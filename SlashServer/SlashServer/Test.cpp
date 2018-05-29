#include "stdafx.h"
#include "Player.h"
#include "Define.h"
#include "DynamicMesh.h"
#include "InputDevice.h"
#include "Network.h"
#include "Component_Manager.h"
#include "Texture_Manager.h"

Player::Player(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice, ComPtr<ID3D12DescriptorHeap> &srv, UINT srvSize)
	: CGameObject(d3dDevice, srv, srvSize)
{
	preKeyInputTime = 0;
	curKeyInputTime = 0;

}

Player::~Player()
{
	delete m_pMesh;

	delete Mat;

	//여기
}


void Player::Animate(const GameTimer & gt)
{
	AnimStateMachine.AnimationStateUpdate(gt); //애니메이션 상태 설정해주는 함수
											   /*cout << endl << endl;

											   cout << World._11 << "\t" << World._12 << "\t" << World._13 << endl;
											   cout << World._21 << "\t" << World._22 << "\t" << World._23 << endl;
											   cout << World._31 << "\t" << World._32 << "\t" << World._33 << endl;
											   cout << World._41 << "\t" << World._42 << "\t" << World._43 << endl;*/

											   //cout << World._41 << "\t" << World._42 << "\t" << World._43 << endl;
	DWORD dwDirection = 0;

	if (KeyBoard_Input(DIK_Q) == CInputDevice::INPUT_PRESS)
	{
		World._42 += 0.1f;
	}
	if (KeyBoard_Input(DIK_E) == CInputDevice::INPUT_PRESS)
	{
		World._42 -= 0.1f;
	}
	if (KeyBoard_Input(DIK_UP) == CInputDevice::INPUT_PRESS)
	{
		AnimStateMachine.SetAnimState(AnimStateMachine.WalkState);
		//KeyInputTest = 1;
		dwDirection |= DIR_FORWARD;

	}
	if (KeyBoard_Input(DIK_DOWN) == CInputDevice::INPUT_PRESS)
	{
		AnimStateMachine.SetAnimState(AnimStateMachine.WalkState);

		//KeyInputTest = 1;
		dwDirection |= DIR_BACKWARD;

	}
	if (KeyBoard_Input(DIK_RIGHT) == CInputDevice::INPUT_PRESS)
	{
		AnimStateMachine.SetAnimState(AnimStateMachine.WalkState);

		//KeyInputTest = 1;
		dwDirection |= DIR_RIGHT;

	}
	if (KeyBoard_Input(DIK_LEFT) == CInputDevice::INPUT_PRESS)
	{
		AnimStateMachine.SetAnimState(AnimStateMachine.WalkState);

		//KeyInputTest = 1;
		dwDirection |= DIR_LEFT;
	}


	if (dwDirection != 0)
	{
		//CNetwork::GetInstance()->SendPacket(dwDirection);
		Move(dwDirection, m_fMoveSpeed * gt.DeltaTime(), true);
		curKeyInputTime = gt.TotalTime();
		if (curKeyInputTime - preKeyInputTime > CS_SEND_PACKET_DELAY)
		{
			//CNetwork::GetInstance()->SendDirKeyPacket(dwDirection);
			preKeyInputTime = gt.TotalTime();
		}

		//Move(dwDirection, 15.0f * gt.DeltaTime(), true);
	}

	if (KeyBoard_Input(DIK_SPACE) == CInputDevice::INPUT_DOWN)
	{
		//KeyInputTest = 2;
		if (AnimStateMachine.GetAnimState() != AnimStateMachine.Attack1State)
		{
			AnimStateMachine.SetAnimState(AnimStateMachine.Attack1State);


			//KeyInputTest = 2;
		}
		else
		{
			AnimStateMachine.SetAnimState(AnimStateMachine.Attack3State);


			//KeyInputTest = 4;//3;
			bAttackMotionTest = true;
		}

		if (bAttackMotionTest == false)
		{
			if (AnimStateMachine.GetAnimState() == AnimStateMachine.Attack2State)
			{

			}
		}


	}
	if (KeyBoard_Input(DIK_P) == CInputDevice::INPUT_DOWN)
	{
		//KeyInputTest = 2;
	}
	if (KeyBoard_Input(DIK_O) == CInputDevice::INPUT_DOWN)
	{
		//KeyInputTest = 2;
	}

	if (KeyBoard_Input(DIK_R) == CInputDevice::INPUT_PRESS)
	{
		World._41 -= World._21; //모델의 LookVector가 반대로 되있음
		World._42 -= World._22;
		World._43 -= World._23;
	}

	if (KeyBoard_Input(DIK_L) == CInputDevice::INPUT_PRESS)
	{

		SetHp(GetHp() - 0.1f);
	}
	if (KeyBoard_Input(DIK_K) == CInputDevice::INPUT_PRESS)
	{

		SetHp(GetHp() + 0.1f);
	}

	if (CInputDevice::GetInstance()->AnyKeyInput())
	{

		if (!AnimStateMachine.bTimerAttack1 &&
			!AnimStateMachine.bTimerAttack2 &&
			!AnimStateMachine.bTimerAttack3 /*&&
											!pTestMesh->bTimerTestWalk*/
			)
		{

			AnimStateMachine.SetAnimState(AnimStateMachine.IdleState);

			//   KeyInputTest = 0;

		}
	}

	m_xmOOBBTransformed.Transform(m_xmOOBB, XMLoadFloat4x4(&(GetWorld())));
	XMStoreFloat4(&m_xmOOBBTransformed.Orientation, XMQuaternionNormalize(XMLoadFloat4(&m_xmOOBBTransformed.Orientation)));

}

bool Player::Update(const GameTimer & gt)
{

	CGameObject::Update(gt);

	Animate(gt);


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


	return true;
}

void Player::Render(ID3D12GraphicsCommandList * cmdList)
{
	AnimStateMachine.SetTimerTrueFalse(); //어떤 애니메이션을 동작 시켜주는 지 


	Render_Head(cmdList);
	Render_Body(cmdList);
	Render_Right(cmdList);
	//Render_Left(cmdList);

}
HRESULT Player::Initialize()
{
	m_pMesh = dynamic_cast<DynamicMesh*>(CComponent_Manager::GetInstance()->Clone_Component(L"Com_Mesh_Warrior"));

	if (nullptr == m_pMesh)
		return E_FAIL;

	AnimStateMachine.vecAnimFrame = &(dynamic_cast<DynamicMesh*>(m_pMesh)->vecAnimFrame);

	Texture* tex = CTexture_Manager::GetInstance()->Find_Texture("VillagerTex", CTexture_Manager::TEX_DEFAULT_2D);         // 이런식으로 가져옴
	if (tex == nullptr)
		return E_FAIL;

	Mat = new Material;
	Mat->Name = "InsecMat";
	Mat->MatCBIndex = 0;
	Mat->DiffuseSrvHeapIndex = tex->Num;         // 텍스쳐 힙에 저장ㄷ외나 수아아ㅇ아ㅏ 아하 지희야그럼 이거이렇게바꿔야함 
	Mat->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	Mat->FresnelR0 = XMFLOAT3(0.05f, 0.05f, 0.05f);
	Mat->Roughness = 0.3f;

	XMStoreFloat4x4(&World, XMMatrixScaling(0.05f, 0.05f, 0.05f)*XMMatrixRotationX(1.7f)*XMMatrixRotationZ(3.14f)*XMMatrixTranslation(0.0f, 0.0f, 0.0f));
	TexTransform = MathHelper::Identity4x4();
	ObjCBIndex = m_iMyObjectID;

	Geo_Head = dynamic_cast<DynamicMesh*>(m_pMesh)->m_Geometry[0].get();
	PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	Element_Head.IndexCount = Geo_Head->DrawArgs[" \"_head\""].IndexCount;
	Element_Head.StartIndexLocation = Geo_Head->DrawArgs[" \"_head\""].StartIndexLocation;
	Element_Head.BaseVertexLocation = Geo_Head->DrawArgs[" \"_head\""].BaseVertexLocation;
	Bounds = Geo_Head->DrawArgs[" \"_head\""].Bounds;


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




	SetOOBB(XMFLOAT3(Bounds.Center.x * 0.05f, Bounds.Center.y * 0.05f, Bounds.Center.z * 0.05f), XMFLOAT3(Bounds.Extents.x * 0.05f, Bounds.Extents.y * 0.05f, Bounds.Extents.z * 0.05f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));

	//Geo_Left = dynamic_cast<DynamicMesh*>(m_pMesh)->m_Geometry[3].get();
	//PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	//Element_Left.IndexCount = Geo_Left->DrawArgs["mage_Left"].IndexCount;
	//Element_Left.StartIndexLocation = Geo_Left->DrawArgs["mage_Left"].StartIndexLocation;
	//Element_Left.BaseVertexLocation = Geo_Left->DrawArgs["mage_Left"].BaseVertexLocation;

	//m_pBoundMesh = dynam


	//m_pBoundMesh = CBoundingBox::Create(m_d3dDevice, mSrvDescriptorHeap, mCbvSrvDescriptorSize, GetPosition(), XMFLOAT3(0.5f, 0.5f, 0.5f));
	return S_OK;
}
void Player::Free()
{
	CGameObject::Free();
}
void Player::Render_Head(ID3D12GraphicsCommandList * cmdList)
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

	auto indexcnt = dynamic_cast<DynamicMesh*>(m_pMesh)->m_vecIndexOffset[0][1];

	int iTest = AnimStateMachine.GetCurAnimFrame();
	int KeyInputTest = AnimStateMachine.GetAnimState();


	//cmdList->DrawIndexedInstanced(Element_Head.IndexCount, 1, Element_Head.StartIndexLocation, Element_Head.BaseVertexLocation , 0);
	//   dynamic_cast<DynamicMesh*>(m_pMesh)->m_vecIndexOffset[0].
	cmdList->DrawIndexedInstanced(indexcnt, 1,
		dynamic_cast<DynamicMesh*>(m_pMesh)->m_vecIndexOffset[0][iTest] + dynamic_cast<DynamicMesh*>(m_pMesh)->m_vecIndexAnimOffset[0][KeyInputTest/*dynamic_cast<DynamicMesh*>(m_pMesh)->iAnimframe*/],
		dynamic_cast<DynamicMesh*>(m_pMesh)->m_vecVertexOffset[0][iTest] + dynamic_cast<DynamicMesh*>(m_pMesh)->m_vecVertexAnimOffset[0][KeyInputTest/*dynamic_cast<DynamicMesh*>(m_pMesh)->iAnimframe*/],
		0);
}
void Player::Render_Body(ID3D12GraphicsCommandList * cmdList)
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

	auto indexcnt = dynamic_cast<DynamicMesh*>(m_pMesh)->m_vecIndexOffset[1][1];

	int iTest = AnimStateMachine.GetCurAnimFrame();
	int KeyInputTest = AnimStateMachine.GetAnimState();

	cmdList->DrawIndexedInstanced(indexcnt, 1,
		Element_Body.StartIndexLocation + dynamic_cast<DynamicMesh*>(m_pMesh)->m_vecIndexOffset[1][iTest] + dynamic_cast<DynamicMesh*>(m_pMesh)->m_vecIndexAnimOffset[1][KeyInputTest/*dynamic_cast<DynamicMesh*>(m_pMesh)->iAnimframe*/],
		Element_Body.BaseVertexLocation + dynamic_cast<DynamicMesh*>(m_pMesh)->m_vecVertexOffset[1][iTest] + dynamic_cast<DynamicMesh*>(m_pMesh)->m_vecVertexAnimOffset[1][KeyInputTest/*dynamic_cast<DynamicMesh*>(m_pMesh)->iAnimframe*/],
		0);

}
void Player::Render_Right(ID3D12GraphicsCommandList * cmdList)
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

	auto indexcnt = dynamic_cast<DynamicMesh*>(m_pMesh)->m_vecIndexOffset[2][1];

	int iTest = AnimStateMachine.GetCurAnimFrame();
	int KeyInputTest = AnimStateMachine.GetAnimState();


	cmdList->DrawIndexedInstanced(indexcnt, 1,
		Element_Right.StartIndexLocation + dynamic_cast<DynamicMesh*>(m_pMesh)->m_vecIndexOffset[2][iTest] + dynamic_cast<DynamicMesh*>(m_pMesh)->m_vecIndexAnimOffset[2][KeyInputTest/*dynamic_cast<DynamicMesh*>(m_pMesh)->iAnimframe*/],
		Element_Right.BaseVertexLocation + dynamic_cast<DynamicMesh*>(m_pMesh)->m_vecVertexOffset[2][iTest] + dynamic_cast<DynamicMesh*>(m_pMesh)->m_vecVertexAnimOffset[2][KeyInputTest/*dynamic_cast<DynamicMesh*>(m_pMesh)->iAnimframe*/],
		0);
}
Player * Player::Create(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice, ComPtr<ID3D12DescriptorHeap>& srv, UINT srvSize)
{
	Player* pInstance = new Player(d3dDevice, srv, srvSize);

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX(L"Player Created Failed");
		Safe_Release(pInstance);
	}

	return pInstance;
}

//void Player::Render_Left(ID3D12GraphicsCommandList * cmdList)
//{
//   UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
//   UINT matCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(MaterialConstants));
//
//   auto objectCB = m_pFrameResource->ObjectCB->Resource();
//   auto matCB = m_pFrameResource->MaterialCB->Resource();
//
//   cmdList->IASetVertexBuffers(0, 1, &Geo_Left->VertexBufferView());
//   cmdList->IASetIndexBuffer(&Geo_Left->IndexBufferView());
//   cmdList->IASetPrimitiveTopology(PrimitiveType);
//
//   CD3DX12_GPU_DESCRIPTOR_HANDLE tex(mSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
//   tex.Offset(Mat->DiffuseSrvHeapIndex, mCbvSrvDescriptorSize);
//
//   Mat->DiffuseSrvHeapIndex;
//   D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + ObjCBIndex * objCBByteSize;
//   D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress() + Mat->MatCBIndex*matCBByteSize;
//
//   cmdList->SetGraphicsRootConstantBufferView(4, objCBAddress);
//   //cmdList->SetGraphicsRootConstantBufferView(5, matCBAddress);
//   cmdList->SetGraphicsRootShaderResourceView(5, matCBAddress);
//
//   cmdList->SetGraphicsRootDescriptorTable(7, tex);
//
//
//   cmdList->DrawIndexedInstanced(Element_Right.IndexCount, 1, 
//      Element_Right.StartIndexLocation + dynamic_cast<DynamicMesh*>(m_pMesh)->m_vecIndexOffset_Left[iTest] + dynamic_cast<DynamicMesh*>(m_pMesh)->m_vecVertexAnimOffset_Left[KeyInputTest/*dynamic_cast<DynamicMesh*>(m_pMesh)->iAnimframe*/],
//      Element_Right.BaseVertexLocation + dynamic_cast<DynamicMesh*>(m_pMesh)->m_vecVertexOffset_Left[iTest] + dynamic_cast<DynamicMesh*>(m_pMesh)->m_vecVertexAnimOffset_Left[KeyInputTest/*dynamic_cast<DynamicMesh*>(m_pMesh)->iAnimframe*/],
//      0);
//
//
//   //cmdList->DrawIndexedInstanced(Element_Left.IndexCount, 1, Element_Left.StartIndexLocation, Element_Left.BaseVertexLocation, 0);
//}


void Player::SetPosition(float x, float y, float z)
{
	World._41 = x;
	World._42 = y;
	World._43 = z;
}

void Player::SetPosition(XMFLOAT3 xmf3Position)
{
	Move(XMFLOAT3(xmf3Position.x - m_xmf3Position.x, xmf3Position.y - m_xmf3Position.y, xmf3Position.z - m_xmf3Position.z));
}

void Player::MoveStrafe(float fDistance)
{
}

void Player::MoveUp(float fDistance)
{
}

void Player::MoveForward(float fDistance)
{
}

void Player::Rotate(float fPitch, float fYaw, float fRoll)
{
	XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(fPitch), XMConvertToRadians(fRoll), XMConvertToRadians(fYaw));
	World = Matrix4x4::Multiply(mtxRotate, World);
}

void Player::Rotate(XMFLOAT3 * pxmf3Axis, float fAngle)
{
	XMMATRIX mtxRotate = XMMatrixRotationAxis(XMLoadFloat3(pxmf3Axis), XMConvertToRadians(fAngle));
	World = Matrix4x4::Multiply(mtxRotate, World);
}


void Player::Move(const XMFLOAT3 & xmf3Shift, bool bVelocity)
{


	XMFLOAT3 CurPos = XMFLOAT3(World._41, World._42, World._43);

	m_xmf3Position = Vector3::Add(CurPos, xmf3Shift);

	XMFLOAT3 xmf3shiftTest = xmf3Shift;

	//XMFLOAT3 test = Vector3::Subtract(CurPos, xmf3shiftTest);

	//cout << xmf3Shift.x << "\t" << xmf3Shift.y << "\t" << xmf3Shift.z << endl;



	World._41 = m_xmf3Position.x;
	World._42 = m_xmf3Position.y;
	World._43 = m_xmf3Position.z;

}

void Player::Move(DWORD dwDirection, float fDistance, bool bUpdateVelocity)
{


	if (dwDirection)
	{
		XMFLOAT3 xmf3Shift = XMFLOAT3(0, 0, 0);
		if (dwDirection & DIR_FORWARD) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Height, fDistance);
		if (dwDirection & DIR_BACKWARD) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Height, -fDistance);
		if (dwDirection & DIR_RIGHT) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Width, fDistance);
		if (dwDirection & DIR_LEFT) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Width, -fDistance);

		//   xmf3Shift = Vector3::Normalize(xmf3Shift);
		//xmf3Shift += m_MovingRefletVector;
		//xmf3Shift = Vector3::Normalize(xmf3Shift);

		// S = xmf3Shift
		// n = m_MovingRfelctVector
		// P = xmf3Shift

		// S = P - n(P·n)

		/*cout << " ---------------------" << endl;
		cout << "Shift : " << xmf3Shift.x << "\t" << xmf3Shift.y << "\t" << xmf3Shift.z << endl;
		cout << "m_MovingRefletVector : " << m_MovingRefletVector.x << "\t" << m_MovingRefletVector.y << "\t" << m_MovingRefletVector.z << endl;
		cout << " ---------------------" << endl;*/

		//cout << "m_MovingRefletVector : " << m_MovingRefletVector.x << "\t" << m_MovingRefletVector.y << "\t" << m_MovingRefletVector.z << endl;

		Move(xmf3Shift, bUpdateVelocity);


		if (dwDirection & DIR_FORWARD)
		{

			xmf3Shift = Vector3::Normalize(xmf3Shift);
			XMFLOAT3 playerLook = Vector3::Normalize(XMFLOAT3(-World._21, -World._22, -World._23));

			XMFLOAT3 crossVector = Vector3::CrossProduct(xmf3Shift, playerLook, true);
			if (crossVector.y > 0)
			{
				float dotproduct = Vector3::DotProduct(xmf3Shift, playerLook);
				float xmf3ShiftLength = Vector3::Length(xmf3Shift);
				float xmf3PlayerLooklength = Vector3::Length(playerLook);

				float cosCeta = dotproduct / xmf3ShiftLength * xmf3PlayerLooklength;

				float ceta = acos(cosCeta); // 현재 각도

				ceta = ceta * m_fDegree;

				//   cout << ceta << endl;
				if (ceta > 0.1f)
				{
					Rotate(0.0f, -m_fRotateSpeed, 0.0f);

				}
			}
			else
			{
				float dotproduct = Vector3::DotProduct(xmf3Shift, playerLook);
				float xmf3ShiftLength = Vector3::Length(xmf3Shift);
				float xmf3PlayerLooklength = Vector3::Length(playerLook);

				float cosCeta = dotproduct / xmf3ShiftLength * xmf3PlayerLooklength;

				float ceta = acos(cosCeta); // 현재 각도

				ceta = ceta * m_fDegree;

				//   cout << ceta << endl;
				if (ceta > 0.1f)
				{
					Rotate(0.0f, m_fRotateSpeed, 0.0f);

				}
			}


		}
		else if (dwDirection & DIR_BACKWARD)
		{


			xmf3Shift = Vector3::Normalize(xmf3Shift);
			XMFLOAT3 playerLook = Vector3::Normalize(XMFLOAT3(-World._21, -World._22, -World._23));

			XMFLOAT3 crossVector = Vector3::CrossProduct(xmf3Shift, playerLook, true);
			if (crossVector.y > 0)
			{
				float dotproduct = Vector3::DotProduct(xmf3Shift, playerLook);
				float xmf3ShiftLength = Vector3::Length(xmf3Shift);
				float xmf3PlayerLooklength = Vector3::Length(playerLook);

				float cosCeta = dotproduct / xmf3ShiftLength * xmf3PlayerLooklength;

				float ceta = acos(cosCeta); // 현재 각도

				ceta = ceta * m_fDegree;

				//   cout << ceta << endl;
				if (ceta > 0.1f)
				{
					Rotate(0.0f, -m_fRotateSpeed, 0.0f);

				}
			}
			else
			{
				float dotproduct = Vector3::DotProduct(xmf3Shift, playerLook);
				float xmf3ShiftLength = Vector3::Length(xmf3Shift);
				float xmf3PlayerLooklength = Vector3::Length(playerLook);

				float cosCeta = dotproduct / xmf3ShiftLength * xmf3PlayerLooklength;

				float ceta = acos(cosCeta); // 현재 각도

				ceta = ceta * m_fDegree;

				//   cout << ceta << endl;
				if (ceta > 0.1f)
				{
					Rotate(0.0f, m_fRotateSpeed, 0.0f);

				}
			}

		}
		else if (dwDirection & DIR_RIGHT)
		{
			xmf3Shift = Vector3::Normalize(xmf3Shift);
			XMFLOAT3 playerLook = Vector3::Normalize(XMFLOAT3(-World._21, -World._22, -World._23));

			XMFLOAT3 crossVector = Vector3::CrossProduct(xmf3Shift, playerLook, true);
			if (crossVector.y > 0)
			{
				float dotproduct = Vector3::DotProduct(xmf3Shift, playerLook);
				float xmf3ShiftLength = Vector3::Length(xmf3Shift);
				float xmf3PlayerLooklength = Vector3::Length(playerLook);

				float cosCeta = dotproduct / xmf3ShiftLength * xmf3PlayerLooklength;

				float ceta = acos(cosCeta); // 현재 각도

				ceta = ceta * m_fDegree;

				//   cout << ceta << endl;
				if (ceta > 0.1f)
				{
					Rotate(0.0f, -m_fRotateSpeed, 0.0f);

				}
			}
			else
			{
				float dotproduct = Vector3::DotProduct(xmf3Shift, playerLook);
				float xmf3ShiftLength = Vector3::Length(xmf3Shift);
				float xmf3PlayerLooklength = Vector3::Length(playerLook);

				float cosCeta = dotproduct / xmf3ShiftLength * xmf3PlayerLooklength;

				float ceta = acos(cosCeta); // 현재 각도

				ceta = ceta * m_fDegree;

				//   cout << ceta << endl;
				if (ceta > 0.1f)
				{
					Rotate(0.0f, m_fRotateSpeed, 0.0f);

				}
			}

		}
		else if (dwDirection & DIR_LEFT)
		{
			xmf3Shift = Vector3::Normalize(xmf3Shift);
			XMFLOAT3 playerLook = Vector3::Normalize(XMFLOAT3(-World._21, -World._22, -World._23));

			XMFLOAT3 crossVector = Vector3::	(xmf3Shift, playerLook, true);
			if (crossVector.y > 0)
			{
				float dotproduct = Vector3::DotProduct(xmf3Shift, playerLook);
				float xmf3ShiftLength = Vector3::Length(xmf3Shift);
				float xmf3PlayerLooklength = Vector3::Length(playerLook);

				float cosCeta = dotproduct / xmf3ShiftLength * xmf3PlayerLooklength;

				float ceta = acos(cosCeta); // 현재 각도

				ceta = ceta * m_fDegree;

				//cout << ceta << endl;
				if (ceta > 0.1f)
				{
					Rotate(0.0f, -m_fRotateSpeed, 0.0f);

				}
			}
			else
			{
				float dotproduct = Vector3::DotProduct(xmf3Shift, playerLook);
				float xmf3ShiftLength = Vector3::Length(xmf3Shift);
				float xmf3PlayerLooklength = Vector3::Length(playerLook);

				float cosCeta = dotproduct / xmf3ShiftLength * xmf3PlayerLooklength;

				float ceta = acos(cosCeta); // 현재 각도

				ceta = ceta * m_fDegree;

				//cout << ceta << endl;
				if (ceta > 0.1f)
				{
					Rotate(0.0f, m_fRotateSpeed, 0.0f);

				}
			}

		}



	}
}