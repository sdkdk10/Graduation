#pragma once

#include "GameObject.h"

class CBoundingBox;
class HPBar;
class NumUI;

class Player : public CGameObject
{
	struct tSkillUI
	{
		HPBar*		pUI = nullptr;
		float		fResetTime;				// �ٽ� Ȱ��ȭ �Ǳ���� �ɸ��� �ð�
		float		fStayTime = 0.f;
		float		alphaAdd;				// �� �����Ӵ� �������� ���İ� 1.f / fResetTime
		bool		isActive = true;		// true : ��ųȰ��ȭ false : ��Ȱ��ȭ
	};
	int AnimationtTest = 0;
public:
	bool bIsUltimateState = false;
	float m_fUltimateTime = 20.0f;
	void CheckUltimate(const GameTimer & gt);
private:
	bool m_bAttackMotionForSound = true;
public:

/////////////////////////////////////////


///////////////////////////////////////////
	bool bAttackMotionTest = false;

	bool IsSoundChange = false;
	bool IsSoundIn = false;
//////////////////////////////////////////////
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
	Player(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice, ComPtr<ID3D12DescriptorHeap> &srv, UINT srvSize, bool isWarrior);
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
	static Player* Create(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice, ComPtr<ID3D12DescriptorHeap> &srv, UINT srvSize, bool isWarrior);

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
	bool GetIsWarrior() {
		return m_IsWarrior;
	}

	void SetUltimateEffect(bool isUltimate);

	void			UIUpdate(const GameTimer& gt);

	virtual void	AddExp(float exp);
	virtual void	SetExp(float exp);

	void			SetLevel(int iLv);

private:
	float m_preKeyInputTime;
	float m_curKeyInputTime;
private:
	int m_CurState;
	int m_PreState;

	bool m_IsWarrior;
	bool	m_GageFull = false;
	HPBar*		m_HpBar;
	HPBar*		m_ExpBar;
	HPBar*		m_GageBar;
	NumUI*		m_LvUI;
	HPBar*		m_GageSkillUI;
	tSkillUI	m_SkillUI[5];

	float		m_fMaxHp = 100.f;
	float		m_fMaxExp = 100.f;
	float		m_fCurGage = 0.f;
	float		m_fMaxGage = 100.f;

	int			m_iLevel = 1;
	
	int			m_iUltimateEffectCnt = -1;
};


class AnimateStateMachine_Player
	: public AnimateStateMachine
{
public:
	
public:
	explicit AnimateStateMachine_Player(CGameObject* pObj, wchar_t* pMachineName, int SoundFrame[State::STATE_END], int EffectFrame[State::STATE_END]);
	virtual ~AnimateStateMachine_Player();
private:
	HRESULT			Initialize();
	virtual void	AnimationStateUpdate(const GameTimer & gt);
	
public:
	void			SetUltimateEffect(bool isUltimate);

private:
	CGameObject*	m_pObject;
	wchar_t*		m_pMachineName;											// > � ��ü�� ���¸ӽ����� �̸��� �����ϴ� ����
	wchar_t*		m_pStateName[State::STATE_END];			// > �� ������ �̸�

	bool			m_IsSoundPlay[State::STATE_END];			// > �ش� �ִϸ��̼��� ���尡 ����ƴ��� Ȯ���ϴ� ����, �ִϸ��̼��� �ٲ�ų� ������ ��� false�� �ٲ�
	int				m_SoundFrame[State::STATE_END];			// > �ش� �ִϸ��̼��� ���° �����ӿ��� ���尡 ���;��ϴ��� �����ϰ� �ִ� ����
	bool			m_IsEffectPlay[State::STATE_END];			// > �ش� �ִϸ��̼��� ����Ʈ�� ����ƴ��� Ȯ���ϴ� ����, �ִϸ��̼��� �ٲ�ų� ������ ��� false�� �ٲ�
	int				m_EffectFrame[State::STATE_END];			// > �ش� �ִϸ��̼��� ���° �����ӿ��� ����Ʈ�� ���;��ϴ��� �����ϰ� �ִ� ����

	unordered_map<State, string>	m_mapEffectName;

public:
	static AnimateStateMachine_Player* Create(CGameObject* pObj, wchar_t* pMachineName, int SoundFrame[State::STATE_END], int EffectFrame[State::STATE_END]);

private:
	virtual void Free();
};