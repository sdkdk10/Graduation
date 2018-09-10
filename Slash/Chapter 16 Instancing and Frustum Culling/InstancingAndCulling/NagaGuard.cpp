#include "stdafx.h"
#include "NagaGuard.h"
#include "Define.h"
#include "DynamicMeshSingle.h"
#include "Camera.h"
#include "Management.h"
#include "Renderer.h"
#include "Effect_Manager.h"
#include "Component_Manager.h"
#include "Texture_Manager.h"
#include "Player.h"
#include "Layer.h"


NagaGuard::NagaGuard(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice, ComPtr<ID3D12DescriptorHeap> &srv, UINT srvSize)
	: CGameObject(d3dDevice, srv, srvSize)
{
	m_fMoveSpeed = 2.0f;

	m_fRotateSpeed = 4.0f;
}


NagaGuard::~NagaGuard()
{
}

bool NagaGuard::Update(const GameTimer & gt)
{



	if (!m_pPlayer)
		m_pPlayer = dynamic_cast<Player*>(CManagement::GetInstance()->Find_Object(L"Layer_Player"));

	CGameObject::Update(gt);




	m_pCamera = CManagement::GetInstance()->Get_MainCam();
	XMMATRIX view = m_pCamera->GetView();
	XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);



	//auto currObjectCB2 = m_pFrameResource->InstanceBuffer.get();

	XMMATRIX world = XMLoadFloat4x4(&World);
	XMMATRIX texTransform = XMLoadFloat4x4(&TexTransform);

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
		//cout << "보인당!" << endl;
		m_bIsVisiable = true;
		Animate(gt);

		auto currObjectCB = m_pFrameResource->ObjectCB.get();

		ObjectConstants objConstants;
		XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(world));
		XMStoreFloat4x4(&objConstants.TexTransform, XMMatrixTranspose(texTransform));
		objConstants.MaterialIndex = Mat->MatCBIndex;

		currObjectCB->CopyData(ObjCBIndex, objConstants);

		auto currMaterialCB = m_pFrameResource->MaterialCB.get();

		XMMATRIX matTransform = XMLoadFloat4x4(&Mat->MatTransform);

		MaterialConstants matConstants;
		matConstants.DiffuseAlbedo = Mat->DiffuseAlbedo;
		matConstants.FresnelR0 = Mat->FresnelR0;
		matConstants.Roughness = Mat->Roughness;
		XMStoreFloat4x4(&matConstants.MatTransform, XMMatrixTranspose(matTransform));

		matConstants.DiffuseMapIndex = Mat->DiffuseSrvHeapIndex;

		currMaterialCB->CopyData(Mat->MatCBIndex, matConstants);
	}
	else
	{
		//cout << "안보인당!" << endl;
		m_bIsVisiable = false;
	}

	CManagement::GetInstance()->GetRenderer()->Add_RenderGroup(CRenderer::RENDER_NONALPHA_FORWARD, this);
	return true;
}

void NagaGuard::Render(ID3D12GraphicsCommandList * cmdList)
{
	if (m_bIsVisiable && m_bIsConnected)
	{
		AnimStateMachine->SetTimerTrueFalse();

		UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
		UINT matCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(MaterialConstants));

		auto objectCB = m_pFrameResource->ObjectCB->Resource();
		auto matCB = m_pFrameResource->MaterialCB->Resource();

		cmdList->IASetVertexBuffers(0, 1, &Geo->VertexBufferView());
		cmdList->IASetIndexBuffer(&Geo->IndexBufferView());
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

		//	auto indexcnt = dynamic_cast<DynamicMesh*>(m_pMesh)->Indexoffset[0];

		auto pMesh = dynamic_cast<DynamicMeshSingle*>(m_pMesh);


		//int iTest = (int)pMesh->m_fTest;

		int iTest = AnimStateMachine->GetCurAnimFrame();
		int AnimaState = AnimStateMachine->GetAnimState();

		cmdList->DrawIndexedInstanced(pMesh->Indexoffset[1], 1,
			pMesh->Indexoffset[iTest] + pMesh->IndexAnimoffset[AnimaState] /*+ pMesh->IndexAnimoffset[0]*/,
			pMesh->Vertexoffset[iTest] + pMesh->VertexAnimoffset[AnimaState]/*+ pMesh->VertexAnimoffset[0]*/, 0);


		//if (m_bLODState == true)
		//{
		//	cmdList->DrawIndexedInstanced(1200, 1,
		//		0 /*+ pMesh->IndexAnimoffset[0]*/,
		//		0/*+ pMesh->VertexAnimoffset[0]*/, 0);
		//}
		//else
		//{
		//	cmdList->DrawIndexedInstanced(pMesh->Indexoffset[1], 1,
		//		pMesh->Indexoffset[iTest] + pMesh->IndexAnimoffset[AnimaState] /*+ pMesh->IndexAnimoffset[0]*/,
		//		pMesh->Vertexoffset[iTest] + pMesh->VertexAnimoffset[AnimaState]/*+ pMesh->VertexAnimoffset[0]*/, 0);

		//}



	}


}

HRESULT NagaGuard::Initialize()
{
	m_pMesh = dynamic_cast<DynamicMeshSingle*>(CComponent_Manager::GetInstance()->Clone_Component(L"Com_Mesh_NagaGuard"));
	if (nullptr == m_pMesh)
		return E_FAIL;



	Texture* tex = CTexture_Manager::GetInstance()->Find_Texture("NagaGuardTex", CTexture_Manager::TEX_DEFAULT_2D);
	if (nullptr == tex)
		return E_FAIL;

	m_strTexName[SPIDER_BRICK] = "bricksTex";
	m_strTexName[SPIDER_STONE] = "stoneTex";
	m_strTexName[SPIDER_TILE] = "tileTex";
	m_strTexName[SPIDER_ICE] = "iceTex";


	AnimStateMachine = new AnimateStateMachine;

	AnimStateMachine->vecAnimFrame = &(dynamic_cast<DynamicMeshSingle*>(m_pMesh)->vecAnimFrame);

	AnimStateMachine->SetAnimState(State::STATE_IDLE);

	Mat = new Material;
	Mat->Name = "SpiderMat";
	Mat->MatCBIndex = m_iMyObjectID;
	Mat->DiffuseSrvHeapIndex = tex->Num;
	Mat->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	Mat->FresnelR0 = XMFLOAT3(0.05f, 0.05f, 0.05f);
	Mat->Roughness = 0.3f;

	XMStoreFloat4x4(&World, XMMatrixScaling(m_fScale, m_fScale, m_fScale)*XMMatrixRotationX(1.7f)*XMMatrixRotationZ(3.14f)*XMMatrixTranslation(0.0f, 0.0f, 0.0f));


	//XMStoreFloat4x4(&World, XMMatrixScaling(m_fScale, m_fScale, m_fScale)*XMMatrixTranslation(0.0f, 0.0f, 0.0f)*XMMatrixTranslation(0.0f, 0.0f, 0.0f));
	TexTransform = MathHelper::Identity4x4();
	ObjCBIndex = m_iMyObjectID;

	Geo = dynamic_cast<DynamicMeshSingle*>(m_pMesh)->m_Geometry[0].get();
	PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	IndexCount = Geo->DrawArgs["SingleMesh"].IndexCount;
	StartIndexLocation = Geo->DrawArgs["SingleMesh"].StartIndexLocation;
	BaseVertexLocation = Geo->DrawArgs["SingleMesh"].BaseVertexLocation;
	Bounds = Geo->DrawArgs["SingleMesh"].Bounds;



	m_xmf3Scale = XMFLOAT3(0.08f, 0.08f, 0.08f);
	m_xmf3Rot = XMFLOAT3(1.7f, 0.f, 3.14f);

	return S_OK;
}

void NagaGuard::Animate(const GameTimer & gt)
{
	AnimStateMachine->AnimationStateUpdate(gt);

}

void NagaGuard::SaveSlidingVector(CGameObject * pobj, CGameObject * pCollobj)
{


	//XMFLOAT3 pCollObjPos2= pCollobj->m_pTransCom->m_f3Position;
	XMFLOAT3 Center = pCollobj->GetPosition();
	XMFLOAT3 Player = pobj->GetPosition();
	XMFLOAT3 dirVector = Vector3::Subtract(Player, Center);   // 충돌 객체에서 플레이어로 가는 벡터

	float tanceta = dirVector.z / dirVector.x;
	float ceta = atan(tanceta) * 57.3248f;

	float extenttanceta = pCollobj->m_xmOOBB.Extents.z / pCollobj->m_xmOOBB.Extents.x;


	float extentceta = atan(extenttanceta) * 57.3248f;


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
		pobj->m_MovingRefletVector = XMFLOAT3(0, 0, -1);

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
}

//void NagaGuard::SetTexture(SpiderTex _tex)
//{
//	Texture* tex = CTexture_Manager::GetInstance()->Find_Texture(m_strTexName[_tex], CTexture_Manager::TEX_DEFAULT_2D);
//	if (nullptr == tex)
//		return;
//	Mat->DiffuseSrvHeapIndex = tex->Num;
//
//}

NagaGuard * NagaGuard::Create(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice, ComPtr<ID3D12DescriptorHeap>& srv, UINT srvSize)
{
	NagaGuard* pInstance = new NagaGuard(d3dDevice, srv, srvSize);

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX(L"Player Created Failed");
		Safe_Release(pInstance);
	}

	return pInstance;
}


// > ---------------------------- StateMachine_NagaGuard -----------------------------------
//void AnimateStateMachine_NagaGuard::AnimationStateUpdate(const GameTimer & gt)
//{
//	if (bTimerIdle == true)
//	{
//		m_fAnimationKeyFrameIndex += gt.DeltaTime() * 25;
//		//m_iCurAnimFrame = m_fAnimationKeyFrameIndex;
//		if (m_fAnimationKeyFrameIndex > (*vecAnimFrame)[0])
//		{
//			bTimerIdle = false;
//
//			m_fAnimationKeyFrameIndex = 0;
//		}
//	}
//
//
//	if (bTimerWalk == true)
//	{
//		m_fAnimationKeyFrameIndex_Walk += gt.DeltaTime() * 45;
//		//m_iCurAnimFrame = m_fAnimationKeyFrameIndex_Walk;
//		if (m_fAnimationKeyFrameIndex_Walk > (*vecAnimFrame)[1])
//		{
//			bTimerWalk = false;
//			m_fAnimationKeyFrameIndex_Walk = 0;
//		}
//
//	}
//
//
//	if (bTimerAttack1 == true)
//	{
//
//		m_fAnimationKeyFrameIndex_Attack1 += gt.DeltaTime() * 20;
//		//m_iCurAnimFrame = m_fAnimationKeyFrameIndex_Attack1;
//		if (!m_IsSoundPlay[State::STATE_ATTACK1] && m_fAnimationKeyFrameIndex_Attack1 > m_SoundFrame[State::STATE_ATTACK1])
//		{
//			m_IsSoundPlay[State::STATE_ATTACK1] = true;
//			CManagement::GetInstance()->GetSound()->PlayEffect(L"Sound", L"Attack");
//			//CManagement::GetInstance()->GetSound()->PlayEffect(m_pMachineName, m_pStateName[State::STATE_ATTACK1]);
//		}
//
//		if (!m_IsEffectPlay[State::STATE_ATTACK1] && m_fAnimationKeyFrameIndex_Attack1 > m_EffectFrame[State::STATE_ATTACK1])
//		{
//			m_IsEffectPlay[State::STATE_ATTACK1] = true;
//			// > 스킬넣어주기
//			//CEffect_Manager::GetInstance()->Play_SkillEffect("스킬이름");
//			//cout << "스킬!" << endl;
//			CEffect_Manager::GetInstance()->Play_SkillEffect("Warrior_Turn", &m_pObject->GetWorld());
//			//cout << "Player Pos : " << m_pObject->GetPosition().x << ", " << m_pObject->GetPosition().y << ", " << m_pObject->GetPosition().z << endl;
//		}
//
//		if (m_fAnimationKeyFrameIndex_Attack1 > (*vecAnimFrame)[2])
//		{
//			bTimerAttack1 = false;
//			m_fAnimationKeyFrameIndex_Attack1 = 0.f;
//
//			m_IsSoundPlay[State::STATE_ATTACK1] = false;
//			m_IsEffectPlay[State::STATE_ATTACK1] = false;
//
//			m_pObject->GetAnimateMachine()->SetAnimState(STATE_IDLE);
//			CNetwork::GetInstance()->SendStopPacket();
//		}
//
//	}
//
//
//	if (bTimerAttack2 == true)
//	{
//
//		m_fAnimationKeyFrameIndex_Attack2 += gt.DeltaTime() * 30;
//		//m_iCurAnimFrame = m_fAnimationKeyFrameIndex_Attack2;
//
//		if (!m_IsSoundPlay[State::STATE_ATTACK2] && m_fAnimationKeyFrameIndex_Attack2 > m_SoundFrame[State::STATE_ATTACK2])
//		{
//			m_IsSoundPlay[State::STATE_ATTACK2] = true;
//			CManagement::GetInstance()->GetSound()->PlayEffect(L"Sound", L"Attack");
//			//CManagement::GetInstance()->GetSound()->PlayEffect(m_pMachineName, m_pStateName[State::STATE_ATTACK2]);
//		}
//
//		if (!m_IsEffectPlay[State::STATE_ATTACK2] && m_fAnimationKeyFrameIndex_Attack2 > m_EffectFrame[State::STATE_ATTACK2])
//		{
//			m_IsEffectPlay[State::STATE_ATTACK2] = true;
//			// > 스킬넣어주기
//			//CEffect_Manager::GetInstance()->Play_SkillEffect("스킬이름");
//			CEffect_Manager::GetInstance()->Play_SkillEffect("orbAttack", &m_pObject->GetWorld());
//		}
//
//		if (m_fAnimationKeyFrameIndex_Attack2 > (*vecAnimFrame)[3])
//		{
//			bTimerAttack2 = false;
//			m_fAnimationKeyFrameIndex_Attack2 = 0;
//
//			m_IsSoundPlay[State::STATE_ATTACK2] = false;
//			m_IsEffectPlay[State::STATE_ATTACK2] = false;
//
//			m_pObject->GetAnimateMachine()->SetAnimState(STATE_IDLE);
//			CNetwork::GetInstance()->SendStopPacket();
//		}
//
//
//	}
//
//	if (bTimerAttack3 == true)
//	{
//
//		m_fAnimationKeyFrameIndex_Attack3 += gt.DeltaTime() * 30;
//		//m_iCurAnimFrame = m_fAnimationKeyFrameIndex_Attack3;
//
//		if (!m_IsSoundPlay[State::STATE_ATTACK3] && m_fAnimationKeyFrameIndex_Attack3 > m_SoundFrame[State::STATE_ATTACK3])
//		{
//			m_IsSoundPlay[State::STATE_ATTACK3] = true;
//			//CManagement::GetInstance()->GetSound()->PlayEffect(m_pMachineName, m_pStateName[State::STATE_ATTACK3]);		// > 모든 사운드가 들어갔을때 이렇게 바꿔야함!
//			CManagement::GetInstance()->GetSound()->PlayEffect(L"Sound", L"Attack");
//		}
//
//		if (!m_IsEffectPlay[State::STATE_ATTACK3] && m_fAnimationKeyFrameIndex_Attack3 > m_EffectFrame[State::STATE_ATTACK3])
//		{
//			m_IsEffectPlay[State::STATE_ATTACK3] = true;
//			// > 스킬넣어주기
//			//CEffect_Manager::GetInstance()->Play_SkillEffect("스킬이름");
//			CEffect_Manager::GetInstance()->Play_SkillEffect("hh", &m_pObject->GetWorld());
//		}
//
//		if (m_fAnimationKeyFrameIndex_Attack3 > (*vecAnimFrame)[4])
//		{
//			bTimerAttack3 = false;
//			m_fAnimationKeyFrameIndex_Attack3 = 0;
//
//			m_IsSoundPlay[State::STATE_ATTACK3] = false;
//			m_IsEffectPlay[State::STATE_ATTACK3] = false;
//
//			m_pObject->GetAnimateMachine()->SetAnimState(STATE_IDLE);
//			CNetwork::GetInstance()->SendStopPacket();
//		}
//
//	}
//
//	if (bTimerDead == true)
//	{
//		//cout << m_fAnimationKeyFrameIndex_Dead << endl;
//		if (m_bIsLife == true)
//			m_fAnimationKeyFrameIndex_Dead += gt.DeltaTime() * 20;
//		//m_iCurAnimFrame = m_fAnimationKeyFrameIndex_Attack3;
//
//		if (m_fAnimationKeyFrameIndex_Dead + 1 > (*vecAnimFrame)[5])
//		{
//			m_bIsLife = false;
//			bTimerDead = false;
//			//m_fAnimationKeyFrameIndex_Dead = 0;
//		}
//
//	}
//	if (bTimerUltimate == true)
//	{
//
//		auto * m_pPlayer = CManagement::GetInstance()->Find_Object(L"Layer_Player");
//
//		m_fAnimationKeyFrameIndex_Ultimate += gt.DeltaTime() * 20;
//		//m_iCurAnimFrame = m_fAnimationKeyFrameIndex_Attack2;
//
//		if (!m_IsSoundPlay[State::STATE_ULTIMATE] && m_fAnimationKeyFrameIndex_Ultimate > m_SoundFrame[State::STATE_ULTIMATE])
//		{
//			m_IsSoundPlay[State::STATE_ULTIMATE] = true;
//			CManagement::GetInstance()->GetSound()->PlayEffect(L"Sound", L"Attack");
//			//CManagement::GetInstance()->GetSound()->PlayEffect(m_pMachineName, m_pStateName[State::STATE_ATTACK2]);
//		}
//
//		if (!m_IsEffectPlay[State::STATE_ULTIMATE] && m_fAnimationKeyFrameIndex_Ultimate > m_EffectFrame[State::STATE_ULTIMATE])
//		{
//			m_IsEffectPlay[State::STATE_ULTIMATE] = true;
//			CEffect_Manager::GetInstance()->Play_SkillEffect("Drop", &m_pObject->GetWorld());
//		}
//
//		if (m_fAnimationKeyFrameIndex_Ultimate > (*vecAnimFrame)[6])
//		{
//			bTimerUltimate = false;
//			m_fAnimationKeyFrameIndex_Ultimate = 0;
//
//			m_IsSoundPlay[State::STATE_ULTIMATE] = false;
//			m_IsEffectPlay[State::STATE_ULTIMATE] = false;
//
//			m_pObject->GetAnimateMachine()->SetAnimState(STATE_IDLE);
//			CNetwork::GetInstance()->SendStopPacket();
//		}
//
//
//	}
//
//	if (bTimerRoll == true)
//	{
//
//		auto * m_pPlayer = CManagement::GetInstance()->Find_Object(L"Layer_Player");
//
//		//m_pPlayer->MoveForward(10.0f);
//		m_fAnimationKeyFrameIndex_Roll += gt.DeltaTime() * 30;
//		//m_iCurAnimFrame = m_fAnimationKeyFrameIndex_Attack3;
//
//		if (!m_IsSoundPlay[State::STATE_ROLL] && m_fAnimationKeyFrameIndex_Roll > m_SoundFrame[State::STATE_ROLL])
//		{
//			m_IsSoundPlay[State::STATE_ROLL] = true;
//			//CManagement::GetInstance()->GetSound()->PlayEffect(m_pMachineName, m_pStateName[State::STATE_ATTACK3]);		// > 모든 사운드가 들어갔을때 이렇게 바꿔야함!
//			CManagement::GetInstance()->GetSound()->PlayEffect(L"Sound", L"Attack");
//		}
//
//		if (!m_IsEffectPlay[State::STATE_ROLL] && m_fAnimationKeyFrameIndex_Roll > m_EffectFrame[State::STATE_ROLL])
//		{
//			m_IsEffectPlay[State::STATE_ROLL] = true;
//			// > 스킬넣어주기
//			//CEffect_Manager::GetInstance()->Play_SkillEffect("스킬이름");
//			CEffect_Manager::GetInstance()->Play_SkillEffect("hh", &m_pObject->GetWorld());
//		}
//
//		if (m_fAnimationKeyFrameIndex_Roll > (*vecAnimFrame)[7])
//		{
//			bTimerRoll = false;
//			m_fAnimationKeyFrameIndex_Roll = 0;
//
//			m_IsSoundPlay[State::STATE_ROLL] = false;
//			m_IsEffectPlay[State::STATE_ROLL] = false;
//
//			m_pObject->GetAnimateMachine()->SetAnimState(STATE_IDLE);
//			CNetwork::GetInstance()->SendStopPacket();
//		}
//
//	}
//
//}
//
//AnimateStateMachine_NagaGuard * AnimateStateMachine_NagaGuard::Create(CGameObject* pObj, wchar_t * pMachineName, int SoundFrame[State::STATE_END], int EffectFrame[State::STATE_END])
//{
//	AnimateStateMachine_NagaGuard* pInstance = new AnimateStateMachine_NagaGuard(pObj, pMachineName, SoundFrame, EffectFrame);
//
//	if (FAILED(pInstance->Initialize()))
//	{
//		MSG_BOX(L"AnimateStateMachine_NagaGuard Created Failed");
//		Safe_Release(pInstance);
//	}
//
//	return pInstance;
//}
//
//void AnimateStateMachine_NagaGuard::Free()
//{
//}
