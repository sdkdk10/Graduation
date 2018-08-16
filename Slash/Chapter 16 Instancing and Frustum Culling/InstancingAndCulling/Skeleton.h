 #pragma once

#include "GameObject.h"
#include "Define.h"

class CSkeleton
	: public CGameObject
{
private:
	explicit CSkeleton(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice, ComPtr<ID3D12DescriptorHeap> &srv, UINT srvSize, wchar_t* meshName, bool isWarrior);
public:
	virtual ~CSkeleton();

public:
	virtual void OnPrepareRender();
	virtual void Animate(const GameTimer & gt);


private:

	vector<pair<const string, const string>>			m_vecMeshPath;
	MeshGeometry* Geo_Body = nullptr;
	MeshGeometry* Geo_Head = nullptr;
	MeshGeometry* Geo_Left = nullptr;
	MeshGeometry* Geo_Right = nullptr;

	DrawElement Element_Body;
	DrawElement Element_Head;
	DrawElement Element_Left;
	DrawElement Element_Right;

	bool		m_IsWarrior;

public:
	virtual HRESULT			Initialize();
	virtual bool			Update(const GameTimer& gt);
	virtual void			Render(ID3D12GraphicsCommandList * cmdList);

public:
	void					Render_Body(ID3D12GraphicsCommandList* cmdList);
	void					Render_Head(ID3D12GraphicsCommandList* cmdList);
	void					Render_Right(ID3D12GraphicsCommandList* cmdList);

	virtual void Rotate(float fPitch, float fYaw, float fRoll);

public:
	static CSkeleton* Create(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice, ComPtr<ID3D12DescriptorHeap> &srv, UINT srvSize, wchar_t* meshName, bool isWarrior);

	void Move(const XMFLOAT3 & xmf3Shift, bool bVelocity);

public:
	void SetCurState(int state) {
		m_CurState = state;
	}
	void SetPreState(int state) {
		m_PreState = state;
	}
	int GetCurState() {
		return m_CurState;
	}
	int GetPreState() {
		return m_PreState;
	}
private:
	int m_CurState;
	int m_PreState;

private:
	virtual void Free();

};