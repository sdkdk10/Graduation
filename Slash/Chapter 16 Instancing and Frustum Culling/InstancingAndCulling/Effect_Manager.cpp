#include "stdafx.h"
#include "Effect_Manager.h"
#include "Management.h"
#include "SkillEffect.h"

IMPLEMENT_SINGLETON(CEffect_Manager)

CEffect_Manager::CEffect_Manager()
{
}

CEffect_Manager::~CEffect_Manager()
{
}

HRESULT CEffect_Manager::Ready_Effect(string name, CEffect * effect)
{
	if (Find_Effect(name) == nullptr)
		return E_FAIL;

	m_mapEffect.emplace(name, effect);

	return S_OK;
}

HRESULT CEffect_Manager::Ready_SkillEffect(string name, CSkillEffect * skillEffect)
{
	if (Find_SkillEffect(name) == nullptr)
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

HRESULT CEffect_Manager::Play_SkillEffect(string name)
{
	auto effect = Find_SkillEffect(name);
	if (effect == nullptr)
		return E_FAIL;

//	CGameObject* obj = new CSkillEffect(effect);

	CManagement::GetInstance()->Get_CurScene()->Ready_GameObject(L"Effect", dynamic_cast<CGameObject*>(effect));

	return S_OK;
}

void CEffect_Manager::Free()
{

}
