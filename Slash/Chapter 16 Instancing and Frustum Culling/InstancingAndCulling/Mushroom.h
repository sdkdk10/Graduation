#pragma once

#include "GameObject.h"
#include "../../SlashServer/SlashServer/Protocol.h"

class Player;

class Mushroom : public CGameObject
{
private:
	const float m_fScale = 0.05f;
private:
	Mushroom(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice, ComPtr<ID3D12DescriptorHeap> &srv, UINT srvSize);
public:
	virtual ~Mushroom();
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
	/*	if (_animState == 5)
		{
			AnimStateMachine->SetAnimState(4);

		}
		else if (_animState == 4)
		{
			AnimStateMachine->SetAnimState(5);

		}
		else
		{

		}*/
		AnimStateMachine->SetAnimState(_animState);

	}
private:
	Player * m_pPlayer = nullptr;
	string					m_strTexName[SPIDER_END];

public:
	static Mushroom* Create(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice, ComPtr<ID3D12DescriptorHeap> &srv, UINT srvSize);

};


class AnimateStateMachine_Mushroom
	: public AnimateStateMachine
{
public:

public:
	explicit AnimateStateMachine_Mushroom(CGameObject* pObj, wchar_t* pMachineName, int SoundFrame[State::STATE_END], int EffectFrame[State::STATE_END]);
	virtual ~AnimateStateMachine_Mushroom();
private:
	HRESULT			Initialize();
	virtual void	AnimationStateUpdate(const GameTimer & gt);
	virtual void SetTimerTrueFalse();

private:
	CGameObject * m_pObject;
	wchar_t*		m_pMachineName;											// > � ��ü�� ���¸ӽ����� �̸��� �����ϴ� ����
	wchar_t*		m_pStateName[State::STATE_END];			// > �� ������ �̸�

	bool			m_IsSoundPlay[State::STATE_END];			// > �ش� �ִϸ��̼��� ���尡 ����ƴ��� Ȯ���ϴ� ����, �ִϸ��̼��� �ٲ�ų� ������ ��� false�� �ٲ�
	int				m_SoundFrame[State::STATE_END];			// > �ش� �ִϸ��̼��� ���° �����ӿ��� ���尡 ���;��ϴ��� �����ϰ� �ִ� ����
	bool			m_IsEffectPlay[State::STATE_END];			// > �ش� �ִϸ��̼��� ����Ʈ�� ����ƴ��� Ȯ���ϴ� ����, �ִϸ��̼��� �ٲ�ų� ������ ��� false�� �ٲ�
	int				m_EffectFrame[State::STATE_END];			// > �ش� �ִϸ��̼��� ���° �����ӿ��� ����Ʈ�� ���;��ϴ��� �����ϰ� �ִ� ����

public:
	static AnimateStateMachine_Mushroom* Create(CGameObject* pObj, wchar_t* pMachineName, int SoundFrame[State::STATE_END], int EffectFrame[State::STATE_END]);

private:
	virtual void Free();
};