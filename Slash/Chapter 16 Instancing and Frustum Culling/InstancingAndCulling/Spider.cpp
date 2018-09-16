#include "stdafx.h"
#include "Spider.h"
#include "Define.h"
#include "DynamicMeshSingle.h"
#include "Camera.h"
#include "Management.h"
#include "Renderer.h"
#include "Component_Manager.h"
#include "Texture_Manager.h"
#include "Player.h"
#include "Layer.h"
#include "Effect_Manager.h"


Spider::Spider(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice, ComPtr<ID3D12DescriptorHeap> &srv, UINT srvSize)
	: CGameObject(d3dDevice, srv, srvSize)
{
	m_fMoveSpeed = 2.0f;

	m_fRotateSpeed = 4.0f;
}


Spider::~Spider()
{
}

bool Spider::Update(const GameTimer & gt)
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
		//cout << "���δ�!" << endl;
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
		//cout << "�Ⱥ��δ�!" << endl;
		m_bIsVisiable = false;
	}


	/*cout << "World : " << World._41 << "\t" << World._42 << "\t" << World._43 << endl;
	cout << "Bounds Center: " << Bounds.Center.x << "\t" << Bounds.Center.y << "\t" << Bounds.Center.z << endl;*/


	//////////////////////////////////////////////////


	// Next FrameResource need to be updated too.
	//NumFramesDirty--;
	CManagement::GetInstance()->GetRenderer()->Add_RenderGroup(CRenderer::RENDER_NONALPHA_FORWARD, this);
	return true;
}

void Spider::Render(ID3D12GraphicsCommandList * cmdList)
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


		if (m_bLODState == true)
		{
			cmdList->DrawIndexedInstanced(1200, 1,
				0 /*+ pMesh->IndexAnimoffset[0]*/,
				0/*+ pMesh->VertexAnimoffset[0]*/, 0);
		}
		else
		{
			cmdList->DrawIndexedInstanced(pMesh->Indexoffset[1], 1,
				pMesh->Indexoffset[iTest] + pMesh->IndexAnimoffset[AnimaState] /*+ pMesh->IndexAnimoffset[0]*/,
				pMesh->Vertexoffset[iTest] + pMesh->VertexAnimoffset[AnimaState]/*+ pMesh->VertexAnimoffset[0]*/, 0);

		}



	}


}

HRESULT Spider::Initialize()
{
	m_pMesh = dynamic_cast<DynamicMeshSingle*>(CComponent_Manager::GetInstance()->Clone_Component(L"Com_Mesh_Spider"));
	if (nullptr == m_pMesh)
		return E_FAIL;



	Texture* tex = CTexture_Manager::GetInstance()->Find_Texture("SpiderTex", CTexture_Manager::TEX_DEFAULT_2D);
	if (nullptr == tex)
		return E_FAIL;

	m_strTexName[SPIDER_BRICK] = "bricksTex";
	m_strTexName[SPIDER_STONE] = "stoneTex";
	m_strTexName[SPIDER_TILE] = "tileTex";
	m_strTexName[SPIDER_ICE] = "iceTex";



	wchar_t* machineName;
	machineName = L"Spider";
	int test[MonsterState::MSTATE_END] = { 0, };
	AnimStateMachine = AnimateStateMachine_Spider::Create(this, machineName, test, test);
	if (AnimStateMachine == nullptr)
		return E_FAIL;

	AnimStateMachine->vecAnimFrame = &(dynamic_cast<DynamicMeshSingle*>(m_pMesh)->vecAnimFrame);

	AnimStateMachine->SetAnimState(State::STATE_IDLE);

	Mat = new Material;
	Mat->Name = "SpiderMat";
	Mat->MatCBIndex = m_iMyObjectID;
	Mat->DiffuseSrvHeapIndex = tex->Num;
	Mat->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	Mat->FresnelR0 = XMFLOAT3(0.05f, 0.05f, 0.05f);
	Mat->Roughness = 0.3f;

	XMStoreFloat4x4(&World, XMMatrixScaling(m_fScale, m_fScale, m_fScale)*XMMatrixTranslation(0.0f, 0.0f, 0.0f)*XMMatrixTranslation(0.0f, 0.0f, 0.0f));
	TexTransform = MathHelper::Identity4x4();
	ObjCBIndex = m_iMyObjectID;

	Geo = dynamic_cast<DynamicMeshSingle*>(m_pMesh)->m_Geometry[0].get();
	PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	IndexCount = Geo->DrawArgs["SingleMesh"].IndexCount;
	StartIndexLocation = Geo->DrawArgs["SingleMesh"].StartIndexLocation;
	BaseVertexLocation = Geo->DrawArgs["SingleMesh"].BaseVertexLocation;
	Bounds = Geo->DrawArgs["SingleMesh"].Bounds;



	//SetOOBB(XMFLOAT3(Bounds.Center.x *m_fScale, Bounds.Center.y * m_fScale, Bounds.Center.z *m_fScale), XMFLOAT3(Bounds.Extents.x * m_fScale, Bounds.Extents.y * m_fScale, Bounds.Extents.z * m_fScale), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));

	//SetOOBB(XMFLOAT3(Bounds.Center.x , Bounds.Center.y , Bounds.Center.z ), XMFLOAT3(Bounds.Extents.x, Bounds.Extents.y, Bounds.Extents.z), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));

	m_xmf3Scale = XMFLOAT3(2.0f, 2.0f, 2.0f);
	m_xmf3Rot = XMFLOAT3(0.0f, 0.0f, 0.0f);

	return S_OK;
}

void Spider::Animate(const GameTimer & gt)
{
	//if (Vector3::BetweenVectorLength(m_pPlayer->GetPosition(), GetPosition()) > 500.0f)
	//{
	//	m_bLODState = true;


	//	return;
	//}
	//m_bLODState = false;

	//if (Vector3::BetweenVectorLength(m_pPlayer->GetPosition(), GetPosition()) > 100.0f)
	//{
	//	m_xmOOBBTransformed.Transform(m_xmOOBB, XMLoadFloat4x4(&(GetWorld())));
	//	XMStoreFloat4(&m_xmOOBBTransformed.Orientation, XMQuaternionNormalize(XMLoadFloat4(&m_xmOOBBTransformed.Orientation)));

	//	return;
	//}
	//if (GetHp() < 0)
	//{
	//	SetObjectAnimState(State::STATE_DEAD);
	//	AnimStateMachine->AnimationStateUpdate(gt);
	//	return;
	//}

	//XMFLOAT3 playerPos = m_pPlayer->GetPosition();
	//XMFLOAT3 Shaft = GetLook();

	////	cout << Shaft.x << "\t" << Shaft.y << "\t" << Shaft.z << endl;

	//XMFLOAT3 dirVector = Vector3::Subtract(playerPos, GetPosition());   // ��ü���� �÷��̾�� ���� ����

	//dirVector = Vector3::Normalize(dirVector);

	//XMFLOAT3 crossVector = Vector3::CrossProduct(Shaft, dirVector, true);

	//float dotproduct = Vector3::DotProduct(Shaft, dirVector);
	//float ShafttLength = Vector3::Length(Shaft);
	//float dirVectorLength = Vector3::Length(dirVector);

	//float cosCeta = (dotproduct / ShafttLength * dirVectorLength);

	//float ceta = acos(cosCeta);

	//if (playerPos.z < GetPosition().z)
	//	ceta = 360.f - ceta * 57.3248f;// +180.0f;
	//else
	//	ceta = ceta * 57.3248f;

	////cout << ceta << endl;
	////float a = Vector3::Length(dirVector);
	////XMFLOAT3 Normal = XMFLOAT3(a * cos(ceta), 0, a * sin(ceta));
	////Normal = Vector3::Normalize(Normal);

	//////�ٸ��ֵ� ã��
	////auto pLayer = CManagement::GetInstance()->Get_Layer(L"Layer_Monster");
	////if (pLayer != nullptr)
	////{
	////	auto objList = pLayer->Get_ObjectList();
	////	size_t iSize = objList.size();
	////	for (size_t i = 0; i < iSize; ++i)
	////	{
	////		if (objList[i] == this)
	////			continue;

	////	}
	////}



	//if (m_pPlayer->m_xmOOBB.Contains(this->m_xmOOBB))
	//{
	//	//cout << "�Ź̶� �浹" << endl;


	//}

	//auto pLayer = CManagement::GetInstance()->Get_Layer(L"Layer_Monster");
	//if (pLayer != nullptr)
	//{
	//	auto objList = pLayer->Get_ObjectList();
	//	size_t iSize = objList.size();
	//	for (size_t i = 0; i < iSize; ++i)
	//	{

	//		/*	float MinX = objList[i]->m_xmOOBB.Center.x - m_xmOOBB.Extents.x;
	//		float MaxX = objList[i]->m_xmOOBB.Center.x + m_xmOOBB.Extents.x;

	//		float MinZ = objList[i]->m_xmOOBB.Center.z - m_xmOOBB.Extents.z;
	//		float MaxZ = objList[i]->m_xmOOBB.Center.z + m_xmOOBB.Extents.z;*/

	//		if (objList[i] == this)
	//			continue;

	//		if (objList[i]->m_xmOOBB.Contains(m_xmOOBB))
	//		{
	//			objList[i]->m_pCollider = this;
	//			this->m_pCollider = objList[i];


	//		}

	//	}
	//}




	AnimStateMachine->AnimationStateUpdate(gt);

	//if (Vector3::BetweenVectorLength(m_pPlayer->GetPosition(), GetPosition()) < 15.0f)
	//{

	//	if (m_pPlayer->m_xmOOBB.Contains(m_xmOOBB)) //�浹�� ������ ����� ������
	//	{
	//		if (m_pPlayer->GetAnimateMachine()->GetAnimState() == m_pPlayer->GetAnimateMachine()->State::STATE_ATTACK1)
	//		{
	//			if (m_pPlayer->GetAnimateMachine()->GetCurAnimFrame() == 8)
	//			{
	//				SetHp(-100);
	//				CManagement::GetInstance()->GetSound()->PlayEffect(L"Sound", L"Hit");
	//			}

	//		}
	//		//cout << "�Ź� �浹 " << endl;

	//		m_pPlayer->m_pCollider = this;
	//		//SetObjectAnimState(State::STATE_ATTACK1);
	//		SetObjectAnimState(State::STATE_ATTACK1);

	//		//cout << m_pPlayer->GetHp() - 1.0f << endl;
	//		if (AnimStateMachine->GetCurAnimFrame() == 13)
	//			m_pPlayer->SetHp(m_pPlayer->GetHp() - 1.0f);

	//	}
	//	else //�浹�� �������� �÷��̾����� �̵�
	//	{

	//		if (crossVector.y > 0)
	//		{
	//			if (ceta > 0.1f)
	//			{
	//				Rotate(0.0f, m_fRotateSpeed * gt.DeltaTime() * 10.0f, 0.0f);

	//			}
	//			//cout << "�ð��" << endl;

	//		}
	//		if (crossVector.y < 0)
	//		{
	//			if (ceta > 0.1f)
	//			{
	//				Rotate(0.0f, -m_fRotateSpeed * gt.DeltaTime()*10.0f, 0.0f);


	//			}
	//			//cout << "�ݽð��" << endl;

	//		}
	//		XMFLOAT3 moveingVector = XMFLOAT3(dirVector.x * gt.DeltaTime() *m_fMoveSpeed, dirVector.y * gt.DeltaTime() *m_fMoveSpeed, dirVector.z * gt.DeltaTime() *m_fMoveSpeed);

	//		moveingVector = Vector3::Subtract(moveingVector, Vector3::MultiplyScalr(m_MovingRefletVector, Vector3::DotProduct(moveingVector, m_MovingRefletVector)));


	//		Move(XMFLOAT3(moveingVector.x, moveingVector.y, moveingVector.z), true);

	//		//cout << "�̵�����" << endl;
	//		SetObjectAnimState(State::STATE_WALK);

	//	}

	//}
	//else
	//{
	//	/*m_bIsCollide = false;

	//	if (coll == false)
	//	{
	//	m_pPlayer->m_MovingRefletVector = XMFLOAT3(0, 0, 0);

	//	}*/
	//	SetObjectAnimState(State::STATE_IDLE);


	//}


	////cout << coll << endl;
	//m_xmOOBBTransformed.Transform(m_xmOOBB, XMLoadFloat4x4(&(GetWorld())));
	//XMStoreFloat4(&m_xmOOBBTransformed.Orientation, XMQuaternionNormalize(XMLoadFloat4(&m_xmOOBBTransformed.Orientation)));


}

void Spider::SaveSlidingVector(CGameObject * pobj, CGameObject * pCollobj)
{


	//XMFLOAT3 pCollObjPos2= pCollobj->m_pTransCom->m_f3Position;
	XMFLOAT3 Center = pCollobj->GetPosition();
	XMFLOAT3 Player = pobj->GetPosition();
	XMFLOAT3 dirVector = Vector3::Subtract(Player, Center);   // �浹 ��ü���� �÷��̾�� ���� ����

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
		//cout << " ������ �浹" << endl;
		pobj->m_MovingRefletVector = XMFLOAT3(1, 0, 0);


	}
	if (Player.z > Center.z && extentceta <ceta && ceta < 90) // 2
	{
		//cout << "���� �浹" << endl;
		pobj->m_MovingRefletVector = XMFLOAT3(0, 0, 1);
	}
	if (Player.z > Center.z && -90 < ceta && ceta < -extentceta) // 3
	{
		//cout << "���� �浹" << endl;
		//pobj->m_MovingRefletVector = XMFLOAT3(-1, 0, 0);
		pobj->m_MovingRefletVector = XMFLOAT3(0, 0, 1);

	}
	if (Player.x < Center.x && -extentceta < ceta && ceta < 0)// 4
	{
		//cout << "���� �浹" << endl;
		//pobj->m_MovingRefletVector = XMFLOAT3(-1, 0, 0);
		pobj->m_MovingRefletVector = XMFLOAT3(-1, 0, 0);

	}
	if (Player.x < Center.x && 0 < ceta && ceta < extentceta) // 5
	{
		//cout << "���� �浹" << endl;
		//pobj->m_MovingRefletVector = XMFLOAT3(-1, 0, 0);
		pobj->m_MovingRefletVector = XMFLOAT3(-1, 0, 0);

	}
	if (Player.z < Center.z && extentceta < ceta && ceta < 90) // 6
	{
		//cout << "�Ʒ��� �浹" << endl;
		//pobj->m_MovingRefletVector = XMFLOAT3(-1, 0, 0);
		pobj->m_MovingRefletVector = XMFLOAT3(0, 0, -1);

	}
	if (Player.z < Center.z && -90 < ceta && ceta < -extentceta) // 7
	{
		//cout << "�Ʒ��� �浹" << endl;
		//pobj->m_MovingRefletVector = XMFLOAT3(-1, 0, 0);
		pobj->m_MovingRefletVector = XMFLOAT3(0, 0, -1);

	}
	if (Player.x > Center.x && -extentceta < ceta && ceta < 0) // 8
	{
		//cout << "������ �浹" << endl;
		//pobj->m_MovingRefletVector = XMFLOAT3(-1, 0, 0);
		pobj->m_MovingRefletVector = XMFLOAT3(1, 0, 0);
	}
}

void Spider::Hit(float fDamage)
{
	CEffect_Manager::GetInstance()->Play_SkillEffect("Hit5", &World);
}

void Spider::SetTexture(SpiderType _tex)
{
	Texture* tex = CTexture_Manager::GetInstance()->Find_Texture(m_strTexName[_tex], CTexture_Manager::TEX_DEFAULT_2D);
	if (nullptr == tex)
		return;
	Mat->DiffuseSrvHeapIndex = tex->Num;
		
}

Spider * Spider::Create(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice, ComPtr<ID3D12DescriptorHeap>& srv, UINT srvSize)
{
	Spider* pInstance = new Spider(d3dDevice, srv, srvSize);

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX(L"Player Created Failed");
		Safe_Release(pInstance);
	}

	return pInstance;
}



// > ---------------------------- StateMachine_Spider-----------------------------------

AnimateStateMachine_Spider::AnimateStateMachine_Spider(CGameObject * pObj, wchar_t * pMachineName, int SoundFrame[State::STATE_END], int EffectFrame[State::STATE_END])
	: m_pMachineName(pMachineName)
	, m_pObject(pObj)
{
}

AnimateStateMachine_Spider::~AnimateStateMachine_Spider()
{
}

HRESULT AnimateStateMachine_Spider::Initialize()
{
	return S_OK;
}

void AnimateStateMachine_Spider::AnimationStateUpdate(const GameTimer & gt)
{
	if (bTimerIdle == true)
	{
		m_fAnimationKeyFrameIndex += gt.DeltaTime() * 25;
		//m_iCurAnimFrame = m_fAnimationKeyFrameIndex;
		if (m_fAnimationKeyFrameIndex > (*vecAnimFrame)[MonsterState::MSTATE_IDLE])
		{
			bTimerIdle = false;

			m_fAnimationKeyFrameIndex = 0;
		}
	}


	if (bTimerWalk == true)
	{
		m_fAnimationKeyFrameIndex_Walk += gt.DeltaTime() * 45;
		//m_iCurAnimFrame = m_fAnimationKeyFrameIndex_Walk;
		if (m_fAnimationKeyFrameIndex_Walk > (*vecAnimFrame)[MonsterState::MSTATE_WALK])
		{
			bTimerWalk = false;
			m_fAnimationKeyFrameIndex_Walk = 0;
		}

	}


	if (bTimerAttack1 == true)
	{

		/*if ((int)m_fAnimationKeyFrameIndex_Attack1 == 1)
		{
			CManagement::GetInstance()->GetSound()->PlayEffect(L"Sound", L"Spider_Attack_Sound");
		}*/

		m_fAnimationKeyFrameIndex_Attack1 += gt.DeltaTime() * 20;
		//m_iCurAnimFrame = m_fAnimationKeyFrameIndex_Attack1;
		if (!m_IsSoundPlay[MonsterState::MSTATE_ATTACK1] && m_fAnimationKeyFrameIndex_Attack1 > m_SoundFrame[MonsterState::MSTATE_ATTACK1])
		{
			m_IsSoundPlay[MonsterState::MSTATE_ATTACK1] = true;
			CManagement::GetInstance()->GetSound()->PlayEffect(L"Sound", L"Spider_Attack_Sound");
		}

		if (!m_IsEffectPlay[MonsterState::MSTATE_ATTACK1] && m_fAnimationKeyFrameIndex_Attack1 > m_EffectFrame[MonsterState::MSTATE_ATTACK1])
		{
			m_IsEffectPlay[MonsterState::MSTATE_ATTACK1] = true;
			// > ��ų�־��ֱ�
			//CEffect_Manager::GetInstance()->Play_SkillEffect("��ų�̸�");
			//cout << "��ų!" << endl;
			CEffect_Manager::GetInstance()->Play_SkillEffect("Warrior_Turn", &m_pObject->GetWorld());
			//cout << "Player Pos : " << m_pObject->GetPosition().x << ", " << m_pObject->GetPosition().y << ", " << m_pObject->GetPosition().z << endl;
		}

		if (m_fAnimationKeyFrameIndex_Attack1 > (*vecAnimFrame)[MonsterState::MSTATE_ATTACK1])
		{
			bTimerAttack1 = false;
			m_fAnimationKeyFrameIndex_Attack1 = 0.f;

			m_IsSoundPlay[MonsterState::MSTATE_ATTACK1] = false;
			m_IsEffectPlay[MonsterState::MSTATE_ATTACK1] = false;

		}

	}


	if (bTimerAttack2 == true)
	{

		m_fAnimationKeyFrameIndex_Attack2 += gt.DeltaTime() * 30;
		//m_iCurAnimFrame = m_fAnimationKeyFrameIndex_Attack2;

		if (!m_IsSoundPlay[MonsterState::MSTATE_ATTACK2] && m_fAnimationKeyFrameIndex_Attack2 > m_SoundFrame[MonsterState::MSTATE_ATTACK2])
		{
			m_IsSoundPlay[MonsterState::MSTATE_ATTACK2] = true;
			CManagement::GetInstance()->GetSound()->PlayEffect(L"Sound", L"Attack");
			//CManagement::GetInstance()->GetSound()->PlayEffect(m_pMachineName, m_pStateName[State::STATE_ATTACK2]);
		}

		if (!m_IsEffectPlay[MonsterState::MSTATE_ATTACK2] && m_fAnimationKeyFrameIndex_Attack2 > m_EffectFrame[MonsterState::MSTATE_ATTACK2])
		{
			m_IsEffectPlay[MonsterState::MSTATE_ATTACK2] = true;
			// > ��ų�־��ֱ�
			//CEffect_Manager::GetInstance()->Play_SkillEffect("��ų�̸�");
			CEffect_Manager::GetInstance()->Play_SkillEffect("orbAttack", &m_pObject->GetWorld());
		}

		if (m_fAnimationKeyFrameIndex_Attack2 > (*vecAnimFrame)[3])
		{
			bTimerAttack2 = false;
			m_fAnimationKeyFrameIndex_Attack2 = 0;

			m_IsSoundPlay[MonsterState::MSTATE_ATTACK2] = false;
			m_IsEffectPlay[MonsterState::MSTATE_ATTACK2] = false;

		}


	}

	if (bTimerHit)
	{

		/*if ((int)m_fAnimationKeyFrameIndex_Hit == 1)
		{
			CManagement::GetInstance()->GetSound()->PlayEffect(L"Sound", L"Spider_Hit_Sound");
		}*/


		auto * m_pPlayer = CManagement::GetInstance()->Find_Object(L"Layer_Player");

		//m_pPlayer->MoveForward(10.0f);
		m_fAnimationKeyFrameIndex_Hit += gt.DeltaTime() * 30;
		//m_iCurAnimFrame = m_fAnimationKeyFrameIndex_Attack3;

		if (!m_IsSoundPlay[MonsterState::MSTATE_HIT] && m_fAnimationKeyFrameIndex_Hit > m_SoundFrame[MonsterState::MSTATE_HIT])
		{
			m_IsSoundPlay[State::STATE_HIT] = true;
			CManagement::GetInstance()->GetSound()->PlayEffect(L"Sound", L"Spider_Hit_Sound");
			//CManagement::GetInstance()->GetSound()->PlayEffect(m_pMachineName, m_pStateName[State::STATE_ATTACK3]);		// > ��� ���尡 ������ �̷��� �ٲ����!
		}

		if (!m_IsEffectPlay[MonsterState::MSTATE_HIT] && m_fAnimationKeyFrameIndex_Hit > m_EffectFrame[MonsterState::MSTATE_HIT])
		{
			m_IsEffectPlay[MonsterState::MSTATE_HIT] = true;
			// > ��ų�־��ֱ�
			//CEffect_Manager::GetInstance()->Play_SkillEffect("��ų�̸�");
			CEffect_Manager::GetInstance()->Play_SkillEffect("hh", &m_pObject->GetWorld());
		}

		if (m_fAnimationKeyFrameIndex_Hit > (*vecAnimFrame)[MonsterState::MSTATE_HIT])
		{
			bTimerHit = false;
			m_fAnimationKeyFrameIndex_Hit = 0;

			m_IsSoundPlay[MonsterState::MSTATE_HIT] = false;
			m_IsEffectPlay[MonsterState::MSTATE_HIT] = false;

		}


	}

	if (bTimerDead == true)
	{
		if ((int)m_fAnimationKeyFrameIndex_Dead == 1)
		{
			CManagement::GetInstance()->GetSound()->PlayEffect(L"Sound", L"Spider_Dead_Sound");
		}

		//cout << m_fAnimationKeyFrameIndex_Dead << endl;
		if (m_bIsLife == true)
			m_fAnimationKeyFrameIndex_Dead += gt.DeltaTime() * 20;
		//m_iCurAnimFrame = m_fAnimationKeyFrameIndex_Attack3;

		if (m_fAnimationKeyFrameIndex_Dead + 1 > (*vecAnimFrame)[MonsterState::MSTATE_DEAD])
		{
			m_bIsLife = false;
			bTimerDead = false;
			//m_fAnimationKeyFrameIndex_Dead = 0;
		}

	}


}

void AnimateStateMachine_Spider::SetTimerTrueFalse()
{

	if (m_iAnimState == MonsterState::MSTATE_IDLE)
	{
		bTimerIdle = true;
		m_iCurAnimFrame = m_fAnimationKeyFrameIndex;
	}
	if (m_iAnimState == MonsterState::MSTATE_WALK)
	{
		bTimerWalk = true;
		m_iCurAnimFrame = m_fAnimationKeyFrameIndex_Walk;
	}
	if (m_iAnimState == MonsterState::MSTATE_ATTACK1)
	{
		bTimerAttack1 = true;
		m_iCurAnimFrame = m_fAnimationKeyFrameIndex_Attack1;
	}
	if (m_iAnimState == MonsterState::MSTATE_ATTACK2)
	{
		bTimerAttack2 = true;
		m_iCurAnimFrame = m_fAnimationKeyFrameIndex_Attack2;
	}

	if (m_iAnimState == MonsterState::MSTATE_HIT)
	{
		bTimerHit = true;
		m_iCurAnimFrame = m_fAnimationKeyFrameIndex_Hit;
	}

	if (m_iAnimState == MonsterState::MSTATE_DEAD)
	{
		bTimerDead = true;
		m_iCurAnimFrame = m_fAnimationKeyFrameIndex_Dead;
	}





}

AnimateStateMachine_Spider * AnimateStateMachine_Spider::Create(CGameObject* pObj, wchar_t * pMachineName, int SoundFrame[State::STATE_END], int EffectFrame[State::STATE_END])
{
	AnimateStateMachine_Spider* pInstance = new AnimateStateMachine_Spider(pObj, pMachineName, SoundFrame, EffectFrame);

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX(L"AnimateStateMachine_Spider Created Failed");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void AnimateStateMachine_Spider::Free()
{
}
