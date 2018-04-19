#pragma once

#include "Mesh.h"


typedef struct bone
{

	int iTimeValue = 0;

	string boneName;

	vector<Vertex> vecVertex;
	vector<int> vecIndex;

	int iNumVertex = 0;
	int iNumIndex = 0;

	vector<XMFLOAT2> uv;
	vector<int>		uvIndex;

	int iNumTexCnt = 0;
	int iNumTexIndex = 0;

	vector<Vertex> realvecVertex;

	static bool Comp(const bone &t1, const bone &t2)
	{
		return (t1.boneName < t2.boneName);
	}


}Bone;


typedef struct animInfo
{
	vector<Mesh::Character>			AnimationModel;			// 애니메이션 프레임 마다의 정점들
	int								iAnimationFrameSize;		// 한 애니메이션 전체 프레임
}AnimInfo;


class DynamicMeshSingle : public Mesh
{
public:
	float m_fTest = 0;

	int iAnimframe = 0; // 애니메이션 프레임 인덱스

	vector<int> Vertexoffset;
	vector<int> Indexoffset;

	vector<int> VertexAnimoffset;
	vector<int> IndexAnimoffset;
public:

	virtual HRESULT Initialize(vector<pair<const string, const string>> & pFilePath);
	virtual int Update(const GameTimer& gt);
	virtual void Draw();

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

