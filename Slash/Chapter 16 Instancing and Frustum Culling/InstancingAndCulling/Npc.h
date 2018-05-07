 #pragma once

#include "GameObject.h"
#include "Define.h"

class CNpc
	: public CGameObject
{
private:
	explicit CNpc(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice, ComPtr<ID3D12DescriptorHeap> &srv, UINT srvSize, wchar_t* meshName);
public:
	virtual ~CNpc();

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

public:
	virtual HRESULT			Initialize();
	virtual bool			Update(const GameTimer& gt);
	virtual void			Render(ID3D12GraphicsCommandList * cmdList);

public:
	void					Render_Body(ID3D12GraphicsCommandList* cmdList);
	void					Render_Head(ID3D12GraphicsCommandList* cmdList);
	void					Render_Right(ID3D12GraphicsCommandList* cmdList);

public:
	static CNpc* Create(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice, ComPtr<ID3D12DescriptorHeap> &srv, UINT srvSize, wchar_t* meshName);

private:
	virtual void Free();

};