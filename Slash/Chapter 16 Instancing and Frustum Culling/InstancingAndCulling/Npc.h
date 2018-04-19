 #pragma once

#include "GameObject.h"
#include "Define.h"

class CNpc
	: public CGameObject
{
private:
	explicit CNpc(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice, ComPtr<ID3D12DescriptorHeap> &srv, UINT srvSize, vector<pair<const string, const string>> path);
public:
	virtual ~CNpc();

public:
	virtual void OnPrepareRender();
	virtual void Animate(const GameTimer & gt);

	virtual void Set_AnimState(int iState) { m_iCurAnimState = iState; }

private:
	int iTest = 0;
	int m_iCurAnimState = 1;

	vector<pair<const string, const string>>			m_vecMeshPath;
	MeshGeometry* Geo_Body = nullptr;
	MeshGeometry* Geo_Head = nullptr;
	MeshGeometry* Geo_Left = nullptr;
	MeshGeometry* Geo_Right = nullptr;

	DrawElement Element_Body;
	DrawElement Element_Head;
	DrawElement Element_Left;
	DrawElement Element_Right;

public:
	virtual HRESULT			Initialize();
	virtual bool			Update(const GameTimer& gt);
	virtual void			Render(ID3D12GraphicsCommandList * cmdList);

public:
	void					Render_Body(ID3D12GraphicsCommandList* cmdList);
	void					Render_Head(ID3D12GraphicsCommandList* cmdList);
	void					Render_Right(ID3D12GraphicsCommandList* cmdList);

public:
	static CNpc* Create(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice, ComPtr<ID3D12DescriptorHeap> &srv, UINT srvSize, vector<pair<const string, const string>> path);

private:
	virtual void Free();

};