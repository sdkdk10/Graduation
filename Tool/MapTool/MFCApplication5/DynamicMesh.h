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
class DynamicMesh
	: public Mesh
{
public:
public:
	int iAnimframe_Idle = 0; //�ִϸ��̼��� �� ���������ΰ�?
	int iAnimframe_Walk = 0; //�ִϸ��̼��� �� ���������ΰ�?

	int iAnimframe = 0; // �ִϸ��̼� ������ �ε���

public:
	explicit DynamicMesh(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice);
	virtual ~DynamicMesh();

public:
	HRESULT Initialize(vector<pair<const string, const string>> & pFilePath);

public:
	static DynamicMesh* Create(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice, vector<pair<const string, const string>> & pFilePath);

	virtual CComponent* Clone(void);
	virtual void Free();

	//virtual HRESULT LoadMesh(const wchar_t* AnimName, const char* pFilePath);

public:
	bool bTimerTestIdle = false;
	bool bTimerTestWalk = false;
	bool bTimerTestAttack1 = false;
	bool bTimerTestAttack2 = false;
	bool bTimerTestAttack3 = false;


	int				m_iAnimationState = 0;				// �ִϸ��̼� ����
	int				m_iAnimationSize = 0;				// ���� �����ִ� �ִϸ��̼� ��

	float			m_fAnimationKeyFrameIndex = 0.f;		// �ִϸ��̼� �ε���
	float			m_fAnimationKeyFrameIndex_Walk = 0.f;		// �ִϸ��̼� �ε���

	float			m_fAnimationKeyFrameIndex_Attack1 = 0.f;		// �ִϸ��̼� �ε���
	float			m_fAnimationKeyFrameIndex_Attack2 = 0.f;		// �ִϸ��̼� �ε���
	float			m_fAnimationKeyFrameIndex_Attack3 = 0.f;		// �ִϸ��̼� �ε���

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
