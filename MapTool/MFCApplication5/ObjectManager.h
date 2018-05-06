#pragma once

#include "Base.h"
#include "Define.h"

#include "Renderer.h"
class CGameObject;
class GameTimer;

class CObjectManager
	: public CBase
{
	DECLARE_SINGLETON(CObjectManager)

private:
	explicit CObjectManager();
	virtual ~CObjectManager();

public:
	vector<ComPtr<ID3D12DescriptorHeap>>		GetSrvDescriptorHeap() { return m_pRenderer->GetSrvDescriptorHeap(); }
	UINT&										GetCbvSrvDescriptorSize() { return m_pRenderer->GetCbvSrvDescriptorSize(); }
	FrameResource*								GetCurFrameResource() { return mCurrFrameResource; }
	Microsoft::WRL::ComPtr<ID3D12Device>		GetDevice() { return m_d3dDevice; }
	CRenderer*						GetRenderer() { return m_pRenderer; }

	void			Clear_Object();


	CGameObject*	Find_Object(unsigned int iIdx);

	HRESULT			Add_Object(CGameObject* pObj);

public:
	HRESULT		Init_ObjMgr(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice, CRenderer* pRenderer);
	int			Update_ObjMgr(const GameTimer& gt, const FrameResource* pCurRsc);
	void		Render_ObjMgr(ID3D12GraphicsCommandList* cmdList);

	
private:
	CRenderer*						m_pRenderer;
	vector<CGameObject*>			m_vecObject;

	ComPtr<ID3D12DescriptorHeap> mSrvDescriptorHeap = nullptr;
	UINT mCbvSrvDescriptorSize = 0;

	FrameResource* mCurrFrameResource = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Device>			m_d3dDevice;
private:
	virtual void Free();
};