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
	HRESULT					Ready_Effect(string name, CEffect* effect);
	HRESULT					Ready_SkillEffect(string name, CSkillEffect* skillEffect);
	CSkillEffect*			Find_SkillEffect(string name);
	CEffect*				Find_Effect(string name);
	HRESULT					Play_SkillEffect(string name);

private:
	unordered_map<string, CEffect*>					m_mapEffect;
	unordered_map<string, CSkillEffect*>			m_mapSkillEffect;

private:
	virtual void Free();
};