#pragma once
#include "Base.h"

class Camera;
class Mesh;
class GameTimer;
class CTransform;
struct RenderItem;

class GeometryMesh;
constexpr      unsigned long MAXOBJECTID = 100000;

typedef struct objdrawelement
{
	UINT IndexCount = 0;
	UINT StartIndexLocation = 0;
	int BaseVertexLocation = 0;
}DrawElement;

struct CB_ObjectConstants
{
	DirectX::XMFLOAT4X4 World = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 TexTransform = MathHelper::Identity4x4();
};

class AnimateStateMachine
	: public CBase
{
public:
	enum ANIM_STATE { STATE_IDLE, STATE_WALK, STATE_ATTACK1, STATE_ATTACK2, STATE_ATTACK3, STATE_DEAD, STATE_END,STATE_ULTIMATE, STATE_ROLL};
public:
	bool m_bIsLife = true;

public:
	bool bTimerIdle = false;
	bool bTimerWalk = false;
	bool bTimerAttack1 = false;
	bool bTimerAttack2 = false;
	bool bTimerAttack3 = false;
	bool bTimerDead = false;
	bool bTimerHit = false;

	bool bTimerUltimate = false;
	bool bTimerRoll = false;

private:

protected:
	float			m_fAnimationKeyFrameIndex = 0.f;		// 애니메이션 인덱스
	float			m_fAnimationKeyFrameIndex_Walk = 0.f;		// 애니메이션 인덱스

	float			m_fAnimationKeyFrameIndex_Attack1 = 0.f;		// 애니메이션 인덱스
	float			m_fAnimationKeyFrameIndex_Attack2 = 0.f;		// 애니메이션 인덱스
	float			m_fAnimationKeyFrameIndex_Attack3 = 0.f;		// 애니메이션 인덱스

	float			m_fAnimationKeyFrameIndex_Dead = 0.f;		// 애니메이션 인덱스
	float			m_fAnimationKeyFrameIndex_Ultimate = 0.f;
	float			m_fAnimationKeyFrameIndex_Roll = 0.f;


public:
	vector<int> * vecAnimFrame;

public: //애니메이션 상태

	const int IdleState = 0;
	const int WalkState = 1;
	const int Attack1State = 2;
	const int Attack2State = 3;
	const int Attack3State = 4;
	const int DeadState = 5;
	const int UltimateState = 6;
	const int RollState = 7;
protected:
	int m_iAnimState = 0; // 현재 애니메이션 상태
	int m_iCurAnimFrame = 0; // 현재 애니메이션 몇번째 프레임인지
public:
	virtual void AnimationStateUpdate(const GameTimer & gt);
	void SetTimerTrueFalse();
	void SetAnimState(int _animstate) { m_iAnimState = _animstate; }
	int GetAnimState() { return m_iAnimState; }
	int GetCurAnimFrame() { return m_iCurAnimFrame; }

private:
	virtual void Free();
};

class CGameObject
	: public CBase
{
protected:
	bool m_bLODState = false;
public:
public:
	CGameObject * m_pCollider = NULL;
public:
	XMFLOAT3 m_MovingRefletVector = XMFLOAT3(0, 0, 0); // 슬라이딩 벡터를 위한 반사벡터
													   //XMFLOAT4					m_pxmf4WallPlanes[4]; 
public:
	AnimateStateMachine * GetAnimateMachine() { return AnimStateMachine; }
public:
	void SetObjectAnimState(int _animState) { AnimStateMachine->SetAnimState(_animState); }
protected:
	AnimateStateMachine * AnimStateMachine = nullptr;
public:
	int planeCollision = 0;
public:
	bool m_bIsCollide = false;

private:
	float hp = 100;
public:
	float GetHp() { return hp; }
	void SetHp(float _hp) { hp = _hp; }
public:
	bool m_bIsVisiable = true;

	bool m_bIsConnected = false;
	Camera * m_pCamera = nullptr;
	BoundingFrustum				mCamFrustum;
	bool						mFrustumCullingEnabled = true;

	void					SetCamera(Camera* pCam) { m_pCamera = pCam; }
	void					SetCamFrustum(BoundingFrustum frustum) { mCamFrustum = frustum; }

public:
public:
	BoundingOrientedBox		m_xmOOBB;
	BoundingOrientedBox		m_xmOOBBTransformed;

	void SetOOBB(XMFLOAT3& xmCenter, XMFLOAT3& xmExtents, XMFLOAT4& xmOrientation) { m_xmOOBBTransformed = m_xmOOBB = BoundingOrientedBox(xmCenter, xmExtents, xmOrientation); }

	////////////////////////////////////////////////////////
protected:
	float m_fMoveSpeed = 8.0f;
	float m_fRotateSpeed = 12.0f;
	float m_fDegree = 57.3248f;
	float m_fScale = 0.05f;
	float RotationAngle = 0.0f;

	float RotationDeltaRIGHT = 0.0f;
	float RotationDeltaLEFT = 0.0f;

	float RotationDeltaFORWARD = 0.0f;
	float RotationDeltaBACKWARD = 0.0f;
public:
	XMFLOAT3 GetPosition();
	XMFLOAT3 GetLook();
	XMFLOAT3 GetUp();
	XMFLOAT3 GetRight();

	virtual void SetPosition(float x, float y, float z);
	virtual void SetPosition(XMFLOAT3 xmf3Position);

	virtual void MoveStrafe(float fDistance = 1.0f);
	virtual void MoveUp(float fDistance = 1.0f);
	virtual void MoveForward(float fDistance = 1.0f);

	virtual void Rotate(float fPitch = 10.0f, float fYaw = 10.0f, float fRoll = 10.0f);
	virtual void Rotate(XMFLOAT3 *pxmf3Axis, float fAngle);
	virtual void Rotation(float x, float y, float z);
	virtual void Move(const XMFLOAT3& xmf3Shift, bool bVelocity = false);
	virtual void Move(DWORD dwDirection, float fDistance, bool bUpdateVelocity);

	virtual void Animate(const GameTimer & gt); //애니메이션 상태 설정, 객체 이동, 회전 여기서 하면 됨

	XMFLOAT3					m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3					m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
	XMFLOAT3					m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	XMFLOAT3					m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);

	XMFLOAT3					m_xmf3Height = XMFLOAT3(0.0f, 0.0f, 1.0f);
	XMFLOAT3					m_xmf3Width = XMFLOAT3(1.0f, 0.0f, 0.0f);
	XMFLOAT3					m_xmf3Scale;
	XMFLOAT3					m_xmf3Rot;

	float           			m_fPitch = 0.0f;
	float           			m_fYaw = 0.0f;
	float           			m_fRoll = 0.0f;
	////////////////////////////////////////////////////
public:
	CGameObject(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice, ComPtr<ID3D12DescriptorHeap> &srv, UINT srvSize);
	virtual ~CGameObject();

public:
	XMFLOAT4X4 & GetWorld() { return World; }

protected:
	// World matrix of the shape that describes the object's local space
	// relative to the world space, which defines the position, orientation,
	// and scale of the object in the world.
	XMFLOAT4X4 World = MathHelper::Identity4x4();

	XMFLOAT4X4 TexTransform = MathHelper::Identity4x4();

	// Dirty flag indicating the object data has changed and we need to update the constant buffer.
	// Because we have an object cbuffer for each FrameResource, we have to apply the
	// update to each FrameResource.  Thus, when we modify obect data we should set 
	// NumFramesDirty = gNumFrameResources so that each frame resource gets the update.
	int NumFramesDirty = gNumFrameResources;

	// Index into GPU constant buffer corresponding to the ObjectCB for this render item.
	UINT ObjCBIndex = -1;

	Material* Mat = nullptr;
	MeshGeometry* Geo = nullptr;

	MeshGeometry* Geo_Bounds = nullptr;
	// Primitive topology.
	D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	// DrawIndexedInstanced parameters.
	UINT IndexCount = 0;
	UINT StartIndexLocation = 0;
	int BaseVertexLocation = 0;


	FrameResource*								m_pFrameResource = NULL;
	Microsoft::WRL::ComPtr<ID3D12Device>		m_d3dDevice;

	ComPtr<ID3D12DescriptorHeap> mSrvDescriptorHeap = nullptr;
	UINT mCbvSrvDescriptorSize = 0;

	BoundingBox					Bounds;

	static unsigned long	m_iAllObjectIndex;
	static CGameObject*      m_pAllObject[MAXOBJECTID];

	unsigned long         m_iMyObjectID;

public:
	Mesh * m_pMesh = nullptr;
	CTransform*			m_pTransCom = nullptr;
	wchar_t*			m_pwstrMeshName;
public:
	BoundingBox GetBounds() { return Bounds; }
	virtual void		SaveSlidingVector(CGameObject * pobj, CGameObject * pCollobj);
	virtual CTransform* GetTransform() { return m_pTransCom; }

public:
	virtual bool			Update(const GameTimer & gt);
	virtual void			Render(ID3D12GraphicsCommandList* cmdList);
	virtual void			OnPrepareRender() { };
	virtual HRESULT			Initialize();
	//virtual void			Release();

	void SetFrameResource(FrameResource* frs) { m_pFrameResource = frs; }
	void SetSrvDescriptorHeap(ComPtr<ID3D12DescriptorHeap> &srv) { mSrvDescriptorHeap = srv; }
	void SetSrvDescriptorSize(UINT srvSize) { mCbvSrvDescriptorSize = srvSize; }

	virtual void			RenderBounds(ID3D12GraphicsCommandList* cmdList);

public:

protected:
	virtual void			Free();
};

