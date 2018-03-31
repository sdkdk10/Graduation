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


struct AnimInfo
{
	vector<Mesh::Character>			mapAnimationModel;			// 애니메이션 프레임 마다의 정점들
	int								iAnimationFrameSize;		// 한 애니메이션 전체 프레임
};



class DynamicMesh
	: public Mesh
{
public:
public:
	int iAnimframe_Idle = 0; //애니메이션이 총 몇프레임인가?
	int iAnimframe_Walk = 0; //애니메이션이 총 몇프레임인가?

	int iAnimframe = 0; // 애니메이션 프레임 인덱스

public:
	explicit DynamicMesh(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice);
	virtual ~DynamicMesh();

public:
	virtual HRESULT Initialize(vector<pair<const string, const string>> & pFilePath);
	virtual int Update(const GameTimer& gt);
	virtual void Draw();
	virtual void Release();

	//virtual HRESULT LoadMesh(const wchar_t* AnimName, const char* pFilePath);

public:
	int				m_iAnimationState = 0;				// 애니메이션 상태
	int				m_iAnimationSize = 0;				// 모델이 갖고있는 애니메이션 수

	float			m_fAnimationKeyFrameIndex = 0.f;		// 애니메이션 인덱스
	float			m_fAnimationKeyFrameIndex_Walk = 0.f;		// 애니메이션 인덱스
	float			m_fAnimationKeyFrameIndex_Back = 0.f;		// 애니메이션 인덱스


	vector<int> vecAnimFrame;

	vector<vector<int>>		m_vecVertexOffset;
	vector<vector<int>>		m_vecIndexOffset;

	vector<vector<int>>		m_vecVertexAnimOffset;
	vector<vector<int>>		m_vecIndexAnimOffset;

public:
	vector <std::unique_ptr<MeshGeometry>>	m_Geometry;

//public:
//	std::unique_ptr<MeshGeometry> GetGeometry
//	{
//		return m_Geometry;
//	}
};
