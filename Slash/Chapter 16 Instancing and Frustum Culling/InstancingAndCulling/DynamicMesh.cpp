#include "stdafx.h"
#include "DynamicMesh.h"

DynamicMesh::DynamicMesh(Microsoft::WRL::ComPtr<ID3D12Device> md3dDevice)
	: Mesh(md3dDevice)
{
}

DynamicMesh::~DynamicMesh()
{
}

HRESULT DynamicMesh::Initialize(vector<pair<const string, const string>> &pFilePath)
{

	vector<vector<Bone>> vecBoneInfo;
	vecBoneInfo.resize(pFilePath.size());

	std::string ignore;
	bool bFirstBone = true;

	string curBoneName;

	std::vector<Vertex> vertices;
	std::vector<std::int32_t> indices;


	for (int i = 0; i < pFilePath.size(); ++i)
	{
		std::ifstream fin(pFilePath[i].second);

		if (!fin)
		{
			return E_FAIL;
		}
		bFirstBone = true;

		//Idle 로드
		while (!fin.eof())
		{
			if (bFirstBone)
				fin >> ignore;

			if (ignore == "*GEOMOBJECT") // Bone 발견
			{
				fin >> ignore; // {
				fin >> ignore; // *NODE_NAME
							   //fin >> curBoneName; // Node_name
				getline(fin, curBoneName);
				while (1)
				{
					fin >> ignore;
					if (fin.eof())
					{
						iAnimframe = 0;
						break;
					}
					if (ignore == "*GEOMOBJECT")  // 뼈 하나 다 읽을경우
					{
						if(iAnimframe != 0)
							vecAnimFrame.push_back(iAnimframe);
						iAnimframe = 0;
						bFirstBone = false;
						break;
					}
					if (ignore == "*TIMEVALUE") // TIMEVALUE 찾았으면
					{
						iAnimframe++;
						Bone tCurBoneInfo;
						tCurBoneInfo.boneName = curBoneName;

						fin >> tCurBoneInfo.iTimeValue; // 0 , 1580 ...
						fin >> ignore; // MESH_NUMVERTEX
						fin >> tCurBoneInfo.iNumVertex; // 115
						fin >> ignore; // MESH_NUMFACES
						fin >> tCurBoneInfo.iNumIndex; // 210
						fin >> ignore; //*MESH_VERTEX_LIST 
						fin >> ignore; //{

									   // 정점 읽어오기
						for (int i = 0; i < tCurBoneInfo.iNumVertex; ++i)
						{
							Vertex curPos;
							fin >> ignore; //*MESH_VERTEX
							fin >> ignore; // Index;
							fin >> curPos.Pos.x;
							fin >> curPos.Pos.y;
							fin >> curPos.Pos.z;

							tCurBoneInfo.vecVertex.push_back(curPos);
						}

						//인덱스 읽어오기
						fin >> ignore; //}
						fin >> ignore; //*MESH_FACE_LIST 
						fin >> ignore; // {

						for (int i = 0; i < tCurBoneInfo.iNumIndex; ++i)
						{
							fin >> ignore; //*MESH_FACE 
							fin >> ignore; // Index:
							fin >> ignore; // A:
							fin >> ignore;
							tCurBoneInfo.vecIndex.push_back(atoi(ignore.c_str()));
							fin >> ignore; // B:
							fin >> ignore;
							tCurBoneInfo.vecIndex.push_back(atoi(ignore.c_str()));
							fin >> ignore; // C:
							fin >> ignore;
							tCurBoneInfo.vecIndex.push_back(atoi(ignore.c_str()));

							fin >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore;
						}

						//텍스쳐 좌표 읽어오기
						fin >> ignore; // }
						fin >> ignore; // *MESH_NUMTVERTEX 

						fin >> tCurBoneInfo.iNumTexCnt;
						fin >> ignore; // *MESH_TVERLIST
						fin >> ignore; // {
						for (int i = 0; i < tCurBoneInfo.iNumTexCnt; ++i)
						{
							XMFLOAT2 curUV;

							fin >> ignore; // *MESH_TVERT
							fin >> ignore; // Index;
							fin >> curUV.x;
							fin >> curUV.y;
							fin >> ignore; // w

							curUV.y = 1.0f - curUV.y;

							tCurBoneInfo.uv.push_back(curUV);
						}

						//텍스쳐 좌표 인덱스 읽어오기
						fin >> ignore; // }
						fin >> ignore;

						fin >> tCurBoneInfo.iNumTexIndex;
						fin >> ignore; // *MESH_TFACELIST
						fin >> ignore; // {
						for (int i = 0; i < tCurBoneInfo.iNumTexIndex; ++i)
						{
							fin >> ignore; // *MESH_TFACE
							fin >> ignore; // INDEX
							fin >> ignore;
							tCurBoneInfo.uvIndex.push_back(atoi(ignore.c_str()));
							fin >> ignore;
							tCurBoneInfo.uvIndex.push_back(atoi(ignore.c_str()));
							fin >> ignore;
							tCurBoneInfo.uvIndex.push_back(atoi(ignore.c_str()));
						}
						while (ignore != "*MESH_NORMALS")
						{
							fin >> ignore; // 계속읽어

						}
						if (ignore == "*MESH_NORMALS")
						{
							fin >> ignore; //{
							for (int i = 0; i < tCurBoneInfo.iNumIndex; ++i)
							{
								int index = 0;

								fin >> ignore; //*MESH_FACENORMAL 
								fin >> ignore; // FaceIndex

								atoi(ignore.c_str());
								fin >> ignore >> ignore >> ignore;

								for (int k = 0; k < 3; ++k)
								{
									fin >> ignore; //*MESH_VERTEXNORMAL 
									fin >> index; // index;
									fin >> tCurBoneInfo.vecVertex[index].Normal.x;
									fin >> tCurBoneInfo.vecVertex[index].Normal.y;
									fin >> tCurBoneInfo.vecVertex[index].Normal.z;
								}
							}

							//텍스쳐 좌표  중복 해결
							tCurBoneInfo.realvecVertex.resize(tCurBoneInfo.uv.size());

							//텍스쳐 좌표는 그대로 넣어주고
							for (int i = 0; i < tCurBoneInfo.uv.size(); ++i)
							{
								tCurBoneInfo.realvecVertex[i].TexC = tCurBoneInfo.uv[i];
							}

							//정점을 텍스쳐 좌표만큼 중복시킨다.
							for (int i = 0; i < tCurBoneInfo.vecIndex.size(); ++i)
							{
								int vertexIndex = tCurBoneInfo.vecIndex[i];
								int realvecVertexIndex = tCurBoneInfo.uvIndex[i];

								tCurBoneInfo.realvecVertex[realvecVertexIndex].Pos = tCurBoneInfo.vecVertex[vertexIndex].Pos;
								tCurBoneInfo.realvecVertex[realvecVertexIndex].Normal = tCurBoneInfo.vecVertex[vertexIndex].Normal;
							}
							vecBoneInfo[i].push_back(tCurBoneInfo);
						}
					}
				}
			}
		}

	}



	vecAnimFrame.erase(unique(vecAnimFrame.begin(), vecAnimFrame.end()), vecAnimFrame.end());


	vector<string> boneName;
	boneName.push_back(" \"_head\"");
	boneName.push_back(" \"_body\"");
	boneName.push_back(" \"_rh-01\"");

	m_vecVertexAnimOffset.resize(boneName.size());
	m_vecIndexAnimOffset.resize(boneName.size());

	m_vecVertexOffset.resize(boneName.size());
	m_vecIndexOffset.resize(boneName.size());

	for (int boneCnt = 0; boneCnt < boneName.size(); ++boneCnt) //머리 , 몸, 무기 순으로
	{
		vertices.clear();
		indices.clear();

		for (int i = 0; i < vecBoneInfo.size(); ++i) //애니매이션 갯수만큼
		{

			for (auto iter = vecBoneInfo[i].begin(); iter != vecBoneInfo[i].end(); ++iter)
			{
				if ((*iter).boneName == boneName[boneCnt]/*" \"_head\""*/)
				{
					//Headoffset = i;
					m_vecVertexAnimOffset[boneCnt].push_back((int)vertices.size());
					m_vecIndexAnimOffset[boneCnt].push_back((int)indices.size());


					for (int k = 0; k< vecAnimFrame[i]; ++k)
					{
						m_vecVertexOffset[boneCnt].push_back((int)vertices.size());
						m_vecIndexOffset[boneCnt].push_back((int)indices.size());

						vertices.insert(vertices.end(), std::begin((*iter).realvecVertex/*vecIndex*/), std::end((*iter).realvecVertex/*vecIndex*/));
						indices.insert(indices.end(), std::begin((*iter)./*vecIndex*/uvIndex), std::end((*iter)./*vecIndex*/uvIndex));
						++iter;
					}
					break;
				}


			}

		}

		const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
		const UINT ibByteSize = (UINT)indices.size() * sizeof(std::int32_t);

		auto geo = std::make_unique<MeshGeometry>();
		geo->Name = boneName[boneCnt];

		ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
		CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

		ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
		CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

		geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(m_d3dDevice.Get(),
			mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

		geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(m_d3dDevice.Get(),
			mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

		geo->VertexByteStride = sizeof(Vertex);
		geo->VertexBufferByteSize = vbByteSize;
		geo->IndexFormat = DXGI_FORMAT_R32_UINT;
		geo->IndexBufferByteSize = ibByteSize;

		SubmeshGeometry submesh;
		submesh.IndexCount = (UINT)indices.size();
		submesh.StartIndexLocation = 0;
		submesh.BaseVertexLocation = 0;

		geo->DrawArgs[boneName[boneCnt]] = submesh;
		m_Geometry.push_back(std::move(geo));// = std::move(geo);
	}


	return S_OK;
}

int DynamicMesh::Update(const GameTimer & gt)
{


		m_fAnimationKeyFrameIndex += gt.DeltaTime() * 25;

		if (m_fAnimationKeyFrameIndex > vecAnimFrame[0])
		{
			m_fAnimationKeyFrameIndex = 0;
		}
	
		m_fAnimationKeyFrameIndex_Walk += gt.DeltaTime() * 25;

		if (m_fAnimationKeyFrameIndex_Walk > vecAnimFrame[1])
		{
			m_fAnimationKeyFrameIndex_Walk = 0;
		}

		m_fAnimationKeyFrameIndex_Back += gt.DeltaTime() * 25;

		if (m_fAnimationKeyFrameIndex_Back > vecAnimFrame[2])
		{
			m_fAnimationKeyFrameIndex_Back = 0;
		}

	

	


	//m_fAnimationKeyFrameIndex += gt.DeltaTime() * 10;

	//if (m_fAnimationKeyFrameIndex > m_vecAnimPerFrameSize[m_iAnimationState])
	//{
	//	m_fAnimationKeyFrameIndex = 0;
	//}
	return 0;
}

void DynamicMesh::Draw()
{
}

void DynamicMesh::Release()
{
}

