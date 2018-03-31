#include "stdafx.h"
#include "Component_Manager.h"

IMPLEMENT_SINGLETON(CComponent_Manager)

CComponent_Manager::CComponent_Manager()
{
}

CComponent_Manager::~CComponent_Manager()
{
}

CComponent * CComponent_Manager::Clone_Component()
{
	return nullptr;
}

void CComponent_Manager::Free()
{
}
