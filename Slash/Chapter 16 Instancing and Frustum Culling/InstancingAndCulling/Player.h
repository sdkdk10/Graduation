#pragma once

#include "GameObject.h"

class CBoundingBox;

class Player : public CGameObject
{
private:
	bool m_bAttackMotionForSound = true;
public:

/////////////////////////////////////////


///////////////////////////////////////////
	bool bAttackMotionTest = false;

	bool IsSoundChange = false;
	bool IsSoundIn = false;
//////////////////////////////////////////////

	virtual void SetPosition(float x, float y, float z);
	virtual void SetPosition(XMFLOAT3 xmf3Position);

	virtual void MoveStrafe(float fDistance = 1.0f);
	virtual void MoveUp(float fDistance = 1.0f);
	virtual void MoveForward(float fDistance = 1.0f);

	virtual void Rotate(float fPitch = 10.0f, float fYaw = 10.0f, float fRoll = 10.0f);
	virtual void Rotate(XMFLOAT3 *pxmf3Axis, float fAngle);

	virtual void Animate(const GameTimer & gt);

	virtual void Move(const XMFLOAT3& xmf3Shift, bool bVelocity = false);
	//virtual void Move(DWORD dwDirection, float fDistance, bool bUpdateVelocity, const GameTimer & gt);

	void KeyInput(const GameTimer & gt);
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

public:
	void					Render_Body(ID3D12GraphicsCommandList* cmdList);
	void					Render_Head(ID3D12GraphicsCommandList* cmdList);
	void					Render_Left(ID3D12GraphicsCommandList* cmdList);
	void					Render_Right(ID3D12GraphicsCommandList* cmdList);

public:
	static Player* Create(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice, ComPtr<ID3D12DescriptorHeap> &srv, UINT srvSize);

public:
	void SetCurState(int state) {
		m_CurState = state;
	}
	void SetPreState(int state) {
		m_PreState = state;
	}
	int GetCurState() {
		return m_CurState;
	}
	int GetPreState() {
		return m_PreState;
	}
private:
	float m_preKeyInputTime;
	float m_curKeyInputTime;
private:
	int m_CurState;
	int m_PreState;
};


class AnimateStateMachine_Player
	: public AnimateStateMachine
{
public:
	explicit AnimateStateMachine_Player(CGameObject* pObj, wchar_t* pMachineName, int SoundFrame[AnimateStateMachine::STATE_END], int EffectFrame[AnimateStateMachine::STATE_END]);
	virtual ~AnimateStateMachine_Player();
private:
	HRESULT			Initialize();
	virtual void	AnimationStateUpdate(const GameTimer & gt);

private:
	CGameObject*	m_pObject;
	wchar_t*		m_pMachineName;											// > � ��ü�� ���¸ӽ����� �̸��� �����ϴ� ����
	wchar_t*		m_pStateName[AnimateStateMachine::STATE_END];			// > �� ������ �̸�

	bool			m_IsSoundPlay[AnimateStateMachine::STATE_END];			// > �ش� �ִϸ��̼��� ���尡 ����ƴ��� Ȯ���ϴ� ����, �ִϸ��̼��� �ٲ�ų� ������ ��� false�� �ٲ�
	int				m_SoundFrame[AnimateStateMachine::STATE_END];			// > �ش� �ִϸ��̼��� ���° �����ӿ��� ���尡 ���;��ϴ��� �����ϰ� �ִ� ����
	bool			m_IsEffectPlay[AnimateStateMachine::STATE_END];			// > �ش� �ִϸ��̼��� ����Ʈ�� ����ƴ��� Ȯ���ϴ� ����, �ִϸ��̼��� �ٲ�ų� ������ ��� false�� �ٲ�
	int				m_EffectFrame[AnimateStateMachine::STATE_END];			// > �ش� �ִϸ��̼��� ���° �����ӿ��� ����Ʈ�� ���;��ϴ��� �����ϰ� �ִ� ����

public:
	static AnimateStateMachine_Player* Create(CGameObject* pObj, wchar_t* pMachineName, int SoundFrame[AnimateStateMachine::STATE_END], int EffectFrame[AnimateStateMachine::STATE_END]);

private:
	virtual void Free();
};