#pragma once

#include "Mesh.h"


class UIMesh :
	public Mesh
{
public:
	explicit UIMesh(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice);
	~UIMesh();
public:
	virtual HRESULT Initialize();
	virtual int Update(const GameTimer& gt);
	virtual void Draw();

public:
	static UIMesh* Create(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice);

	virtual CComponent* Clone(void);
	virtual void Free();

	vector <std::unique_ptr<MeshGeometry>>	m_Geometry;
};

