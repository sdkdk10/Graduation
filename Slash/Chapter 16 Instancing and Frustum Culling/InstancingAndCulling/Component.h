#pragma once

#include "Base.h"

class CGameTimer;

class CComponent
	: public CBase
{
protected:
	explicit CComponent();
	virtual ~CComponent();

public:
	virtual CComponent* Clone(void) = 0;
public:
	virtual void Update_Component(/*const CGameTimer & gt*/);
protected:
	virtual void Free(void);

};