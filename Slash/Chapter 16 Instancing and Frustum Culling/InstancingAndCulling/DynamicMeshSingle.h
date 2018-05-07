#pragma once

#include "Mesh.h"



class DynamicMeshSingle : public Mesh
{
public:
	float m_fTest = 0;

	int iAnimframe = 0; // �ִϸ��̼� ������ �ε���

	vector<int> Vertexoffset;
	vector<int> Indexoffset;

	vector<int> VertexAnimoffset;
	vector<int> IndexAnimoffset;
public:

	HRESULT Initialize(vector<pair<const string, const string>> & pFilePath);


public:
	static DynamicMeshSingle* Create(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice, vector<pair<const string, const string>> & pFilePath);

	virtual CComponent* Clone(void);
	virtual void Free();

	vector<int> vecAnimFrame;


	bool bTimerTestIdle = false;
	bool bTimerTestWalk = false;
public:
	vector <std::unique_ptr<MeshGeometry>>	m_Geometry;

public:
	DynamicMeshSingle(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice);
	~DynamicMeshSingle();
};

