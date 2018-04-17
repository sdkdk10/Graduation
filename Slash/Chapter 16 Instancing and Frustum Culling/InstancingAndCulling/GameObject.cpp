#include "stdafx.h"
#include "GameObject.h"
#include "Management.h"
#include "Mesh.h"
#include "Camera.h"
#include "Transform.h"

CGameObject::CGameObject(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice, ComPtr<ID3D12DescriptorHeap> &srv, UINT srvSize)
	: m_d3dDevice(d3dDevice)
	, mSrvDescriptorHeap(srv)
	, mCbvSrvDescriptorSize(srvSize)
{
	//m_pTransCom = 
}


CGameObject::~CGameObject()
{
}


bool CGameObject::Update(const GameTimer & gt)
{
	m_pFrameResource = CManagement::GetInstance()->GetCurFrameResource();
	m_pMesh->Update(gt);

	return true;
}

void CGameObject::Render(ID3D12GraphicsCommandList* cmdList)
{

}

HRESULT CGameObject::Initialize()
{
	return S_OK;
}

void CGameObject::RenderBounds(ID3D12GraphicsCommandList * cmdList)
{
}

void CGameObject::Free()
{
	Safe_Release(m_pMesh);
	Safe_Release(m_pTransCom);
}

void CGameObject::BuildOOBBRenderer(BoundingOrientedBox m_xmOOBB)
{
	
}

XMFLOAT3 CGameObject::GetPosition()
{
	return(XMFLOAT3(World._41, World._42, World._43));
}

XMFLOAT3 CGameObject::GetLook()
{
	return(Vector3::Normalize(XMFLOAT3(World._31, World._32, World._33)));

}

XMFLOAT3 CGameObject::GetUp()
{
	return(Vector3::Normalize(XMFLOAT3(World._21, World._22, World._23)));
}

XMFLOAT3 CGameObject::GetRight()
{
	return(Vector3::Normalize(XMFLOAT3(World._11, World._12, World._13)));
}

void CGameObject::SetPosition(float x, float y, float z)
{
	World._41 = x;
	World._42 = y;
	World._43 = z;
}

void CGameObject::SetPosition(XMFLOAT3 xmf3Position)
{
	SetPosition(xmf3Position.x, xmf3Position.y, xmf3Position.z);
}

void CGameObject::MoveStrafe(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Right = GetRight();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Right, fDistance);
	CGameObject::SetPosition(xmf3Position);
}

void CGameObject::MoveUp(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Up = GetUp();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Up, fDistance);
	CGameObject::SetPosition(xmf3Position);
}

void CGameObject::MoveForward(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Look = GetLook();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Look, fDistance);
	CGameObject::SetPosition(xmf3Position);
}

void CGameObject::Rotate(float fPitch, float fYaw, float fRoll)
{
	XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(fPitch), XMConvertToRadians(fYaw), XMConvertToRadians(fRoll));
	World = Matrix4x4::Multiply(mtxRotate, World);
}

void CGameObject::Rotate(XMFLOAT3 * pxmf3Axis, float fAngle)
{
	XMMATRIX mtxRotate = XMMatrixRotationAxis(XMLoadFloat3(pxmf3Axis), XMConvertToRadians(fAngle));
	World = Matrix4x4::Multiply(mtxRotate, World);
}

void CGameObject::Animate(const GameTimer & gt)
{
	// Animate

}
