#pragma once

#include "Base.h"
#include "Define.h"

class CComponent;

class CComponent_Manager
	: public CBase
{
	DECLARE_SINGLETON(CComponent_Manager)

private:
	explicit CComponent_Manager();
	virtual ~CComponent_Manager();

public:
	CComponent* Clone_Component();

public:
	vector<CComponent*>			m_vecComponet;

private:
	virtual void Free();
};