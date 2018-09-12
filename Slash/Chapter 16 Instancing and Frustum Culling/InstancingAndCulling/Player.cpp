#include "stdafx.h"
#include "Player.h"
#include "Define.h"
#include "DynamicMesh.h"
#include "InputDevice.h"
#include "Network.h"
#include "Component_Manager.h"
#include "Texture_Manager.h"
#include "Effect_Manager.h"
#include "Management.h"
#include "Renderer.h"
#include "Camera.h"
#include "HPBar.h"

Player::Player(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice, ComPtr<ID3D12DescriptorHeap> &srv, UINT srvSize, bool isWarrior)
	: CGameObject(d3dDevice, srv, srvSize)
	, m_IsWarrior(isWarrior)
	, m_HpBar(nullptr)
	, m_ExpBar(nullptr)
	, m_GageBar(nullptr)
{
	m_preKeyInputTime = 0;
	m_curKeyInputTime = 0;
}

Player::~Player()
{
	delete m_pMesh;

	delete Mat;

	//����
}


void Player::Animate(const GameTimer & gt)
{
	if (AnimStateMachine->GetAnimState() == State::STATE_ATTACK1)
	{
		if (GetAnimateMachine()->GetCurAnimFrame() == 0)
		{
			if (m_bAttackMotionForSound == true)
			{
				CManagement::GetInstance()->GetSound()->PlayEffect(L"Sound", L"Attack");
				m_bAttackMotionForSound = false;
			}
		}
		if (GetAnimateMachine()->GetCurAnimFrame() != 0)
		{
			m_bAttackMotionForSound = true;
		}
// > =======================Test===========================
		//if (GetAnimateMachine()->GetCurAnimFrame() == 8)
		//{
		//	AnimStateMachine->SetAnimState(State::STATE_IDLE);
		//	CNetwork::GetInstance()->SendStopPacket();
		//}
// > ===========================================================

	}
	//if (GetHp() < 0)
	//{
	//	SetObjectAnimState(State::STATE_DEAD);
	//	AnimStateMachine->AnimationStateUpdate(gt);
	//	return;
	//}


	AnimStateMachine->AnimationStateUpdate(gt); //�ִϸ��̼� ���� �������ִ� �Լ�

	KeyInput(gt);

	//if (CInputDevice::GetInstance()->AnyKeyInput())
	//{
	//	
	//	if (!AnimStateMachine->bTimerAttack1 &&
	//		!AnimStateMachine->bTimerAttack2 &&
	//		!AnimStateMachine->bTimerAttack3 /*&&
	//		!pTestMesh->bTimerTestWalk*/
	//		)
	//	{
	//		
	//		AnimStateMachine->SetAnimState(State::STATE_IDLE);

	//	//	KeyInputTest = 0;

	//	}
	//}

	// ���� �� �����ؾ��� // ���� ��ķ�..? // �����۾� // ȸ�� �������� // ��ġ ������ // �Ƶ��� �׳� ���� ���ڰ���
	m_xmOOBBTransformed.Transform(m_xmOOBB, XMLoadFloat4x4(&(GetWorld())));
	XMStoreFloat4(&m_xmOOBBTransformed.Orientation, XMQuaternionNormalize(XMLoadFloat4(&m_xmOOBBTransformed.Orientation)));

}

bool Player::Update(const GameTimer & gt)
{
	CheckUltimate(gt);

	//return true;
	CGameObject::Update(gt);

	Animate(gt);
	//m_HPBar->SetHp(m_pPlayer->GetHp());
	m_HpBar->SetHp(GetHp());
	if (!IsSoundIn)
	{
		float X = World._41;
		float Z = World._43;
		if (X > 120.f && X < 160.f)
		{
			if (Z > 45.f && Z < 90.f)
			{
				IsSoundIn = true;
				//
			}
		}
	}

	else
	{
		if(!IsSoundChange)
		{
			CManagement::GetInstance()->GetSound()->Set_BGM_Volume(-1.f * gt.DeltaTime() * 5.f);
			float fVolume = CManagement::GetInstance()->GetSound()->Get_BGM_Volume();
			if (fVolume < 0.2)
			{
				CManagement::GetInstance()->GetSound()->PlayBGM(L"Sound", L"Combat", 1.f);
				//CManagement::GetInstance()->GetSound()->Set_BGM_Volume(-1.f * gt.DeltaTime() * 30.f);
				IsSoundChange = true;
			}
		}
	}

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

	CManagement::GetInstance()->GetRenderer()->Add_RenderGroup(CRenderer::RENDER_NONALPHA_FORWARD, this);

	m_HpBar->Update(gt);

	return true;
}

void Player::Render(ID3D12GraphicsCommandList * cmdList)
{
	//return;
	AnimStateMachine->SetTimerTrueFalse(); //� �ִϸ��̼��� ���� �����ִ� �� 

	Render_Head(cmdList);
	Render_Body(cmdList);
	Render_Right(cmdList);
	//Render_Left(cmdList);
}
HRESULT Player::Initialize()
{
	string strTexName;
	if (m_IsWarrior)
	{
		m_pMesh = dynamic_cast<DynamicMesh*>(CComponent_Manager::GetInstance()->Clone_Component(L"Com_Mesh_Warrior"));
		strTexName = "VillagerTex";
	}

	else
	{
		m_pMesh = dynamic_cast<DynamicMesh*>(CComponent_Manager::GetInstance()->Clone_Component(L"Com_Mesh_Mage"));
		strTexName = "MageTex";
	}

	if (nullptr == m_pMesh)
		return E_FAIL;

	// > �׽�Ʈ������ �־��
	int test[State::STATE_END] = { 0, };
	AnimStateMachine = AnimateStateMachine_Player::Create(this, L"Warrior", test, test);
	if (AnimStateMachine == nullptr)
		return E_FAIL;

	
	AnimStateMachine->vecAnimFrame = &(dynamic_cast<DynamicMesh*>(m_pMesh)->vecAnimFrame);

	Texture* tex = CTexture_Manager::GetInstance()->Find_Texture(strTexName, CTexture_Manager::TEX_DEFAULT_2D);			// �̷������� ������
	if (tex == nullptr)
		return E_FAIL;

	Mat = new Material;
	Mat->Name = "InsecMat";
	Mat->MatCBIndex = m_iMyObjectID;
	Mat->DiffuseSrvHeapIndex = tex->Num;			// �ؽ��� ���� ���大�ܳ� ���ƾƤ��Ƥ� ���� ����߱׷� �̰��̷��Թٲ���� 
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


	m_xmf3Scale = XMFLOAT3(0.05f, 0.05f, 0.05f);
	m_xmf3Rot = XMFLOAT3(1.7f, 0.f, 3.14f);


	// > Hp, Exp, Gage Bar UI Setting
	XMFLOAT2 move = XMFLOAT2(-0.3f, 7.3f);

	move.x = -0.3f;
	move.y = 7.3f;

	XMFLOAT2 scale = XMFLOAT2(1.2f, 0.125f);
	scale.x = 1.2f;
	scale.y = 0.125f;
	float size = 0.25f;

	tex = CTexture_Manager::GetInstance()->Find_Texture("BloodTex", CTexture_Manager::TEX_DEFAULT_2D);

	// > Hp Bar
	m_HpBar = HPBar::Create(m_d3dDevice, mSrvDescriptorHeap, mCbvSrvDescriptorSize, move, scale, size, tex->Num);
	m_HpBar->SetCamera(CManagement::GetInstance()->Get_CurScene()->Get_MainCam());
	//CManagement::GetInstance()->Get_CurScene()->Ready_GameObject(L"Layer_HPBar", m_HpBar);

	// > Exp Bar

	//SetOOBB(XMFLOAT3(Bounds.Center.x * 0.05f, Bounds.Center.y * 0.05f, Bounds.Center.z * 0.05f), XMFLOAT3(Bounds.Extents.x * 0.05f, Bounds.Extents.y * 0.05f, Bounds.Extents.z * 0.05f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));

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
	XMFLOAT3 look = XMFLOAT3(-World._21, -World._22, -World._23);

	Vector3::Normalize(look);



	UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
	UINT matCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(MaterialConstants));

	auto objectCB = m_pFrameResource->ObjectCB->Resource();
	auto matCB = m_pFrameResource->MaterialCB->Resource();

	cmdList->IASetVertexBuffers(0, 1, &Geo_Head->VertexBufferView());
	cmdList->IASetIndexBuffer(&Geo_Head->IndexBufferView());
	cmdList->IASetPrimitiveTopology(PrimitiveType);

	CD3DX12_GPU_DESCRIPTOR_HANDLE tex(mSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
	if (bIsUltimateState)
	{
		Texture* WarriorUltimateTex = CTexture_Manager::GetInstance()->Find_Texture("WarriorUltimateTex", CTexture_Manager::TEX_DEFAULT_2D);
		if (WarriorUltimateTex == nullptr)
			return;
		tex.Offset(WarriorUltimateTex->Num, mCbvSrvDescriptorSize);
	}
	else
		tex.Offset(Mat->DiffuseSrvHeapIndex, mCbvSrvDescriptorSize);

	Mat->DiffuseSrvHeapIndex;
	D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + ObjCBIndex * objCBByteSize;
	D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress() + Mat->MatCBIndex*matCBByteSize;

	cmdList->SetGraphicsRootConstantBufferView(4, objCBAddress);
	//cmdList->SetGraphicsRootConstantBufferView(5, matCBAddress);
	cmdList->SetGraphicsRootShaderResourceView(5, matCBAddress);

	cmdList->SetGraphicsRootDescriptorTable(7, tex);

	auto indexcnt = dynamic_cast<DynamicMesh*>(m_pMesh)->m_vecIndexOffset[0][1];

	int iTest = AnimStateMachine->GetCurAnimFrame();
	int KeyInputTest = AnimStateMachine->GetAnimState();


	//cmdList->DrawIndexedInstanced(Element_Head.IndexCount, 1, Element_Head.StartIndexLocation, Element_Head.BaseVertexLocation , 0);
	//	dynamic_cast<DynamicMesh*>(m_pMesh)->m_vecIndexOffset[0].
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
	if (bIsUltimateState)
	{
		Texture* WarriorUltimateTex = CTexture_Manager::GetInstance()->Find_Texture("WarriorUltimateTex", CTexture_Manager::TEX_DEFAULT_2D);
		if (WarriorUltimateTex == nullptr)
			return;
		tex.Offset(WarriorUltimateTex->Num, mCbvSrvDescriptorSize);
	}
	else
		tex.Offset(Mat->DiffuseSrvHeapIndex, mCbvSrvDescriptorSize);

	Mat->DiffuseSrvHeapIndex;
	D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + ObjCBIndex * objCBByteSize;
	D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress() + Mat->MatCBIndex*matCBByteSize;

	cmdList->SetGraphicsRootConstantBufferView(4, objCBAddress);
	//cmdList->SetGraphicsRootConstantBufferView(5, matCBAddress);
	cmdList->SetGraphicsRootShaderResourceView(5, matCBAddress);

	cmdList->SetGraphicsRootDescriptorTable(7, tex);

	auto indexcnt = dynamic_cast<DynamicMesh*>(m_pMesh)->m_vecIndexOffset[1][1];

	int iTest = AnimStateMachine->GetCurAnimFrame();
	int KeyInputTest = AnimStateMachine->GetAnimState();

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
	if (bIsUltimateState)
	{
		Texture* WarriorUltimateTex = CTexture_Manager::GetInstance()->Find_Texture("WarriorUltimateTex", CTexture_Manager::TEX_DEFAULT_2D);
		if (WarriorUltimateTex == nullptr)
			return;
		tex.Offset(WarriorUltimateTex->Num, mCbvSrvDescriptorSize);
	}
	else
	tex.Offset(Mat->DiffuseSrvHeapIndex, mCbvSrvDescriptorSize);

	Mat->DiffuseSrvHeapIndex;
	D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + ObjCBIndex * objCBByteSize;
	D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress() + Mat->MatCBIndex*matCBByteSize;

	cmdList->SetGraphicsRootConstantBufferView(4, objCBAddress);
	//cmdList->SetGraphicsRootConstantBufferView(5, matCBAddress);
	cmdList->SetGraphicsRootShaderResourceView(5, matCBAddress);

	cmdList->SetGraphicsRootDescriptorTable(7, tex);

	auto indexcnt = dynamic_cast<DynamicMesh*>(m_pMesh)->m_vecIndexOffset[2][1];

	int iTest = AnimStateMachine->GetCurAnimFrame();
	int KeyInputTest = AnimStateMachine->GetAnimState();


	cmdList->DrawIndexedInstanced(indexcnt, 1,
		Element_Right.StartIndexLocation + dynamic_cast<DynamicMesh*>(m_pMesh)->m_vecIndexOffset[2][iTest] + dynamic_cast<DynamicMesh*>(m_pMesh)->m_vecIndexAnimOffset[2][KeyInputTest/*dynamic_cast<DynamicMesh*>(m_pMesh)->iAnimframe*/],
		Element_Right.BaseVertexLocation + dynamic_cast<DynamicMesh*>(m_pMesh)->m_vecVertexOffset[2][iTest] + dynamic_cast<DynamicMesh*>(m_pMesh)->m_vecVertexAnimOffset[2][KeyInputTest/*dynamic_cast<DynamicMesh*>(m_pMesh)->iAnimframe*/],
		0);
}
Player * Player::Create(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice, ComPtr<ID3D12DescriptorHeap>& srv, UINT srvSize, bool isWarrior)
{
	Player* pInstance = new Player(d3dDevice, srv, srvSize, isWarrior);

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX(L"Player Created Failed");
		Safe_Release(pInstance);
	}

	return pInstance;
}

//void Player::Render_Left(ID3D12GraphicsCommandList * cmdList)
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


void Player::CheckUltimate(const GameTimer & gt)
{
	if (bIsUltimateState)
	{
		m_fUltimateTime -= gt.DeltaTime();
		if (m_fUltimateTime < 0.0f)
		{
			m_fUltimateTime = 20.0f;
			bIsUltimateState = false;
		}
	}
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

void Player::KeyInput(const GameTimer & gt)
{
	if (GetHp() <= 0) return;
	if (AnimStateMachine->GetAnimState() == State::STATE_DEAD) return;
	if (AnimStateMachine->GetAnimState() == State::STATE_ATTACK1) return;
	if (AnimStateMachine->GetAnimState() == State::STATE_ATTACK2) return;
	if (AnimStateMachine->GetAnimState() == State::STATE_ATTACK3) return;
	if (AnimStateMachine->GetAnimState() == State::STATE_ROLL) return;
	if (AnimStateMachine->GetAnimState() == State::STATE_ULTIMATE ) return;

	if (CManagement::GetInstance()->Get_IsStop() == true)
		return;
	DWORD dwDirection = 0;
	static bool IsPlayerMoved = false;


	if (CManagement::GetInstance()->Get_MainCam() != NULL)
	{
		if (!CManagement::GetInstance()->Get_MainCam()->bFirstPersonView && !bIsUltimateState)
		{
			if (KeyBoard_Input(DIK_UP) == CInputDevice::INPUT_PRESS) dwDirection |= CS_DIR_FORWARD;
			if (KeyBoard_Input(DIK_DOWN) == CInputDevice::INPUT_PRESS) dwDirection |= CS_DIR_BACKWARD;
			if (KeyBoard_Input(DIK_LEFT) == CInputDevice::INPUT_PRESS) dwDirection |= CS_DIR_LEFT;
			if (KeyBoard_Input(DIK_RIGHT) == CInputDevice::INPUT_PRESS) dwDirection |= CS_DIR_RIGHT;
		}
		if (!CManagement::GetInstance()->Get_MainCam()->bFirstPersonView && bIsUltimateState)
		{
			if (KeyBoard_Input(DIK_UP) == CInputDevice::INPUT_PRESS) dwDirection |= CS_DIR_RIGHT;
			if (KeyBoard_Input(DIK_DOWN) == CInputDevice::INPUT_PRESS) dwDirection |= CS_DIR_LEFT;
			if (KeyBoard_Input(DIK_LEFT) == CInputDevice::INPUT_PRESS) dwDirection |= CS_DIR_FORWARD;
			if (KeyBoard_Input(DIK_RIGHT) == CInputDevice::INPUT_PRESS) dwDirection |= CS_DIR_BACKWARD;
		}

		else
		{
			if (KeyBoard_Input(DIK_W) == CInputDevice::INPUT_PRESS) dwDirection |= CS_DIR_RIGHT;
			if (KeyBoard_Input(DIK_S) == CInputDevice::INPUT_PRESS) dwDirection |= CS_DIR_LEFT;
			if (KeyBoard_Input(DIK_A) == CInputDevice::INPUT_PRESS) dwDirection |= CS_DIR_FORWARD;
			if (KeyBoard_Input(DIK_D) == CInputDevice::INPUT_PRESS) dwDirection |= CS_DIR_BACKWARD;
			if (KeyBoard_Input(DIK_LSHIFT) == CInputDevice::INPUT_DOWN) dwDirection |= CS_ROLL; // �̰� ���� �Լ��� ������
		}

	}


	//m_curKeyInputTime = gt.TotalTime();
	//if (m_curKeyInputTime - m_preKeyInputTime > gt.DeltaTime())
	{
		if (0 == dwDirection)
		{
			if (IsPlayerMoved)
			{
				if (AnimStateMachine->GetAnimState() != State::STATE_ROLL)
				{
					AnimStateMachine->SetAnimState(STATE_IDLE);
					CNetwork::GetInstance()->SendStopPacket();
					IsPlayerMoved = false;
				}
			}
		}
		else
		{
			CNetwork::GetInstance()->SendDirKeyPacket(dwDirection);
			IsPlayerMoved = true;
		}
		//m_preKeyInputTime = gt.TotalTime();
	}
	if (m_bIsConnected)
	{
		auto m_pCamera = CManagement::GetInstance()->Get_MainCam();
		
	
		if (KeyBoard_Input(DIK_1) == CInputDevice::INPUT_DOWN)
		{
			//CManagement::GetInstance()->Add_NumUI(21, XMFLOAT3(GetPosition().x, GetPosition().y +3, GetPosition().z));

			m_pCamera->SetCameraEffect(Camera::SHAKING);
			CNetwork::GetInstance()->SendAttack1Packet();
		}
		else if (KeyBoard_Input(DIK_2) == CInputDevice::INPUT_DOWN)
		{
			//CManagement::GetInstance()->Add_NumUI(210, XMFLOAT3(GetPosition().x, GetPosition().y + 3, GetPosition().z));
			CNetwork::GetInstance()->SendAttack2Packet();

		}
		else if (KeyBoard_Input(DIK_3) == CInputDevice::INPUT_DOWN)
			CNetwork::GetInstance()->SendAttack3Packet();
		else if (KeyBoard_Input(DIK_R) == CInputDevice::INPUT_DOWN)
		{
			bIsUltimateState = true; //�û��·�
			m_pCamera->SetCameraEffect(Camera::ZOOMINROUNDULTIMATE, CManagement::GetInstance()->Find_Object(L"Layer_Player"));

			SetObjectAnimState(State::STATE_ULTIMATE);
		}
		else if (KeyBoard_Input(DIK_4) == CInputDevice::INPUT_DOWN)
		{
			m_pCamera->SetCameraEffect(Camera::ZOOMIN, CManagement::GetInstance()->Find_Object(L"Layer_Dragon"));
		}
	}

	if (KeyBoard_Input(DIK_O) == CInputDevice::INPUT_DOWN)
	{
		//KeyInputTest = 2;
	}


	if (KeyBoard_Input(DIK_L) == CInputDevice::INPUT_PRESS)
	{

		SetHp(GetHp() - 1.0f);

		//cout << GetHp() << endl;
	}
	if (KeyBoard_Input(DIK_K) == CInputDevice::INPUT_PRESS)
	{
		SetHp(GetHp() + 1.0f);
		//cout << GetHp() << endl;

	}
	//if (KeyBoard_Input(DIK_T) == CInputDevice::INPUT_DOWN)
	//{
	//	auto * m_pPNagaGuard= CManagement::GetInstance()->Find_Object(L"Layer_NagaGuard");
	//	auto * m_pPRockWarrior = CManagement::GetInstance()->Find_Object(L"Layer_RockWarrior");
	//	auto * m_pTreeGuard = CManagement::GetInstance()->Find_Object(L"Layer_TreeGuard");
	//	auto * m_pMushroom = CManagement::GetInstance()->Find_Object(L"Layer_Mushroom");

	//	m_pPNagaGuard->SetObjectAnimState(AnimationtTest);
	//	m_pPRockWarrior->SetObjectAnimState(AnimationtTest);
	//	m_pTreeGuard->SetObjectAnimState(AnimationtTest);
	//	m_pMushroom->SetObjectAnimState(AnimationtTest);

	//	AnimationtTest++;

	//	if (AnimationtTest > 5)
	//		AnimationtTest = 0;
	//}
}

//void Player::Move(DWORD dwDirection, float fDistance, bool bUpdateVelocity, const GameTimer & gt)
//{
//	
//
//	
//		if (dwDirection)
//		{
//			XMFLOAT3 xmf3Shift = XMFLOAT3(0, 0, 0);
//			if (dwDirection & DIR_FORWARD) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Height, fDistance);
//			if (dwDirection & DIR_BACKWARD) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Height, -fDistance);
//			if (dwDirection & DIR_RIGHT) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Width, fDistance);
//			if (dwDirection & DIR_LEFT) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Width, -fDistance);
//
//			//	xmf3Shift = Vector3::Normalize(xmf3Shift);
//			//xmf3Shift += m_MovingRefletVector;
//			//xmf3Shift = Vector3::Normalize(xmf3Shift);
//
//			// S = xmf3Shift
//			// n = m_MovingRfelctVector
//			// P = xmf3Shift
//
//			// S = P - n(P��n)
//
//			/*cout << " ---------------------" << endl;
//			cout << "Shift : " << xmf3Shift.x << "\t" << xmf3Shift.y << "\t" << xmf3Shift.z << endl;
//			cout << "m_MovingRefletVector : " << m_MovingRefletVector.x << "\t" << m_MovingRefletVector.y << "\t" << m_MovingRefletVector.z << endl;
//			cout << " ---------------------" << endl;*/
//
//			float cosCeta = Vector3::DotProduct(m_MovingRefletVector, xmf3Shift);
//			if (cosCeta < 0)
//				//if(!Vector3::IsEqual(Vector3::Normalize(m_MovingRefletVector), Vector3::Normalize(xmf3Shift)))
//				xmf3Shift = Vector3::Subtract(xmf3Shift, Vector3::MultiplyScalr(m_MovingRefletVector, Vector3::DotProduct(xmf3Shift, m_MovingRefletVector)));
//
//			//cout << xmf3Shift.x << "\t" << xmf3Shift.y << "\t" << xmf3Shift.z << endl;
//			//cout << "m_MovingRefletVector : " << m_MovingRefletVector.x << "\t" << m_MovingRefletVector.y << "\t" << m_MovingRefletVector.z << endl;
//
//		
//
//			Move(xmf3Shift, bUpdateVelocity);
//
//
//		}
//
//	
//}

//--------------------------------------- AnimateStateMachine-----------------------------------------

AnimateStateMachine_Player::AnimateStateMachine_Player(CGameObject* pObj, wchar_t * pMachineName, int SoundFrame[State::STATE_END], int EffectFrame[State::STATE_END])
	: m_pMachineName(pMachineName)
	, m_pObject(pObj)
{
	for (int i = 0; i < State::STATE_END; ++i)
	{
		m_SoundFrame[i] = SoundFrame[i];
		m_EffectFrame[i] = EffectFrame[i];
		m_IsSoundPlay[i] = false;
		m_IsEffectPlay[i] = false;
	}
}

AnimateStateMachine_Player::~AnimateStateMachine_Player()
{
}

HRESULT AnimateStateMachine_Player::Initialize()
{
	return S_OK;
}

// > ---------------------------- StateMachine_Player -----------------------------------
void AnimateStateMachine_Player::AnimationStateUpdate(const GameTimer & gt)
{
	if (bTimerIdle == true)
	{
		m_fAnimationKeyFrameIndex += gt.DeltaTime() * 25;
		//m_iCurAnimFrame = m_fAnimationKeyFrameIndex;
		if (m_fAnimationKeyFrameIndex > (*vecAnimFrame)[0])
		{
			bTimerIdle = false;

			m_fAnimationKeyFrameIndex = 0;
		}
	}


	if (bTimerWalk == true)
	{
		m_fAnimationKeyFrameIndex_Walk += gt.DeltaTime() * 45;
		//m_iCurAnimFrame = m_fAnimationKeyFrameIndex_Walk;
		if (m_fAnimationKeyFrameIndex_Walk > (*vecAnimFrame)[1])
		{
			bTimerWalk = false;
			m_fAnimationKeyFrameIndex_Walk = 0;
		}

	}


	if (bTimerAttack1 == true)
	{

		m_fAnimationKeyFrameIndex_Attack1 += gt.DeltaTime() * 20;
		//m_iCurAnimFrame = m_fAnimationKeyFrameIndex_Attack1;
		if (!m_IsSoundPlay[State::STATE_ATTACK1] && m_fAnimationKeyFrameIndex_Attack1 > m_SoundFrame[State::STATE_ATTACK1])
		{
			m_IsSoundPlay[State::STATE_ATTACK1] = true;
			CManagement::GetInstance()->GetSound()->PlayEffect(L"Sound", L"Attack");
			//CManagement::GetInstance()->GetSound()->PlayEffect(m_pMachineName, m_pStateName[State::STATE_ATTACK1]);
		}

		if (!m_IsEffectPlay[State::STATE_ATTACK1] && m_fAnimationKeyFrameIndex_Attack1 > m_EffectFrame[State::STATE_ATTACK1])
		{
			m_IsEffectPlay[State::STATE_ATTACK1] = true;
			// > ��ų�־��ֱ�
			//CEffect_Manager::GetInstance()->Play_SkillEffect("��ų�̸�");
			//cout << "��ų!" << endl;
			CEffect_Manager::GetInstance()->Play_SkillEffect("Hit5", &m_pObject->GetWorld());
			//cout << "Player Pos : " << m_pObject->GetPosition().x << ", " << m_pObject->GetPosition().y << ", " << m_pObject->GetPosition().z << endl;
		}

		if (m_fAnimationKeyFrameIndex_Attack1 > (*vecAnimFrame)[2])
		{
			bTimerAttack1 = false;
			m_fAnimationKeyFrameIndex_Attack1 = 0.f;

			m_IsSoundPlay[State::STATE_ATTACK1] = false;
			m_IsEffectPlay[State::STATE_ATTACK1] = false;

			m_pObject->GetAnimateMachine()->SetAnimState(STATE_IDLE);
			CNetwork::GetInstance()->SendStopPacket();
		}

	}


	if (bTimerAttack2 == true)
	{

		m_fAnimationKeyFrameIndex_Attack2 += gt.DeltaTime() * 30;
		//m_iCurAnimFrame = m_fAnimationKeyFrameIndex_Attack2;

		if (!m_IsSoundPlay[State::STATE_ATTACK2] && m_fAnimationKeyFrameIndex_Attack2 > m_SoundFrame[State::STATE_ATTACK2])
		{
			m_IsSoundPlay[State::STATE_ATTACK2] = true;
			CManagement::GetInstance()->GetSound()->PlayEffect(L"Sound", L"Attack");
			//CManagement::GetInstance()->GetSound()->PlayEffect(m_pMachineName, m_pStateName[State::STATE_ATTACK2]);
		}

		if (!m_IsEffectPlay[State::STATE_ATTACK2] && m_fAnimationKeyFrameIndex_Attack2 > m_EffectFrame[State::STATE_ATTACK2])
		{
			m_IsEffectPlay[State::STATE_ATTACK2] = true;
			// > ��ų�־��ֱ�
			//CEffect_Manager::GetInstance()->Play_SkillEffect("��ų�̸�");
			XMFLOAT4X4 matWorld = Matrix4x4::Identity();
			matWorld._41 = m_pObject->GetWorld()._41;
			matWorld._42 = m_pObject->GetWorld()._42;
			matWorld._43 = m_pObject->GetWorld()._43;
			CEffect_Manager::GetInstance()->Play_SkillEffect("orbAttack", &m_pObject->GetWorld(), m_pObject->GetNetRotAngle());
		}

		if (m_fAnimationKeyFrameIndex_Attack2 > (*vecAnimFrame)[3])
		{
			bTimerAttack2 = false;
			m_fAnimationKeyFrameIndex_Attack2 = 0;

			m_IsSoundPlay[State::STATE_ATTACK2] = false;
			m_IsEffectPlay[State::STATE_ATTACK2] = false;

			m_pObject->GetAnimateMachine()->SetAnimState(STATE_IDLE);
			CNetwork::GetInstance()->SendStopPacket();
		}


	}

	if (bTimerAttack3 == true)
	{

		m_fAnimationKeyFrameIndex_Attack3 += gt.DeltaTime() * 30;
		//m_iCurAnimFrame = m_fAnimationKeyFrameIndex_Attack3;

		if (!m_IsSoundPlay[State::STATE_ATTACK3] && m_fAnimationKeyFrameIndex_Attack3 > m_SoundFrame[State::STATE_ATTACK3])
		{
			m_IsSoundPlay[State::STATE_ATTACK3] = true;
			//CManagement::GetInstance()->GetSound()->PlayEffect(m_pMachineName, m_pStateName[State::STATE_ATTACK3]);		// > ��� ���尡 ������ �̷��� �ٲ����!
			CManagement::GetInstance()->GetSound()->PlayEffect(L"Sound", L"Attack");
		}

		if (!m_IsEffectPlay[State::STATE_ATTACK3] && m_fAnimationKeyFrameIndex_Attack3 > m_EffectFrame[State::STATE_ATTACK3])
		{
			m_IsEffectPlay[State::STATE_ATTACK3] = true;
			// > ��ų�־��ֱ�
			//CEffect_Manager::GetInstance()->Play_SkillEffect("��ų�̸�");
			CEffect_Manager::GetInstance()->Play_SkillEffect("Heal_00", &m_pObject->GetWorld());
		}

		if (m_fAnimationKeyFrameIndex_Attack3 > (*vecAnimFrame)[4])
		{
			bTimerAttack3 = false;
			m_fAnimationKeyFrameIndex_Attack3 = 0;

			m_IsSoundPlay[State::STATE_ATTACK3] = false;
			m_IsEffectPlay[State::STATE_ATTACK3] = false;

			m_pObject->GetAnimateMachine()->SetAnimState(STATE_IDLE);
			CNetwork::GetInstance()->SendStopPacket();
		}

	}

	if (bTimerDead == true)
	{
		//cout << m_fAnimationKeyFrameIndex_Dead << endl;
		if (m_bIsLife == true)
			m_fAnimationKeyFrameIndex_Dead += gt.DeltaTime() * 20;
		//m_iCurAnimFrame = m_fAnimationKeyFrameIndex_Attack3;

		if (m_fAnimationKeyFrameIndex_Dead + 1 > (*vecAnimFrame)[5])
		{
			m_bIsLife = false;
			bTimerDead = false;
			//m_fAnimationKeyFrameIndex_Dead = 0;
		}

	}
	if (bTimerUltimate == true)
	{

		auto * m_pPlayer = CManagement::GetInstance()->Find_Object(L"Layer_Player");

		m_fAnimationKeyFrameIndex_Ultimate += gt.DeltaTime() * 20;
		//m_iCurAnimFrame = m_fAnimationKeyFrameIndex_Attack2;

		if (!m_IsSoundPlay[State::STATE_ULTIMATE] && m_fAnimationKeyFrameIndex_Ultimate > m_SoundFrame[State::STATE_ULTIMATE])
		{
			m_IsSoundPlay[State::STATE_ULTIMATE] = true;
			CManagement::GetInstance()->GetSound()->PlayEffect(L"Sound", L"Attack");
			//CManagement::GetInstance()->GetSound()->PlayEffect(m_pMachineName, m_pStateName[State::STATE_ATTACK2]);
		}

		if (!m_IsEffectPlay[State::STATE_ULTIMATE] && m_fAnimationKeyFrameIndex_Ultimate > m_EffectFrame[State::STATE_ULTIMATE])
		{
			m_IsEffectPlay[State::STATE_ULTIMATE] = true;
			CEffect_Manager::GetInstance()->Play_SkillEffect("Drop", &m_pObject->GetWorld());
		}

		if (m_fAnimationKeyFrameIndex_Ultimate > (*vecAnimFrame)[6])
		{
			bTimerUltimate = false;
			m_fAnimationKeyFrameIndex_Ultimate = 0;
			
			m_IsSoundPlay[State::STATE_ULTIMATE] = false;
			m_IsEffectPlay[State::STATE_ULTIMATE] = false;

			m_pObject->GetAnimateMachine()->SetAnimState(STATE_IDLE);
			CNetwork::GetInstance()->SendStopPacket();
		}


	}

	if (bTimerRoll == true)
	{

		auto * m_pPlayer = CManagement::GetInstance()->Find_Object(L"Layer_Player");

		//m_pPlayer->MoveForward(10.0f);
		m_fAnimationKeyFrameIndex_Roll+= gt.DeltaTime() * 30;
		//m_iCurAnimFrame = m_fAnimationKeyFrameIndex_Attack3;

		if (!m_IsSoundPlay[State::STATE_ROLL] && m_fAnimationKeyFrameIndex_Roll > m_SoundFrame[State::STATE_ROLL])
		{
			m_IsSoundPlay[State::STATE_ROLL] = true;
			//CManagement::GetInstance()->GetSound()->PlayEffect(m_pMachineName, m_pStateName[State::STATE_ATTACK3]);		// > ��� ���尡 ������ �̷��� �ٲ����!
			CManagement::GetInstance()->GetSound()->PlayEffect(L"Sound", L"Attack");
		}

		if (!m_IsEffectPlay[State::STATE_ROLL] && m_fAnimationKeyFrameIndex_Roll > m_EffectFrame[State::STATE_ROLL])
		{
			m_IsEffectPlay[State::STATE_ROLL] = true;
			// > ��ų�־��ֱ�
			//CEffect_Manager::GetInstance()->Play_SkillEffect("��ų�̸�");
			CEffect_Manager::GetInstance()->Play_SkillEffect("hh", &m_pObject->GetWorld());
		}

		if (m_fAnimationKeyFrameIndex_Roll > (*vecAnimFrame)[7])
		{
			bTimerRoll= false;
			m_fAnimationKeyFrameIndex_Roll = 0;

			m_IsSoundPlay[State::STATE_ROLL] = false;
			m_IsEffectPlay[State::STATE_ROLL] = false;

			m_pObject->GetAnimateMachine()->SetAnimState(STATE_IDLE);
			CNetwork::GetInstance()->SendStopPacket();
		}

	}

}

AnimateStateMachine_Player * AnimateStateMachine_Player::Create(CGameObject* pObj, wchar_t * pMachineName, int SoundFrame[State::STATE_END], int EffectFrame[State::STATE_END])
{
	AnimateStateMachine_Player* pInstance = new AnimateStateMachine_Player(pObj, pMachineName, SoundFrame, EffectFrame);

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX(L"AnimateStateMachine_Player Created Failed");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void AnimateStateMachine_Player::Free()
{
}
