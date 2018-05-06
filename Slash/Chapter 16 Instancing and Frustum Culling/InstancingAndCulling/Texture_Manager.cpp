#include "stdafx.h"
#include "Texture_Manager.h"

IMPLEMENT_SINGLETON(CTexture_Manager);

CTexture_Manager::CTexture_Manager()
{

}

CTexture_Manager::~CTexture_Manager()
{
}

HRESULT CTexture_Manager::Ready_Texture(string TexName, Texture* tex, TEXTYPE eType)
{
	if (Find_Texture(TexName, eType) != nullptr)
		return E_FAIL;

	m_mapTexture[eType].emplace(TexName, tex);

	return S_OK;
}

Texture* CTexture_Manager::Find_Texture(string TexName, TEXTYPE eType)
{
	auto& iter = m_mapTexture[eType].find(TexName);

	if (iter == m_mapTexture[eType].end())
		return nullptr;

	return iter->second;
}

void CTexture_Manager::Free()
{

}
