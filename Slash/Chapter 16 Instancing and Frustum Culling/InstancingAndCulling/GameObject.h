#pragma once
#include "Base.h"

class Camera;
class Mesh;
class GameTimer;
class CTransform;
struct RenderItem;

class GeometryMesh;

struct CB_ObjectConstants
{
	DirectX::XMFLOAT4X4 World = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 TexTransform = MathHelper::Identity4x4();
};
class CGameObject
	: public CBase
{
public:
	bool m_bIsVisiable = true;
	Camera * m_pCamera;
	BoundingFrustum				mCamFrustum;
	bool						mFrustumCullingEnabled = true;

	void					SetCamera(Camera* pCam) { m_pCamera = pCam; }
	void					SetCamFrustum(BoundingFrustum frustum) { mCamFrustum = frustum; }

public:
	void BuildOOBBRenderer(BoundingOrientedBox		m_xmOOBB);
public:
	BoundingOrientedBox		m_xmOOBB;
	BoundingOrientedBox		m_xmOOBBTransformed;

	void SetOOBB(XMFLOAT3& xmCenter, XMFLOAT3& xmExtents, XMFLOAT4& xmOrientation) { m_xmOOBBTransformed = m_xmOOBB = BoundingOrientedBox(xmCenter, xmExtents, xmOrientation); }

////////////////////////////////////////////////////////
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

	virtual void Animate(const GameTimer & gt);

	XMFLOAT3					m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3					m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
	XMFLOAT3					m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	XMFLOAT3					m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);

	float           			m_fPitch = 0.0f;
	float           			m_fYaw = 0.0f;
	float           			m_fRoll = 0.0f;
////////////////////////////////////////////////////
public:
	CGameObject(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice, ComPtr<ID3D12DescriptorHeap> &srv, UINT srvSize);
	virtual ~CGameObject();

public:
	XMFLOAT4X4& GetWorld() { return World; }

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

public:
	Mesh*				m_pMesh;
	CTransform*			m_pTransCom;
public:
	BoundingBox GetBounds() { return Bounds; }

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

