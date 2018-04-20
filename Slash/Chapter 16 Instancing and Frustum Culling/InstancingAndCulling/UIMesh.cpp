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
	std::vector<Vertex> vertices(6);

	vertices[0].Pos = XMFLOAT3(-0.5f,0.5f,0.0f);
	vertices[0].TexC= XMFLOAT2(0.0f, 0.0f);

	vertices[1].Pos = XMFLOAT3(0.5f, -0.5f, 0.0f);
	vertices[1].TexC = XMFLOAT2(1.0f, 0.0f);

	vertices[2].Pos = XMFLOAT3(-0.5f, -0.5f, 0.0f);
	vertices[2].TexC = XMFLOAT2(0.0f, 1.0f);



	vertices[3].Pos = XMFLOAT3(-0.5f, 0.5f, 0.0f);
	vertices[3].TexC = XMFLOAT2(0.0f, 0.0f);

	vertices[4].Pos = XMFLOAT3(0.5f, 0.5f, 0.0f);
	vertices[4].TexC = XMFLOAT2(1.0f, 0.0f);

	vertices[5].Pos = XMFLOAT3(0.5f, -0.5f, 0.0f);
	vertices[5].TexC = XMFLOAT2(0.0f, 1.0f);


	//vertices[3].Pos = XMFLOAT3(-0.5f, 0.5f, 0);
	//vertices[3].TexC = XMFLOAT2(1.0f, 1.0f);

	


	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "shapeGeo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);


	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(m_d3dDevice.Get(),
		mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);



	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;

	SubmeshGeometry submesh;
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
