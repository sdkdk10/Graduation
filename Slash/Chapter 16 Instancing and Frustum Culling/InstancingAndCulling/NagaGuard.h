#pragma once

#include "GameObject.h"
#include "../../SlashServer/SlashServer/Protocol.h"

class Player;

class NagaGuard : public CGameObject
{
private:
	const float m_fScale = 0.08f;
private:
	NagaGuard(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice, ComPtr<ID3D12DescriptorHeap> &srv, UINT srvSize);
public:
	virtual ~NagaGuard();
public:
	virtual bool			Update(const GameTimer & gt);
	virtual void			Render(ID3D12GraphicsCommandList* cmdList);
	virtual HRESULT			Initialize();
	virtual void			Animate(const GameTimer & gt);
	virtual void			SaveSlidingVector(CGameObject * pobj, CGameObject * pCollobj);

public:
	void SetTexture(SpiderType _tex);
	virtual void SetObjectAnimState(int _animState)
	{
		cout << _animState << endl;
		/*if (_animState == 5)
		{
			AnimStateMachine->SetAnimState(_animState - 1);

		}*/
		AnimStateMachine->SetAnimState(_animState);
	}
private:
	Player * m_pPlayer = nullptr;
	string					m_strTexName[SPIDER_END];

public:
	static NagaGuard* Create(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice, ComPtr<ID3D12DescriptorHeap> &srv, UINT srvSize);

};



class AnimateStateMachine_NagaGuard
	: public AnimateStateMachine
{
public:

public:
	explicit AnimateStateMachine_NagaGuard(CGameObject* pObj, wchar_t* pMachineName, int SoundFrame[State::STATE_END], int EffectFrame[State::STATE_END]);
	virtual ~AnimateStateMachine_NagaGuard();
private:
	HRESULT			Initialize();
	virtual void	AnimationStateUpdate(const GameTimer & gt);

private:
	CGameObject * m_pObject;
	wchar_t*		m_pMachineName;											// > � ��ü�� ���¸ӽ����� �̸��� �����ϴ� ����
	wchar_t*		m_pStateName[State::STATE_END];			// > �� ������ �̸�

	bool			m_IsSoundPlay[State::STATE_END];			// > �ش� �ִϸ��̼��� ���尡 ����ƴ��� Ȯ���ϴ� ����, �ִϸ��̼��� �ٲ�ų� ������ ��� false�� �ٲ�
	int				m_SoundFrame[State::STATE_END];			// > �ش� �ִϸ��̼��� ���° �����ӿ��� ���尡 ���;��ϴ��� �����ϰ� �ִ� ����
	bool			m_IsEffectPlay[State::STATE_END];			// > �ش� �ִϸ��̼��� ����Ʈ�� ����ƴ��� Ȯ���ϴ� ����, �ִϸ��̼��� �ٲ�ų� ������ ��� false�� �ٲ�
	int				m_EffectFrame[State::STATE_END];			// > �ش� �ִϸ��̼��� ���° �����ӿ��� ����Ʈ�� ���;��ϴ��� �����ϰ� �ִ� ����

public:
	static AnimateStateMachine_NagaGuard* Create(CGameObject* pObj, wchar_t* pMachineName, int SoundFrame[State::STATE_END], int EffectFrame[State::STATE_END]);

private:
	virtual void Free();
};