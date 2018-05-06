#pragma once

#include "GameObject.h"

class CBoundingBox;

class Player : public CGameObject
{
	const float m_fMoveSpeed = 10.0f;
	const float m_fRotateSpeed = 3.0f;
	const float m_fDegree = 51.2958f;
	const float m_fScale = 0.05f;
	float RotationAngle = 0.0f;

	float RotationDeltaRIGHT = 0.0f;
	float RotationDeltaLEFT = 0.0f;

	float RotationDeltaFORWARD = 0.0f;
	float RotationDeltaBACKWARD = 0.0f;

///////////////////////////////////////////
	bool bAttackMotionTest = false;


//////////////////////////////////////////////


	virtual void SetPosition(float x, float y, float z);
	virtual void SetPosition(XMFLOAT3 xmf3Position);

	virtual void MoveStrafe(float fDistance = 1.0f);
	virtual void MoveUp(float fDistance = 1.0f);
	virtual void MoveForward(float fDistance = 1.0f);

	virtual void Rotate(float fPitch = 10.0f, float fYaw = 10.0f, float fRoll = 10.0f);
	virtual void Rotate(XMFLOAT3 *pxmf3Axis, float fAngle);

	virtual void Animate(const GameTimer & gt);

	void Move(const XMFLOAT3& xmf3Shift, bool bVelocity = false);
	void Move(DWORD dwDirection, float fDistance, bool bUpdateVelocity);
//////////////////////////////////////////////
public:
	int iTest = 0;
	int iTest2 = 0;
	MeshGeometry* Geo_Body = nullptr;
	MeshGeometry* Geo_Head = nullptr;
	MeshGeometry* Geo_Left = nullptr;
	MeshGeometry* Geo_Right = nullptr;

	DrawElement Element_Body;
	DrawElement Element_Head;
	DrawElement Element_Left;
	DrawElement Element_Right;

	float fAngleTest = 0.1f;
	float fAngleTestSum = 0.0f;

	float fAngleTest2 = 0.1f;
	float fAngleTestSum2 = 0.0f;
	CBoundingBox*						m_pBoundMesh = nullptr;
private:
	Player(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice, ComPtr<ID3D12DescriptorHeap> &srv, UINT srvSize);
public:
	virtual ~Player();
public:
	virtual bool			Update(const GameTimer & gt);
	virtual void			Render(ID3D12GraphicsCommandList* cmdList);
	virtual HRESULT			Initialize();
	virtual void			Free();
	virtual void Set_AnimState(int iState) { KeyInputTest = iState; }

public:
	void					Render_Body(ID3D12GraphicsCommandList* cmdList);
	void					Render_Head(ID3D12GraphicsCommandList* cmdList);
	void					Render_Left(ID3D12GraphicsCommandList* cmdList);
	void					Render_Right(ID3D12GraphicsCommandList* cmdList);

public:
	static Player* Create(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice, ComPtr<ID3D12DescriptorHeap> &srv, UINT srvSize);

private:
	float preKeyInputTime;
	float curKeyInputTime;
};

