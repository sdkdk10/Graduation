#include "stdafx.h"
#include "StaticMesh.h"


StaticMesh::StaticMesh(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice) : Mesh(d3dDevice)
{

}


StaticMesh::~StaticMesh()
{
}

HRESULT StaticMesh::Initialize(vector<pair<const string, const string>> &pFilePath)
{
	std::ifstream fin(pFilePath[0].second);


	if (!fin)
	{
		
		return E_FAIL;
	}


	UINT vcount = 0;
	UINT tcount = 0;
	std::string ignore;
	Character curCharacter;


	//�ִϸ��̼� ���� ����
	while (!fin.eof())
	{
		fin >> ignore;

		if (ignore == "*TIMEVALUE")
		{
			//���� ��ǥ �о����
			fin >> curCharacter.iTimeValue;
			fin >> ignore; //*MESH_NUMVERTEX 
			fin >> curCharacter.iNumVertex;
			fin >> ignore; //*MESH_NUMFACES 
			fin >> curCharacter.iNumIndex;
			fin >> ignore; //*MESH_VERTEX_LIST 
			fin >> ignore; //{
			for (int i = 0; i < curCharacter.iNumVertex; ++i)
			{
				Vertex curPos;
				fin >> ignore; //*MESH_VERTEX
				fin >> ignore; // Index;
				fin >> curPos.Pos.x;
				fin >> curPos.Pos.y;
				fin >> curPos.Pos.z;

				curCharacter.vecVertex.push_back(curPos);

			}
			//�ε��� �о����
			fin >> ignore; //}
			fin >> ignore; //*MESH_FACE_LIST 
			fin >> ignore; // {

			fin >> ignore; //*MESH_FACE 
			for (int i = 0; i < curCharacter.iNumIndex; ++i)
			{
				fin >> ignore; // Index:
				fin >> ignore; // A:
				fin >> ignore;
				curCharacter.vecIndex.push_back(atoi(ignore.c_str()));
				fin >> ignore; // B:
				fin >> ignore;
				curCharacter.vecIndex.push_back(atoi(ignore.c_str()));
				fin >> ignore; // C:
				fin >> ignore;
				curCharacter.vecIndex.push_back(atoi(ignore.c_str()));
				while (ignore != "*MESH_FACE" && ignore != "}")
					fin >> ignore;
			}

			//�ؽ��� ��ǥ �о����
			fin >> ignore; // *MESH_NUMTVERTEX 
			fin >> curCharacter.iNumTexCnt;
			fin >> ignore; // *MESH_TVERLIST
			fin >> ignore; // {
			for (int i = 0; i < curCharacter.iNumTexCnt; ++i)
			{
				XMFLOAT2 curUV;

				fin >> ignore; // *MESH_TVERT
				fin >> ignore; // Index;
				fin >> curUV.x;
				fin >> curUV.y;
				fin >> ignore; // w

				curUV.y = 1.0f - curUV.y;

				curCharacter.uv.push_back(curUV);
			}

			//�ؽ��� ��ǥ �ε��� �о����
			fin >> ignore; // }
			fin >> ignore;

			fin >> curCharacter.iNumTexIndex;
			fin >> ignore; // *MESH_TFACELIST
			fin >> ignore; // {
			for (int i = 0; i < curCharacter.iNumTexIndex; ++i)
			{
				fin >> ignore; // *MESH_TFACE
				fin >> ignore; // INDEX
				fin >> ignore;
				curCharacter.uvIndex.push_back(atoi(ignore.c_str()));
				fin >> ignore;
				curCharacter.uvIndex.push_back(atoi(ignore.c_str()));
				fin >> ignore;
				curCharacter.uvIndex.push_back(atoi(ignore.c_str()));
			}


			while (ignore != "*MESH_NORMALS")
			{
				fin >> ignore; // ����о�

			}
			if (ignore == "*MESH_NORMALS")
			{

				fin >> ignore; //{
				for (int i = 0; i < curCharacter.iNumIndex; ++i)
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
						fin >> curCharacter.vecVertex[index].Normal.x;
						fin >> curCharacter.vecVertex[index].Normal.y;
						fin >> curCharacter.vecVertex[index].Normal.z;

					}


				}
			}
			curCharacter.realvecVertex.resize(curCharacter.uv.size());

			for (int i = 0; i < curCharacter.uv.size(); ++i)
			{
				curCharacter.realvecVertex[i].TexC = curCharacter.uv[i];
			}
			for (int i = 0; i < curCharacter.vecIndex.size(); ++i)
			{
				int vertexIndex = curCharacter.vecIndex[i];
				int realvecVertexIndex = curCharacter.uvIndex[i];

				curCharacter.realvecVertex[realvecVertexIndex].Pos = curCharacter.vecVertex[vertexIndex].Pos;
				curCharacter.realvecVertex[realvecVertexIndex].Normal = curCharacter.vecVertex[vertexIndex].Normal;

			}
			break;
		}

	}
	fin.close();

	//m_mapAnimations[const_cast<wchar_t*>(AnimName)] = tAnimInfo;


	///////////////////////////////////////////////////////////////////////////////////////


	//std::vector<Vertex> vertices;
	//std::vector<std::int32_t> indices;



	//Idle ���ؽ� / �ε��� ������ ����



	//Idle �ִϸ��̼� ���ؽ� ����


	//for (int i = 0; i < curCharacter.realvecVertex/*vecVertex*/.size(); ++i)
	//{
	//	vertices.push_back(curCharacter.realvecVertex/*vecVertex*/[i]);
	//}

	//Idle �ε��� ����

	//indices = curCharacter.uvIndex;



	size_t indexTest = curCharacter.uvIndex.size();

	const UINT vbByteSize = (UINT)curCharacter.realvecVertex.size() * sizeof(Vertex);

	const UINT ibByteSize = (UINT)curCharacter.uvIndex.size() * sizeof(std::int32_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "BarrelGeo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), curCharacter.realvecVertex.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), curCharacter.uvIndex.data(), ibByteSize);

	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(m_d3dDevice.Get(),
		mCommandList.Get(), curCharacter.realvecVertex.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(m_d3dDevice.Get(),
		mCommandList.Get(), curCharacter.uvIndex.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R32_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	SubmeshGeometry submesh;
	submesh.IndexCount = (UINT)curCharacter.uvIndex.size();//(UINT)indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	geo->DrawArgs["Barrel"] = submesh;

	m_Geometry = std::move(geo);

	return S_OK;
}

int StaticMesh::Update(const GameTimer & gt)
{
	return 0;
}

void StaticMesh::Draw()
{
}

void StaticMesh::Release()
{
}

