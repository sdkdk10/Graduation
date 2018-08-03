#include "stdafx.h"
#include "Effect_Manager.h"
#include "Management.h"
#include "SkillEffect.h"
#include "Effect.h"
#include "Renderer.h"

IMPLEMENT_SINGLETON(CEffect_Manager)

CEffect_Manager::CEffect_Manager()
{
}

CEffect_Manager::~CEffect_Manager()
{
}

HRESULT CEffect_Manager::Ready_EffectManager(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice, vector<ComPtr<ID3D12DescriptorHeap>> &srv, UINT srvSize)
{
	m_d3dDevice = d3dDevice;
	mSrvDescriptorHeap = srv;
	mCbvSrvDescriptorSize = srvSize;
	//for (auto& elem : m_mapSkillEffect)
	//{
	//	CManagement::GetInstance()->Get_CurScene()->Ready_GameObject(L"Effect", elem.second);
	//}
	return S_OK;
}

HRESULT CEffect_Manager::Ready_Effect(string name, CEffect * effect)
{
	if (Find_Effect(name) != nullptr)	
		return E_FAIL;

	m_mapEffect.emplace(name, effect);

	return S_OK;
}

HRESULT CEffect_Manager::Ready_SkillEffect(string name, CSkillEffect * skillEffect)
{
	if (Find_SkillEffect(name) != nullptr)
		return E_FAIL;

	m_mapSkillEffect.emplace(name, skillEffect);

	return S_OK;
}

CSkillEffect * CEffect_Manager::Find_SkillEffect(string name)
{
	auto& iter = m_mapSkillEffect.find(name);
	if (iter == m_mapSkillEffect.end())
		return nullptr;

	return iter->second;
}

CEffect * CEffect_Manager::Find_Effect(string name)
{
	auto& iter = m_mapEffect.find(name);
	
	if (iter == m_mapEffect.end())
		return nullptr;
	
	return iter->second;
}

HRESULT CEffect_Manager::Play_SkillEffect(string name, XMFLOAT4X4* Parent)
{
	auto effect = Find_SkillEffect(name);
	if (effect == nullptr)
		return E_FAIL;
	
	//CSkillEffect* play = CSkillEffect::Create(CManagement::GetInstance()->Getdev)
	

	//auto play = CSkillEffect::Create(*effect);
	auto play = CSkillEffect::Create(m_d3dDevice, mSrvDescriptorHeap[HEAP_DEFAULT], mCbvSrvDescriptorSize, name);
	//list<CEffect*> Desclist = effect->GetEffectList();
	//list<CEffect*> Srclist;
	//for (auto elem : Desclist)
	//{
	//	CEffect* pInst = CEffect::Create(m_d3dDevice, mSrvDescriptorHeap[HEAP_DEFAULT], mCbvSrvDescriptorSize, elem->Get_EffectInfo());
	//	Srclist.push_back(pInst);
	//}
	//play->GetEffectList() = Srclist;



	list<CEffect*> DescList = effect->GetEffectList();

	for (auto elem : DescList)
	{
		CEffect* finder = Find_Effect(elem->Get_EffectInfo().strName);
		CEffect* pInst = CEffect::Create(m_d3dDevice, mSrvDescriptorHeap[HEAP_DEFAULT], mCbvSrvDescriptorSize, finder->Get_EffectInfo());
		play->GetEffectList().push_back(pInst);
	}


	play->SetPlay(true);
	if (Parent != nullptr)
	{
		play->Set_ParentMatrix(Parent);
	}

	CManagement::GetInstance()->Get_CurScene()->Ready_GameObject(L"Effect", dynamic_cast<CGameObject*>(play));

	return S_OK;
}

HRESULT CEffect_Manager::Stop_SkillEffect(string name)
{
	auto effect = Find_SkillEffect(name);
	if (effect == nullptr)
		return E_FAIL;

	effect->SetPlay(false);

	return S_OK;
}

void CEffect_Manager::Free()
{

}
