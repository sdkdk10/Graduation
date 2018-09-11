#pragma once

#include "Base.h"
#include "Define.h"

class CSkillEffect;
class CEffect;

class CEffect_Manager
	: public CBase
{
	DECLARE_SINGLETON(CEffect_Manager)
private:
	explicit CEffect_Manager();
	virtual ~CEffect_Manager();

public:
	HRESULT					Ready_EffectManager(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice, vector<ComPtr<ID3D12DescriptorHeap>> &srv, UINT srvSize);
	HRESULT					Ready_Effect(string name, CEffect* effect);
	HRESULT					Ready_SkillEffect(string name, CSkillEffect* skillEffect);
	CSkillEffect*			Find_SkillEffect(string name);
	CEffect*				Find_Effect(string name);
	HRESULT					Play_SkillEffect(string name, XMFLOAT4X4* Parent = nullptr);
	HRESULT					Play_SkillEffect(string name, XMFLOAT4X4* Parent, float fRot);
	HRESULT					Stop_SkillEffect(string name);

private:
	unordered_map<string, CEffect*>					m_mapEffect;
	unordered_map<string, CSkillEffect*>			m_mapSkillEffect;

	vector<ComPtr<ID3D12DescriptorHeap>> mSrvDescriptorHeap;
	UINT mCbvSrvDescriptorSize = 0;

	Microsoft::WRL::ComPtr<ID3D12Device>		m_d3dDevice;

private:
	virtual void Free();
};