#include "stdafx.h"
#include "UIMesh.h"




UIMesh::UIMesh(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice) : Mesh(d3dDevice)
{
}

UIMesh::~UIMesh()
{
}

HRESULT UIMesh::Initialize()
{
	std::vector<Vertex> vertices(4);
	std::vector<std::uint32_t> indices(6);

	vertices[0].Pos = XMFLOAT3(-500.5f,500.5f,0);
	vertices[1].Pos = XMFLOAT3(500.5f, 500.5f, 0);
	vertices[2].Pos = XMFLOAT3(-500.5f, -500.5f, 0);
	vertices[3].Pos = XMFLOAT3(500.5f, -500.5f, 0);



	indices[0] = 0;
	indices[1] = 1;
	indices[2] = 2;
	//ÀÇ½É
	indices[3] = 2;
	indices[4] = 1;
	indices[5] = 3;

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "shapeGeo";

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
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	SubmeshGeometry submesh;
	submesh.IndexCount = (UINT)indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	geo->DrawArgs["UI"] = submesh;


	m_Geometry.push_back(std::move(geo));

	return S_OK;

}

int UIMesh::Update(const GameTimer & gt)
{
	return 0;
}

void UIMesh::Draw()
{
}

UIMesh * UIMesh::Create(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice)
{
	return nullptr;
}

CComponent * UIMesh::Clone(void)
{
	return nullptr;
}

void UIMesh::Free()
{
}
