#pragma once
#include "Mesh.h"
#include "GeometryGenerator.h"

class GeometryMesh :
	public Mesh
{
public:
	explicit GeometryMesh(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice);
	~GeometryMesh();
public:
	virtual HRESULT Initialize();
	virtual int Update(const GameTimer& gt);
	virtual void Draw();
	virtual void Release();

	vector <std::unique_ptr<MeshGeometry>>	m_Geometry;
};


