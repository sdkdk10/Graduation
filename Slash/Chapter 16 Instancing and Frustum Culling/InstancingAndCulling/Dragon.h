#pragma once

#include "GameObject.h"

class Dragon : public CGameObject
{
private:
	const float m_fScale = 0.1f;
private:
	Dragon(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice, ComPtr<ID3D12DescriptorHeap> &srv, UINT srvSize);
public:
	virtual ~Dragon();
public:
	virtual bool			Update(const GameTimer & gt);
	virtual void			Render(ID3D12GraphicsCommandList* cmdList);
	virtual HRESULT			Initialize();
	virtual void			Animate(const GameTimer & gt);


public:
	static Dragon* Create(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice, ComPtr<ID3D12DescriptorHeap> &srv, UINT srvSize);

};

class AnimateStateMachine_Dragon
	: public AnimateStateMachine
{
public:
	explicit AnimateStateMachine_Dragon(CGameObject* pObj, wchar_t* pMachineName, int SoundFrame[AnimateStateMachine::STATE_END], int EffectFrame[AnimateStateMachine::STATE_END]);
	virtual ~AnimateStateMachine_Dragon();
private:
	HRESULT			Initialize();
	virtual void	AnimationStateUpdate(const GameTimer & gt);

private:
	CGameObject * m_pObject;
	wchar_t*		m_pMachineName;											// > � ��ü�� ���¸ӽ����� �̸��� �����ϴ� ����
	wchar_t*		m_pStateName[AnimateStateMachine::STATE_END];			// > �� ������ �̸�

	bool			m_IsSoundPlay[AnimateStateMachine::STATE_END];			// > �ش� �ִϸ��̼��� ���尡 ����ƴ��� Ȯ���ϴ� ����, �ִϸ��̼��� �ٲ�ų� ������ ��� false�� �ٲ�
	int				m_SoundFrame[AnimateStateMachine::STATE_END];			// > �ش� �ִϸ��̼��� ���° �����ӿ��� ���尡 ���;��ϴ��� �����ϰ� �ִ� ����
	bool			m_IsEffectPlay[AnimateStateMachine::STATE_END];			// > �ش� �ִϸ��̼��� ����Ʈ�� ����ƴ��� Ȯ���ϴ� ����, �ִϸ��̼��� �ٲ�ų� ������ ��� false�� �ٲ�
	int				m_EffectFrame[AnimateStateMachine::STATE_END];			// > �ش� �ִϸ��̼��� ���° �����ӿ��� ����Ʈ�� ���;��ϴ��� �����ϰ� �ִ� ����

public:
	static AnimateStateMachine_Dragon* Create(CGameObject* pObj, wchar_t* pMachineName, int SoundFrame[AnimateStateMachine::STATE_END], int EffectFrame[AnimateStateMachine::STATE_END]);

private:
	virtual void Free();
};