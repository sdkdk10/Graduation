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
	wchar_t*		m_pMachineName;											// > 어떤 객체의 상태머신인지 이름을 저장하는 변수
	wchar_t*		m_pStateName[AnimateStateMachine::STATE_END];			// > 각 상태의 이름

	bool			m_IsSoundPlay[AnimateStateMachine::STATE_END];			// > 해당 애니메이션의 사운드가 실행됐는지 확인하는 변수, 애니메이션이 바뀌거나 끝났을 경우 false로 바뀜
	int				m_SoundFrame[AnimateStateMachine::STATE_END];			// > 해당 애니메이션의 몇번째 프레임에서 사운드가 나와야하는지 저장하고 있는 변수
	bool			m_IsEffectPlay[AnimateStateMachine::STATE_END];			// > 해당 애니메이션의 이펙트가 실행됐는지 확인하는 변수, 애니메이션이 바뀌거나 끝났을 경우 false로 바뀜
	int				m_EffectFrame[AnimateStateMachine::STATE_END];			// > 해당 애니메이션의 몇번째 프레임에서 이펙트가 나와야하는지 저장하고 있는 변수

public:
	static AnimateStateMachine_Dragon* Create(CGameObject* pObj, wchar_t* pMachineName, int SoundFrame[AnimateStateMachine::STATE_END], int EffectFrame[AnimateStateMachine::STATE_END]);

private:
	virtual void Free();
};