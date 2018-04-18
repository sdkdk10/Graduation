//***************************************************************************************
// InstancingAndCullingApp.cpp by Frank Luna (C) 2015 All Rights Reserved.
//***************************************************************************************

#include "stdafx.h"
#include "InstancingAndCullingApp.h"
#include "InstancingObject.h"
#include "InputDevice.h"
#include "Management.h"
#include "Renderer.h"
#include "TestScene.h"
#include "Network.h"

const int gNumFrameResources = 3;
Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> mCommandList;
int KeyInputTest = 0;

// Lightweight structure stores parameters to draw a shape.  This will
// vary from app-to-app.


typedef struct character
{
	int iTimeValue = 0;
	vector<Vertex> vecVertex;
	vector<int> vecIndex;

	int iNumVertex = 0;
	int iNumIndex = 0;

	vector<XMFLOAT2> uv;
	vector<int>		uvIndex;

	int iNumTexCnt = 0;
	int iNumTexIndex = 0;

	vector<Vertex> realvecVertex;

}Character;

typedef struct animInfo
{
	vector<Character>				mapAnimationModel;			// 애니메이션 프레임 마다의 정점들
	int								iAnimationFrameSize;		// 한 애니메이션 전체 프레임
}AnimInfo;


enum class RenderLayer : int
{
	Opaque = 0,
	InstancingOpaque,
	Debug,
	Sky,
	Count
};



int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
	PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	try
	{
		InstancingAndCullingApp theApp(hInstance);
		if (!theApp.Initialize())
			return 0;

		return theApp.Run();
	}
	catch (DxException& e)
	{
		MessageBox(nullptr, e.ToString().c_str(), L"HR Failed", MB_OK);
		return 0;
	}
}

InstancingAndCullingApp::InstancingAndCullingApp(HINSTANCE hInstance)
	: D3DApp(hInstance)
{
}

InstancingAndCullingApp::~InstancingAndCullingApp()
{
	if (md3dDevice != nullptr)
		FlushCommandQueue();
}

bool InstancingAndCullingApp::Initialize()
{

	mSrvDescriptorHeap.resize(2);

	if (!D3DApp::Initialize())
		return false;

	AllocConsole();
	freopen("CONOUT$", "wt", stdout);

	// Reset the command list to prep for initialization commands.
	ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));

	// Get the increment size of a descriptor in this heap type.  This is hardware specific, 
	// so we have to query this information.
	mCbvSrvDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	mCamera.SetPosition(0.0f, 2.0f, -15.0f);

	LoadTextures();
	BuildRootSignature();
	BuildDescriptorHeaps();
	BuildShadersAndInputLayout();

	//BuildShapeGeometry();
	//BuildSkullGeometry();
	//BuildBarrelGeometry();


	CRenderer* pRenderer = CRenderer::Create(md3dDevice, mSrvDescriptorHeap);

	CInputDevice::GetInstance()->Ready_InputDevice(mhMainWnd, mhAppInst);
	CManagement::GetInstance()->Init_Management(pRenderer);
	

	BuildPSOs();
	BuildMaterials();
	BuildRenderItems();
	BuildFrameResources();


	CManagement::GetInstance()->GetRenderer()->SetPSOs(mPSOs);
	CManagement::GetInstance()->Get_CurScene()->Set_MainCam(&mCamera);
	CManagement::GetInstance()->Get_CurScene()->Set_CamFrustum(&mCamFrustum);
	mCamera.Set_Object(CManagement::GetInstance()->Get_CurScene()->Find_Object(L"Layer_Player", 0));

	// Execute the initialization commands.
	ThrowIfFailed(mCommandList->Close());
	ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// Wait until initialization is complete.
	FlushCommandQueue();

	//CNetwork::GetInstance()->InitSock(mhMainWnd);

	return true;
}

void InstancingAndCullingApp::BuildShapeGeometry()
{
	GeometryGenerator geoGen;
	GeometryGenerator::MeshData box = geoGen.CreateBox(1.0f, 1.0f, 1.0f, 3);
	GeometryGenerator::MeshData grid = geoGen.CreateGrid(20.0f, 30.0f, 60, 40);
	GeometryGenerator::MeshData sphere = geoGen.CreateSphere(0.5f, 20, 20);
	GeometryGenerator::MeshData cylinder = geoGen.CreateCylinder(0.5f, 0.3f, 3.0f, 20, 20);

	//
	// We are concatenating all the geometry into one big vertex/index buffer.  So
	// define the regions in the buffer each submesh covers.
	//

	// Cache the vertex offsets to each object in the concatenated vertex buffer.
	UINT boxVertexOffset = 0;
	UINT gridVertexOffset = (UINT)box.Vertices.size();
	UINT sphereVertexOffset = gridVertexOffset + (UINT)grid.Vertices.size();
	UINT cylinderVertexOffset = sphereVertexOffset + (UINT)sphere.Vertices.size();

	// Cache the starting index for each object in the concatenated index buffer.
	UINT boxIndexOffset = 0;
	UINT gridIndexOffset = (UINT)box.Indices32.size();
	UINT sphereIndexOffset = gridIndexOffset + (UINT)grid.Indices32.size();
	UINT cylinderIndexOffset = sphereIndexOffset + (UINT)sphere.Indices32.size();

	SubmeshGeometry boxSubmesh;
	boxSubmesh.IndexCount = (UINT)box.Indices32.size();
	boxSubmesh.StartIndexLocation = boxIndexOffset;
	boxSubmesh.BaseVertexLocation = boxVertexOffset;

	SubmeshGeometry gridSubmesh;
	gridSubmesh.IndexCount = (UINT)box.Indices32.size();
	gridSubmesh.StartIndexLocation = gridIndexOffset;
	gridSubmesh.BaseVertexLocation = gridVertexOffset;

	SubmeshGeometry sphereSubmesh;
	sphereSubmesh.IndexCount = (UINT)box.Indices32.size();
	sphereSubmesh.StartIndexLocation = sphereIndexOffset;
	sphereSubmesh.BaseVertexLocation = sphereVertexOffset;

	SubmeshGeometry cylinderSubmesh;
	cylinderSubmesh.IndexCount = (UINT)cylinder.Indices32.size();
	cylinderSubmesh.StartIndexLocation = cylinderIndexOffset;
	cylinderSubmesh.BaseVertexLocation = cylinderVertexOffset;

	//
	// Extract the vertex elements we are interested in and pack the
	// vertices of all the meshes into one vertex buffer.
	//

	auto totalVertexCount =
		box.Vertices.size() +
		grid.Vertices.size() +
		sphere.Vertices.size() +
		cylinder.Vertices.size();

	std::vector<Vertex> vertices(totalVertexCount);

	UINT k = 0;
	for (size_t i = 0; i < box.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = box.Vertices[i].Position;
		vertices[k].Normal = box.Vertices[i].Normal;
		vertices[k].TexC = box.Vertices[i].TexC;
	}

	for (size_t i = 0; i < grid.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = grid.Vertices[i].Position;
		vertices[k].Normal = grid.Vertices[i].Normal;
		vertices[k].TexC = grid.Vertices[i].TexC;
	}

	for (size_t i = 0; i < sphere.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = sphere.Vertices[i].Position;
		vertices[k].Normal = sphere.Vertices[i].Normal;
		vertices[k].TexC = sphere.Vertices[i].TexC;
	}

	for (size_t i = 0; i < cylinder.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = cylinder.Vertices[i].Position;
		vertices[k].Normal = cylinder.Vertices[i].Normal;
		vertices[k].TexC = cylinder.Vertices[i].TexC;
	}

	std::vector<std::uint16_t> indices;
	indices.insert(indices.end(), std::begin(box.GetIndices16()), std::end(box.GetIndices16()));
	indices.insert(indices.end(), std::begin(grid.GetIndices16()), std::end(grid.GetIndices16()));
	indices.insert(indices.end(), std::begin(sphere.GetIndices16()), std::end(sphere.GetIndices16()));
	indices.insert(indices.end(), std::begin(cylinder.GetIndices16()), std::end(cylinder.GetIndices16()));

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "shapeGeo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	geo->DrawArgs["box"] = boxSubmesh;
	geo->DrawArgs["grid"] = gridSubmesh;
	geo->DrawArgs["sphere"] = sphereSubmesh;
	geo->DrawArgs["cylinder"] = cylinderSubmesh;

	mGeometries[geo->Name] = std::move(geo);
}

void InstancingAndCullingApp::OnResize()
{
	D3DApp::OnResize();

	mCamera.SetLens(0.25f*MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);

	BoundingFrustum::CreateFromMatrix(mCamFrustum, mCamera.GetProj());
}

void InstancingAndCullingApp::Update(const GameTimer& gt)
{
	OnKeyboardInput(gt);

	CInputDevice::GetInstance()->SetUp_InputDeviceState();

	// Cycle through the circular frame resource array.
	mCurrFrameResourceIndex = (mCurrFrameResourceIndex + 1) % gNumFrameResources;
	mCurrFrameResource = mFrameResources[mCurrFrameResourceIndex].get();

	// Has the GPU finished processing the commands of the current frame resource?
	// If not, wait until the GPU has completed commands up to this fence point.
	if (mCurrFrameResource->Fence != 0 && mFence->GetCompletedValue() < mCurrFrameResource->Fence)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
		ThrowIfFailed(mFence->SetEventOnCompletion(mCurrFrameResource->Fence, eventHandle));
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}

	AnimateMaterials(gt);
	//UpdateMaterialBuffer(gt);
	//UpdateInstanceData(gt);

	/*	UpdateObjectCBs(gt);
	UpdateMaterialCBs(gt);*/

	CManagement::GetInstance()->Update(gt, mCurrFrameResource);

	UpdateMainPassCB(gt);
}

void InstancingAndCullingApp::Draw(const GameTimer& gt)
{
	auto cmdListAlloc = mCurrFrameResource->CmdListAlloc;

	// Reuse the memory associated with command recording.
	// We can only reset when the associated command lists have finished execution on the GPU.
	ThrowIfFailed(cmdListAlloc->Reset());

	// A command list can be reset after it has been added to the command queue via ExecuteCommandList.
	// Reusing the command list reuses memory.
	ThrowIfFailed(mCommandList->Reset(cmdListAlloc.Get(), mPSOs["InstancingOpaque"].Get()));


	mCommandList->RSSetViewports(1, &mScreenViewport);
	mCommandList->RSSetScissorRects(1, &mScissorRect);

	// Indicate a state transition on the resource usage.
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	// Clear the back buffer and depth buffer.
	mCommandList->ClearRenderTargetView(CurrentBackBufferView(), Colors::LightSteelBlue, 0, nullptr);
	mCommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	// Specify the buffers we are going to render to.
	mCommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());

	ID3D12DescriptorHeap* descriptorHeaps[] = { mSrvDescriptorHeap[1].Get() };
	mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	mCommandList->SetGraphicsRootSignature(mRootSignature.Get());

	// Bind all the materials used in this scene.  For structured buffers, we can bypass the heap and 
	// set as a root descriptor.
	auto matBuffer = mCurrFrameResource->MaterialBuffer->Resource();
	mCommandList->SetGraphicsRootShaderResourceView(1, matBuffer->GetGPUVirtualAddress());

	auto passCB = mCurrFrameResource->PassCB->Resource();
	mCommandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());

	// Bind all the textures used in this scene.
	mCommandList->SetGraphicsRootDescriptorTable(3, mSrvDescriptorHeap[1]->GetGPUDescriptorHandleForHeapStart() );

	mCommandList->SetPipelineState(mPSOs["InstancingOpaque"].Get());

	DrawRenderItems(mCommandList.Get(), mOpaqueRitems);

	// Indicate a state transition on the resource usage.
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	// Done recording commands.
	ThrowIfFailed(mCommandList->Close());

	// Add the command list to the queue for execution.
	ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// Swap the back and front buffers
	ThrowIfFailed(mSwapChain->Present(0, 0));
	mCurrBackBuffer = (mCurrBackBuffer + 1) % SwapChainBufferCount;

	// Advance the fence value to mark commands up to this fence point.
	mCurrFrameResource->Fence = ++mCurrentFence;

	// Add an instruction to the command queue to set a new fence point. 
	// Because we are on the GPU timeline, the new fence point won't be 
	// set until the GPU finishes processing all the commands prior to this Signal().
	mCommandQueue->Signal(mFence.Get(), mCurrentFence);
}

void InstancingAndCullingApp::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	SetCapture(mhMainWnd);
}

void InstancingAndCullingApp::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void InstancingAndCullingApp::OnMouseMove(WPARAM btnState, int x, int y)
{
	if ((btnState & MK_LBUTTON) != 0)
	{
		// Make each pixel correspond to a quarter of a degree.
		float dx = XMConvertToRadians(0.25f*static_cast<float>(x - mLastMousePos.x));
		float dy = XMConvertToRadians(0.25f*static_cast<float>(y - mLastMousePos.y));

		mCamera.Pitch(dy);
		mCamera.RotateY(dx);
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}

void InstancingAndCullingApp::OnKeyboardInput(const GameTimer& gt)
{
	const float dt = gt.DeltaTime();

	if (GetAsyncKeyState('W') & 0x8000)
		mCamera.Walk(20.0f*dt);

	if (GetAsyncKeyState('S') & 0x8000)
		mCamera.Walk(-20.0f*dt);

	if (GetAsyncKeyState('A') & 0x8000)
		mCamera.Strafe(-20.0f*dt);

	if (GetAsyncKeyState('D') & 0x8000)
		mCamera.Strafe(20.0f*dt);

	if (GetAsyncKeyState('1') & 0x8000)
		mFrustumCullingEnabled = true;

	if (GetAsyncKeyState('2') & 0x8000)
		mFrustumCullingEnabled = false;

	if (GetAsyncKeyState('3') & 0x8000)
		KeyInputTest= 0;

	if (GetAsyncKeyState('4') & 0x8000)
		KeyInputTest = 1;

	if (GetAsyncKeyState('5') & 0x8000)
		KeyInputTest = 2;


	//mCamera.UpdateViewMatrix();
	mCamera.Update();
}

void InstancingAndCullingApp::AnimateMaterials(const GameTimer& gt)
{

}

void InstancingAndCullingApp::UpdateInstanceData(const GameTimer& gt)
{
	XMMATRIX view = mCamera.GetView();
	XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);

	auto currInstanceBuffer = mCurrFrameResource->InstanceBuffer.get();
	for (auto& e : mAllRitems)
	{
		const auto& instanceData = e->Instances;

		int visibleInstanceCount = 0;

		for (UINT i = 0; i < (UINT)instanceData.size(); ++i)
		{
			XMMATRIX world = XMLoadFloat4x4(&instanceData[i].World);
			XMMATRIX texTransform = XMLoadFloat4x4(&instanceData[i].TexTransform);

			XMMATRIX invWorld = XMMatrixInverse(&XMMatrixDeterminant(world), world);

			// View space to the object's local space.
			XMMATRIX viewToLocal = XMMatrixMultiply(invView, invWorld);

			// Transform the camera frustum from view space to the object's local space.
			BoundingFrustum localSpaceFrustum;
			mCamFrustum.Transform(localSpaceFrustum, viewToLocal);

			// Perform the box/frustum intersection test in local space.
			if ((localSpaceFrustum.Contains(e->Bounds) != DirectX::DISJOINT) || (mFrustumCullingEnabled == false))
			{
				InstanceData data;
				XMStoreFloat4x4(&data.World, XMMatrixTranspose(world));
				XMStoreFloat4x4(&data.TexTransform, XMMatrixTranspose(texTransform));
				data.MaterialIndex = instanceData[i].MaterialIndex;

				// Write the instance data to structured buffer for the visible objects.
				currInstanceBuffer->CopyData(visibleInstanceCount++, data);
			}
		}

		e->InstanceCount = visibleInstanceCount;

		std::wostringstream outs;
		outs.precision(6);
		outs << L"Instancing and Culling Demo" <<
			L"    " << e->InstanceCount <<
			L" objects visible out of " << e->Instances.size();
		mMainWndCaption = outs.str();
	}
}

void InstancingAndCullingApp::UpdateMaterialBuffer(const GameTimer& gt)
{
	auto currMaterialBuffer = mCurrFrameResource->MaterialBuffer.get();
	for (auto& e : mMaterials)
	{
		// Only update the cbuffer data if the constants have changed.  If the cbuffer
		// data changes, it needs to be updated for each FrameResource.
		Material* mat = e.second.get();
	/*	if (mat->NumFramesDirty > 0)
		{*/
			XMMATRIX matTransform = XMLoadFloat4x4(&mat->MatTransform);

			MaterialData matData;
			matData.DiffuseAlbedo = mat->DiffuseAlbedo;
			matData.FresnelR0 = mat->FresnelR0;
			matData.Roughness = mat->Roughness;
			XMStoreFloat4x4(&matData.MatTransform, XMMatrixTranspose(matTransform));
			matData.DiffuseMapIndex = mat->DiffuseSrvHeapIndex;

			currMaterialBuffer->CopyData(mat->MatCBIndex, matData);

			// Next FrameResource need to be updated too.
			//mat->NumFramesDirty--;
		//}
	}
}

void InstancingAndCullingApp::UpdateMainPassCB(const GameTimer& gt)
{
	XMMATRIX view = mCamera.GetView();
	XMMATRIX proj = mCamera.GetProj();

	XMMATRIX viewProj = XMMatrixMultiply(view, proj);
	XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);
	XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(proj), proj);
	XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(viewProj), viewProj);

	XMStoreFloat4x4(&mMainPassCB.View, XMMatrixTranspose(view));
	XMStoreFloat4x4(&mMainPassCB.InvView, XMMatrixTranspose(invView));
	XMStoreFloat4x4(&mMainPassCB.Proj, XMMatrixTranspose(proj));
	XMStoreFloat4x4(&mMainPassCB.InvProj, XMMatrixTranspose(invProj));
	XMStoreFloat4x4(&mMainPassCB.ViewProj, XMMatrixTranspose(viewProj));
	XMStoreFloat4x4(&mMainPassCB.InvViewProj, XMMatrixTranspose(invViewProj));
	mMainPassCB.EyePosW = mCamera.GetPosition3f();
	mMainPassCB.RenderTargetSize = XMFLOAT2((float)mClientWidth, (float)mClientHeight);
	mMainPassCB.InvRenderTargetSize = XMFLOAT2(1.0f / mClientWidth, 1.0f / mClientHeight);
	mMainPassCB.NearZ = 1.0f;
	mMainPassCB.FarZ = 1000.0f;
	mMainPassCB.TotalTime = gt.TotalTime();
	mMainPassCB.DeltaTime = gt.DeltaTime();
	mMainPassCB.AmbientLight = { 0.25f, 0.25f, 0.35f, 1.0f };
	mMainPassCB.Lights[0].Direction = { 0.57735f, -0.57735f, 0.57735f };
	mMainPassCB.Lights[0].Strength = { 0.8f, 0.8f, 0.8f };
	mMainPassCB.Lights[1].Direction = { -0.57735f, -0.57735f, 0.57735f };
	mMainPassCB.Lights[1].Strength = { 0.4f, 0.4f, 0.4f };
	mMainPassCB.Lights[2].Direction = { 0.0f, -0.707f, -0.707f };
	mMainPassCB.Lights[2].Strength = { 0.2f, 0.2f, 0.2f };

	auto currPassCB = mCurrFrameResource->PassCB.get();
	currPassCB->CopyData(0, mMainPassCB);
}

void InstancingAndCullingApp::UpdateObjectCBs(const GameTimer & gt)
{
	auto currObjectCB = mCurrFrameResource->ObjectCB.get();
	for (auto& e : mAllRitems)
	{
		// Only update the cbuffer data if the constants have changed.  
		// This needs to be tracked per frame resource.
	/*	if (e->NumFramesDirty > 0)
		{*/
			//e->World._42 += 0.01;
			XMMATRIX world = XMLoadFloat4x4(&e->World);
			
			XMMATRIX texTransform = XMLoadFloat4x4(&e->TexTransform);

			ObjectConstants objConstants;
			XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(world));
			XMStoreFloat4x4(&objConstants.TexTransform, XMMatrixTranspose(texTransform));

			currObjectCB->CopyData(e->ObjCBIndex, objConstants);

			// Next FrameResource need to be updated too.
			//e->NumFramesDirty--;
		//}
	}
}

void InstancingAndCullingApp::UpdateMaterialCBs(const GameTimer & gt)
{
	auto currMaterialCB = mCurrFrameResource->MaterialCB.get();
	for (auto& e : mMaterials)
	{
		// Only update the cbuffer data if the constants have changed.  If the cbuffer
		// data changes, it needs to be updated for each FrameResource.
		Material* mat = e.second.get();
	/*	if (mat->NumFramesDirty > 0)
		{*/
			XMMATRIX matTransform = XMLoadFloat4x4(&mat->MatTransform);

			MaterialConstants matConstants;
			matConstants.DiffuseAlbedo = mat->DiffuseAlbedo;
			matConstants.FresnelR0 = mat->FresnelR0;
			matConstants.Roughness = mat->Roughness;
			XMStoreFloat4x4(&matConstants.MatTransform, XMMatrixTranspose(matTransform));

			currMaterialCB->CopyData(mat->MatCBIndex, matConstants);

			// Next FrameResource need to be updated too.
			//mat->NumFramesDirty--;
		//}
	}

}

void InstancingAndCullingApp::LoadTextures()
{
	auto bricksTex = std::make_unique<Texture>();
	bricksTex->Name = "bricksTex";
	bricksTex->Filename = L"../../Textures/bricks.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
		mCommandList.Get(), bricksTex->Filename.c_str(),
		bricksTex->Resource, bricksTex->UploadHeap));

	auto stoneTex = std::make_unique<Texture>();
	stoneTex->Name = "stoneTex";
	stoneTex->Filename = L"../../Textures/stone.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
		mCommandList.Get(), stoneTex->Filename.c_str(),
		stoneTex->Resource, stoneTex->UploadHeap));

	auto tileTex = std::make_unique<Texture>();
	tileTex->Name = "tileTex";
	tileTex->Filename = L"../../Textures/tile.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
		mCommandList.Get(), tileTex->Filename.c_str(),
		tileTex->Resource, tileTex->UploadHeap));

	auto crateTex = std::make_unique<Texture>();
	crateTex->Name = "crateTex";
	crateTex->Filename = L"../../Textures/WoodCrate01.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
		mCommandList.Get(), crateTex->Filename.c_str(),
		crateTex->Resource, crateTex->UploadHeap));

	auto iceTex = std::make_unique<Texture>();
	iceTex->Name = "iceTex";
	iceTex->Filename = L"../../Textures/ice.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
		mCommandList.Get(), iceTex->Filename.c_str(),
		iceTex->Resource, iceTex->UploadHeap));

	auto grassTex = std::make_unique<Texture>();
	grassTex->Name = "grassTex";
	grassTex->Filename = L"../../Textures/grass.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
		mCommandList.Get(), grassTex->Filename.c_str(),
		grassTex->Resource, grassTex->UploadHeap));

	auto SpiderTex = std::make_unique<Texture>();
	SpiderTex->Name = "SpiderTex";
	SpiderTex->Filename = L"../../Textures/spider.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
		mCommandList.Get(), SpiderTex->Filename.c_str(),
		SpiderTex->Resource, SpiderTex->UploadHeap));

	/*auto defaultTex = std::make_unique<Texture>();
	defaultTex->Name = "defaultTex";
	defaultTex->Filename = L"../../Textures/white1x1.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
		mCommandList.Get(), defaultTex->Filename.c_str(),
		defaultTex->Resource, defaultTex->UploadHeap));*/

	auto InsecTex = std::make_unique<Texture>();
	InsecTex->Name = "InsecTex";
	InsecTex->Filename = L"../../Textures/villager02_diff.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
		mCommandList.Get(), InsecTex->Filename.c_str(),
		InsecTex->Resource, InsecTex->UploadHeap));

	auto SkyTex = std::make_unique<Texture>();
	SkyTex->Name = "SkyTex";
	SkyTex->Filename = L"../../Textures/grasscube1024.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
	mCommandList.Get(), SkyTex->Filename.c_str(),
		SkyTex->Resource, SkyTex->UploadHeap));

	auto FenceTex = std::make_unique<Texture>();
	FenceTex->Name = "FenceTex";
	FenceTex->Filename = L"../../Textures/WireFence.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
		mCommandList.Get(), FenceTex->Filename.c_str(),
		FenceTex->Resource, FenceTex->UploadHeap));

	auto DragonTex = std::make_unique<Texture>();
	DragonTex->Name = "DragonTex";
	DragonTex->Filename = L"../../Textures/DragonTex.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
		mCommandList.Get(), DragonTex->Filename.c_str(),
		DragonTex->Resource, DragonTex->UploadHeap));

	mMaterials_Instancing[bricksTex->Name] = std::move(bricksTex);
	mMaterials_Instancing[stoneTex->Name] = std::move(stoneTex);
	mMaterials_Instancing[tileTex->Name] = std::move(tileTex);
	mMaterials_Instancing[crateTex->Name] = std::move(crateTex);
	mMaterials_Instancing[iceTex->Name] = std::move(iceTex);
	mMaterials_Instancing[grassTex->Name] = std::move(grassTex);
	//mMaterials[defaultTex->Name] = std::move(defaultTex);
	mMaterials_Instancing[InsecTex->Name] = std::move(InsecTex);
	mMaterials_Instancing[SkyTex->Name] = std::move(SkyTex);
	mMaterials_Instancing[FenceTex->Name] = std::move(FenceTex);
	mMaterials_Instancing[SpiderTex->Name] = std::move(SpiderTex);
	mMaterials_Instancing[DragonTex->Name] = std::move(DragonTex);

}

void InstancingAndCullingApp::BuildRootSignature()
{

	// Instancing - Space 0

	// Default - Space 1


	CD3DX12_DESCRIPTOR_RANGE texTable; //인스턴싱 텍스쳐
	texTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 7, 2 ,0 );

	CD3DX12_DESCRIPTOR_RANGE texTable0; //스카이박스
	texTable0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, 1);

	CD3DX12_DESCRIPTOR_RANGE texTable1; //디폴트 텍스쳐
	texTable1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 3, 2, 1);

	// Root parameter can be a table, root descriptor or root constants.
	CD3DX12_ROOT_PARAMETER slotRootParameter[8];

	// Perfomance TIP: Order from most frequent to least frequent.
	slotRootParameter[0].InitAsShaderResourceView(0, 0);
	slotRootParameter[1].InitAsShaderResourceView(1, 0);
	slotRootParameter[2].InitAsConstantBufferView(0);
	slotRootParameter[3].InitAsDescriptorTable(1, &texTable, D3D12_SHADER_VISIBILITY_PIXEL);

	//////////////////////////////////////////////////////
	slotRootParameter[4].InitAsConstantBufferView(0,1); //상수버퍼
	slotRootParameter[5].InitAsShaderResourceView(0,1); //Default 재질 스트럭쳐 버퍼로
	//slotRootParameter[5].InitAsConstantBufferView(1, 1); //Default 재질 스트럭쳐 버퍼로

	//////////////////////////////////////////////////////
	slotRootParameter[6].InitAsDescriptorTable(1, &texTable0, D3D12_SHADER_VISIBILITY_PIXEL);
	slotRootParameter[7].InitAsDescriptorTable(1, &texTable1, D3D12_SHADER_VISIBILITY_PIXEL);

	auto staticSamplers = GetStaticSamplers();

	// A root signature is an array of root parameters.
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(8, slotRootParameter,
		(UINT)staticSamplers.size(), staticSamplers.data(),
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	// create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

	if (errorBlob != nullptr)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}
	ThrowIfFailed(hr);

	ThrowIfFailed(md3dDevice->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(mRootSignature.GetAddressOf())));
}

void InstancingAndCullingApp::BuildDescriptorHeaps()
{
	//
	// Create the SRV heap.
	//
	ComPtr<ID3D12DescriptorHeap> Heap;


	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc2 = {}; //Default Texture
	srvHeapDesc2.NumDescriptors = 6;
	srvHeapDesc2.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc2.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	ThrowIfFailed(md3dDevice->CreateDescriptorHeap(&srvHeapDesc2, IID_PPV_ARGS(&mSrvDescriptorHeap[HEAP_DEFAULT])));


	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {}; //Instancing Texture
	srvHeapDesc.NumDescriptors = 6;
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	ThrowIfFailed(md3dDevice->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&mSrvDescriptorHeap[HEAP_INSTANCING])));

	//
	// Fill out the heap with actual descriptors.
	//
	CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor_Instancing(mSrvDescriptorHeap[HEAP_INSTANCING]->GetCPUDescriptorHandleForHeapStart()); // 인스턴싱

	CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor_Default(mSrvDescriptorHeap[HEAP_DEFAULT]->GetCPUDescriptorHandleForHeapStart()); // 디폴트

	auto bricksTex = mMaterials_Instancing["bricksTex"]->Resource;
	auto stoneTex = mMaterials_Instancing["stoneTex"]->Resource;
	auto tileTex = mMaterials_Instancing["tileTex"]->Resource;
	auto crateTex = mMaterials_Instancing["crateTex"]->Resource;
	auto iceTex = mMaterials_Instancing["iceTex"]->Resource;
	auto grassTex = mMaterials_Instancing["grassTex"]->Resource;
	//auto defaultTex = mMaterials["defaultTex"]->Resource;
	//auto skyTex = mMaterials["skyCubeMap"]->Resource;
	auto InsecTex = mMaterials_Instancing["InsecTex"]->Resource;
	auto SkyTex = mMaterials_Instancing["SkyTex"]->Resource;
	auto FenceTex = mMaterials_Instancing["FenceTex"]->Resource;
	auto SpiderTex = mMaterials_Instancing["SpiderTex"]->Resource;
	auto DragonTex = mMaterials_Instancing["DragonTex"]->Resource;



	// Instancing 

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc_Instancing = {};
	srvDesc_Instancing.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc_Instancing.Format = bricksTex->GetDesc().Format;
	srvDesc_Instancing.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc_Instancing.Texture2D.MostDetailedMip = 0;
	srvDesc_Instancing.Texture2D.MipLevels = bricksTex->GetDesc().MipLevels;
	srvDesc_Instancing.Texture2D.ResourceMinLODClamp = 0.0f;
	md3dDevice->CreateShaderResourceView(bricksTex.Get(), &srvDesc_Instancing, hDescriptor_Instancing);

	// next descriptor 1
	hDescriptor_Instancing.Offset(1, mCbvSrvDescriptorSize);

	srvDesc_Instancing.Format = stoneTex->GetDesc().Format;
	srvDesc_Instancing.Texture2D.MipLevels = stoneTex->GetDesc().MipLevels;
	md3dDevice->CreateShaderResourceView(stoneTex.Get(), &srvDesc_Instancing, hDescriptor_Instancing);

	// next descriptor 2
	hDescriptor_Instancing.Offset(1, mCbvSrvDescriptorSize);

	srvDesc_Instancing.Format = tileTex->GetDesc().Format;
	srvDesc_Instancing.Texture2D.MipLevels = tileTex->GetDesc().MipLevels;
	md3dDevice->CreateShaderResourceView(tileTex.Get(), &srvDesc_Instancing, hDescriptor_Instancing);

	// next descriptor 3
	hDescriptor_Instancing.Offset(1, mCbvSrvDescriptorSize);

	srvDesc_Instancing.Format = crateTex->GetDesc().Format;
	srvDesc_Instancing.Texture2D.MipLevels = crateTex->GetDesc().MipLevels;
	md3dDevice->CreateShaderResourceView(crateTex.Get(), &srvDesc_Instancing, hDescriptor_Instancing);

	// next descriptor 4
	hDescriptor_Instancing.Offset(1, mCbvSrvDescriptorSize);

	srvDesc_Instancing.Format = iceTex->GetDesc().Format;
	srvDesc_Instancing.Texture2D.MipLevels = iceTex->GetDesc().MipLevels;
	md3dDevice->CreateShaderResourceView(iceTex.Get(), &srvDesc_Instancing, hDescriptor_Instancing);

	// next descriptor 5
	hDescriptor_Instancing.Offset(1, mCbvSrvDescriptorSize);

	srvDesc_Instancing.Format = grassTex->GetDesc().Format;
	srvDesc_Instancing.Texture2D.MipLevels = grassTex->GetDesc().MipLevels;
	md3dDevice->CreateShaderResourceView(grassTex.Get(), &srvDesc_Instancing, hDescriptor_Instancing);

	////////////////////////

	//Default Player
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc_Default = {};
	srvDesc_Default.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc_Default.Format = InsecTex->GetDesc().Format;
	srvDesc_Default.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc_Default.Texture2D.MostDetailedMip = 0;
	srvDesc_Default.Texture2D.MipLevels = InsecTex->GetDesc().MipLevels;
	srvDesc_Default.Texture2D.ResourceMinLODClamp = 0.0f;
	md3dDevice->CreateShaderResourceView(InsecTex.Get(), &srvDesc_Default, hDescriptor_Default);

	// next descriptor 1 Barrel
	hDescriptor_Default.Offset(1, mCbvSrvDescriptorSize);

	srvDesc_Default.Format = FenceTex->GetDesc().Format;
	srvDesc_Default.Texture2D.MipLevels = FenceTex->GetDesc().MipLevels;
	md3dDevice->CreateShaderResourceView(FenceTex.Get(), &srvDesc_Default, hDescriptor_Default);

	// next descriptor 2 Terrain
	hDescriptor_Default.Offset(1, mCbvSrvDescriptorSize);

	srvDesc_Default.Format = stoneTex->GetDesc().Format;
	srvDesc_Default.Texture2D.MipLevels = stoneTex->GetDesc().MipLevels;
	md3dDevice->CreateShaderResourceView(stoneTex.Get(), &srvDesc_Default, hDescriptor_Default);
	
	// next descriptor 3 Spdier
	hDescriptor_Default.Offset(1, mCbvSrvDescriptorSize);

	srvDesc_Default.Format = SpiderTex->GetDesc().Format;
	srvDesc_Default.Texture2D.MipLevels = SpiderTex->GetDesc().MipLevels;
	md3dDevice->CreateShaderResourceView(SpiderTex.Get(), &srvDesc_Default, hDescriptor_Default);

	// next descriptor 4 Dragon
	hDescriptor_Default.Offset(1, mCbvSrvDescriptorSize);

	srvDesc_Default.Format = DragonTex->GetDesc().Format;
	srvDesc_Default.Texture2D.MipLevels = DragonTex->GetDesc().MipLevels;
	md3dDevice->CreateShaderResourceView(DragonTex.Get(), &srvDesc_Default, hDescriptor_Default);

	// next descriptor 5 SkyBox
	hDescriptor_Default.Offset(1, mCbvSrvDescriptorSize);

	srvDesc_Default.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
	srvDesc_Default.TextureCube.MostDetailedMip = 0;
	srvDesc_Default.TextureCube.MipLevels = SkyTex->GetDesc().MipLevels;
	srvDesc_Default.TextureCube.ResourceMinLODClamp = 0.0f;
	srvDesc_Default.Format = SkyTex->GetDesc().Format;
	md3dDevice->CreateShaderResourceView(SkyTex.Get(), &srvDesc_Default, hDescriptor_Default);

	//mSkyTexHeapIndex = 7;
	
}

void InstancingAndCullingApp::BuildShadersAndInputLayout()
{
	const D3D_SHADER_MACRO defines[] =
	{
		"FOG", "1",
		NULL, NULL
	};

	const D3D_SHADER_MACRO alphaTestDefines[] =
	{
		"FOG", "1",
		"ALPHA_TEST", "1",
		NULL, NULL
	};

	mShaders["InstancingVS"] = d3dUtil::CompileShader(L"Shaders\\Instancing.hlsl", nullptr, "VS", "vs_5_1");
	mShaders["InstancingPS"] = d3dUtil::CompileShader(L"Shaders\\Instancing.hlsl", nullptr, "PS", "ps_5_1");

	mShaders["ObjectVS"] = d3dUtil::CompileShader(L"Shaders\\Default.hlsl", nullptr, "VS", "vs_5_1");
	mShaders["ObjectPS"] = d3dUtil::CompileShader(L"Shaders\\Default.hlsl", nullptr, "PS", "ps_5_1");

	mShaders["skyVS"] = d3dUtil::CompileShader(L"Shaders\\Sky.hlsl", nullptr, "VS", "vs_5_1");
	mShaders["skyPS"] = d3dUtil::CompileShader(L"Shaders\\Sky.hlsl", nullptr, "PS", "ps_5_1");


	mInputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};
}

void InstancingAndCullingApp::BuildSkullGeometry()
{
	AnimInfo tAnimInfo;

	UINT vcount = 0;
	UINT tcount = 0;
	std::string ignore;
	bool countStart = false;
	std::ifstream fin("Models/SpiderAnim/spider_Idle.ASE", ios::binary);


	if (!fin)
	{
		exit(-1);
	}

	//애니메이션 갯수 세기
	while (!fin.eof())
	{
		fin >> ignore;

		if (ignore == "*TIMEVALUE" && countStart)
		{
			//정점 좌표 읽어오기
			Character curCharacter;
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
			//인덱스 읽어오기
			fin >> ignore; //}
			fin >> ignore; //*MESH_FACE_LIST 
			fin >> ignore; // {

			for (int i = 0; i < curCharacter.iNumIndex; ++i)
			{
				fin >> ignore; //*MESH_FACE 
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
				fin >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore;
			}

			//텍스쳐 좌표 읽어오기
			fin >> ignore; // }
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

			//텍스쳐 좌표 인덱스 읽어오기
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
				fin >> ignore; // 계속읽어

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

			tAnimInfo.mapAnimationModel.push_back(curCharacter);
		}
		if (ignore == "*MESH_ANIMATION")
		{
			countStart = true;
		}
	}
	fin.close();

	tAnimInfo.iAnimationFrameSize = (int)tAnimInfo.mapAnimationModel.size();
	m_vecAnimPerFrameSize.push_back((int)(tAnimInfo.mapAnimationModel.size()));

	//m_mapAnimations[const_cast<wchar_t*>(AnimName)] = tAnimInfo;


	///////////////////////////////////////////////////////////////////////////////////////


	std::vector<Vertex> vertices;
	std::vector<std::int32_t> indices;

	int nAnimVertexoffset = 0;
	int nAnimIndexoffset = 0;

	//Idle 애니메이션 정점, 인덱스 초기화
	int nVertexoffset = 0;
	int nIndexoffset = 0;

	//Idle 버텍스 / 인덱스 오프셋 셋팅
	auto iter = tAnimInfo.mapAnimationModel.begin();
	auto iter_end = tAnimInfo.mapAnimationModel.end();

	for (iter; iter != iter_end; ++iter)
	{
		m_vecVertexOffset.push_back(nVertexoffset);
		nVertexoffset += (int)((*iter).uv.size());

		m_vecIndexOffset.push_back(nIndexoffset);
		nIndexoffset += (*iter).iNumIndex * 3;
	}

	//Idle 애니메이션 버텍스 넣음
	iter = tAnimInfo.mapAnimationModel.begin();
	for (iter; iter != iter_end; ++iter)
	{
		for (int i = 0; i < (*iter).realvecVertex/*vecVertex*/.size(); ++i)
		{
			vertices.push_back((*iter).realvecVertex/*vecVertex*/[i]);
		}
	}

	//Idle 인덱스 셋팅
	int iSize = (int)tAnimInfo.mapAnimationModel.size();

	for (int i = 0; i < iSize; ++i)
	{
		indices.insert(indices.end(), std::begin(tAnimInfo.mapAnimationModel[i].uvIndex/*vecIndex*/), std::end(tAnimInfo.mapAnimationModel[i].uvIndex/*vecIndex*/));
	}



	size_t indexTest = indices.size();

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);

	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::int32_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "SpiderGeo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R32_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	SubmeshGeometry submesh;
	submesh.IndexCount = (UINT)indexTest;//(UINT)indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	geo->DrawArgs["Spider"] = submesh;

	mGeometries[geo->Name] = std::move(geo);


	
}

void InstancingAndCullingApp::BuildBarrelGeometry()
{
	std::ifstream fin("Models/StaticMesh/staticMesh.ASE");

	if (!fin)
	{
		exit(-1);
	}

	UINT vcount = 0;
	UINT tcount = 0;
	std::string ignore;
	Character curCharacter;


	//애니메이션 갯수 세기
	while (!fin.eof())
	{
		fin >> ignore;

		if (ignore == "*TIMEVALUE")
		{
			//정점 좌표 읽어오기
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
			//인덱스 읽어오기
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

			//텍스쳐 좌표 읽어오기
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

			//텍스쳐 좌표 인덱스 읽어오기
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
				fin >> ignore; // 계속읽어

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

	size_t indexTest = curCharacter.uvIndex.size();

	const UINT vbByteSize = (UINT)curCharacter.realvecVertex.size() * sizeof(Vertex);

	const UINT ibByteSize = (UINT)curCharacter.uvIndex.size() * sizeof(std::int32_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "BarrelGeo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), curCharacter.realvecVertex.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), curCharacter.uvIndex.data(), ibByteSize);

	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), curCharacter.realvecVertex.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
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

	mGeometries[geo->Name] = std::move(geo);


}

void InstancingAndCullingApp::BuildPSOs()
{
	
	D3D12_GRAPHICS_PIPELINE_STATE_DESC opaquePsoDesc;

	//
	// PSO for opaque objects.
	//
	ZeroMemory(&opaquePsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	opaquePsoDesc.InputLayout = { mInputLayout.data(), (UINT)mInputLayout.size() };
	opaquePsoDesc.pRootSignature = mRootSignature.Get();
	opaquePsoDesc.VS =
	{
		reinterpret_cast<BYTE*>(mShaders["ObjectVS"]->GetBufferPointer()),
		mShaders["ObjectVS"]->GetBufferSize()
	};
	opaquePsoDesc.PS =
	{
		reinterpret_cast<BYTE*>(mShaders["ObjectPS"]->GetBufferPointer()),
		mShaders["ObjectPS"]->GetBufferSize()
	};
	opaquePsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	opaquePsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	opaquePsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	opaquePsoDesc.SampleMask = UINT_MAX;
	opaquePsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	opaquePsoDesc.NumRenderTargets = 1;
	opaquePsoDesc.RTVFormats[0] = mBackBufferFormat;
	opaquePsoDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
	opaquePsoDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
	opaquePsoDesc.DSVFormat = mDepthStencilFormat;
	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&opaquePsoDesc, IID_PPV_ARGS(&mPSOs["Opaque"])));

	//
	// PSO for opaque objects.
	//

	D3D12_GRAPHICS_PIPELINE_STATE_DESC InstancingOpaquePsoDesc = opaquePsoDesc;
	InstancingOpaquePsoDesc.VS =
	{
		reinterpret_cast<BYTE*>(mShaders["InstancingVS"]->GetBufferPointer()),
		mShaders["InstancingVS"]->GetBufferSize()
	};
	InstancingOpaquePsoDesc.PS =
	{
		reinterpret_cast<BYTE*>(mShaders["InstancingPS"]->GetBufferPointer()),
		mShaders["InstancingPS"]->GetBufferSize()
	};

	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&InstancingOpaquePsoDesc, IID_PPV_ARGS(&mPSOs["InstancingOpaque"])));

	//
	// PSO for sky.
	//
	D3D12_GRAPHICS_PIPELINE_STATE_DESC skyPsoDesc = opaquePsoDesc;

	// The camera is inside the sky sphere, so just turn off culling.
	skyPsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE; //카메라가 구 내부에 있음으로 후면선별을 끔 


																// Make sure the depth function is LESS_EQUAL and not just LESS.  
																// Otherwise, the normalized depth values at z = 1 (NDC) will 
																// fail the depth test if the depth buffer was cleared to 1.
	skyPsoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL; //깊이판정 LESS_EQUAL해야 구가 깊이판정 통과함
	skyPsoDesc.pRootSignature = mRootSignature.Get();

	skyPsoDesc.VS =
	{
		reinterpret_cast<BYTE*>(mShaders["skyVS"]->GetBufferPointer()),
		mShaders["skyVS"]->GetBufferSize()
	};
	skyPsoDesc.PS =
	{
		reinterpret_cast<BYTE*>(mShaders["skyPS"]->GetBufferPointer()),
		mShaders["skyPS"]->GetBufferSize()
	};
	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&skyPsoDesc, IID_PPV_ARGS(&mPSOs["sky"])));

	//
	// PSO for transparent objects
	//

	D3D12_GRAPHICS_PIPELINE_STATE_DESC transparentPsoDesc = opaquePsoDesc;

	D3D12_RENDER_TARGET_BLEND_DESC transparencyBlendDesc;
	transparencyBlendDesc.BlendEnable = true;
	transparencyBlendDesc.LogicOpEnable = false;
	transparencyBlendDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
	transparencyBlendDesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	transparencyBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;
	transparencyBlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
	transparencyBlendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
	transparencyBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	transparencyBlendDesc.LogicOp = D3D12_LOGIC_OP_NOOP;
	transparencyBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	transparentPsoDesc.BlendState.RenderTarget[0] = transparencyBlendDesc;
	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&transparentPsoDesc, IID_PPV_ARGS(&mPSOs["transparent"])));


}

void InstancingAndCullingApp::BuildFrameResources()
{
	for (int i = 0; i < gNumFrameResources; ++i)
	{
		mFrameResources.push_back(std::make_unique<FrameResource>(md3dDevice.Get(),
			1, (UINT)mAllRitems.size() + 1, (UINT)mMaterials.size()));
	}
}

void InstancingAndCullingApp::BuildMaterials()
{
	///////////////////////////////////////////////////////////////////// Instancing.hlsl
	auto bricks0 = std::make_unique<Material>();
	bricks0->Name = "bricks0";
	bricks0->MatCBIndex = 0;
	bricks0->DiffuseSrvHeapIndex = 0;
	bricks0->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	bricks0->FresnelR0 = XMFLOAT3(0.02f, 0.02f, 0.02f);
	bricks0->Roughness = 0.1f;

	auto stone0 = std::make_unique<Material>();
	stone0->Name = "stone0";
	stone0->MatCBIndex = 1;
	stone0->DiffuseSrvHeapIndex = 1;
	stone0->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	stone0->FresnelR0 = XMFLOAT3(0.05f, 0.05f, 0.05f);
	stone0->Roughness = 0.3f;

	auto tile0 = std::make_unique<Material>();
	tile0->Name = "tile0";
	tile0->MatCBIndex = 2;
	tile0->DiffuseSrvHeapIndex = 2;
	tile0->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	tile0->FresnelR0 = XMFLOAT3(0.02f, 0.02f, 0.02f);
	tile0->Roughness = 0.3f;

	auto crate0 = std::make_unique<Material>();
	crate0->Name = "checkboard0";
	crate0->MatCBIndex = 3;
	crate0->DiffuseSrvHeapIndex = 3;
	crate0->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	crate0->FresnelR0 = XMFLOAT3(0.05f, 0.05f, 0.05f);
	crate0->Roughness = 0.2f;

	auto ice0 = std::make_unique<Material>();
	ice0->Name = "ice0";
	ice0->MatCBIndex = 4;
	ice0->DiffuseSrvHeapIndex = 4;
	ice0->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	ice0->FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
	ice0->Roughness = 0.0f;

	auto grass0 = std::make_unique<Material>();
	grass0->Name = "grass0";
	grass0->MatCBIndex = 5;
	grass0->DiffuseSrvHeapIndex = 5;
	grass0->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	grass0->FresnelR0 = XMFLOAT3(0.05f, 0.05f, 0.05f);
	grass0->Roughness = 0.2f;
	
	///////////////////////////////////////////////////////////////////// Default.hlsl
	auto skullMat = std::make_unique<Material>();
	skullMat->Name = "skullMat";
	skullMat->MatCBIndex = 6;
	skullMat->DiffuseSrvHeapIndex = 6;
	skullMat->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	skullMat->FresnelR0 = XMFLOAT3(0.05f, 0.05f, 0.05f);
	skullMat->Roughness = 0.5f;


	///////////////////////////////////////////////////////////////////// Sky.hlsl
	auto sky = std::make_unique<Material>();
	sky->Name = "sky";
	sky->MatCBIndex = 7;
	sky->DiffuseSrvHeapIndex = 7;
	sky->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	sky->FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
	sky->Roughness = 1.0f;

	mMaterials["bricks0"] = std::move(bricks0);
	mMaterials["stone0"] = std::move(stone0);
	mMaterials["tile0"] = std::move(tile0);
	mMaterials["crate0"] = std::move(crate0);
	mMaterials["ice0"] = std::move(ice0);
	mMaterials["grass0"] = std::move(grass0);

	////////////////////////////////////////////////////////
	mMaterials["skullMat"] = std::move(skullMat);
	mMaterials["sky"] = std::move(sky);
}

void InstancingAndCullingApp::BuildRenderItems()
{
	auto skullRitem = std::make_unique<RenderItem>();
	mAllRitems.push_back(std::move(skullRitem));

	auto SpiderRitem = std::make_unique<RenderItem>();
	mAllRitems.push_back(std::move(SpiderRitem));

	auto BarrelRitem = std::make_unique<RenderItem>();
	mAllRitems.push_back(std::move(BarrelRitem));

	auto TerrainRitem = std::make_unique<RenderItem>();
	mAllRitems.push_back(std::move(TerrainRitem));


	auto skyRitem = std::make_unique<RenderItem>();
	mAllRitems.push_back(std::move(skyRitem));

	auto TerrainRitem1 = std::make_unique<RenderItem>();
	mAllRitems.push_back(std::move(TerrainRitem1));

	auto DragonRiTem = std::make_unique<RenderItem>();
	mAllRitems.push_back(std::move(DragonRiTem));

	// All the render items are opaque.
	for (auto& e : mAllRitems)
		mOpaqueRitems.push_back(e.get());

	CScene* pScene = CTestScene::Create(md3dDevice, mSrvDescriptorHeap, mCbvSrvDescriptorSize);
	CManagement::GetInstance()->Change_Scene(pScene);
}

void InstancingAndCullingApp::DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems )
{


	/*ID3D12DescriptorHeap* descriptorHeaps[] = { mSrvDescriptorHeap_InstancingTex.Get() };
	mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);*/


	CManagement::GetInstance()->Render(cmdList);

	//mCommandList->SetPipelineState(mPSOs["Opaque"].Get());

	//m_vecObjects[0]->Render(cmdList); // 거미

	//m_vecObjects[1]->Render(cmdList); // 배럴

	//m_vecObjects[2]->Render(cmdList); // 지형

	//mCommandList->SetPipelineState(mPSOs["sky"].Get());
	//m_vecObjects[3]->Render(cmdList); // 스카이박스


	//ID3D12DescriptorHeap* descriptorHeaps2[] = { mSrvDescriptorHeap_InstancingTex.Get() };
	//mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps2), descriptorHeaps2);

	//mCommandList->SetPipelineState(mPSOs["InstancingOpaque"].Get());
	//m_vecObjects[4]->Render(cmdList);

}

std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> InstancingAndCullingApp::GetStaticSamplers()
{
	// Applications usually only need a handful of samplers.  So just define them all up front
	// and keep them available as part of the root signature.  

	const CD3DX12_STATIC_SAMPLER_DESC pointWrap(
		0, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC pointClamp(
		1, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC linearWrap(
		2, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC linearClamp(
		3, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicWrap(
		4, // shaderRegister
		D3D12_FILTER_ANISOTROPIC, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressW
		0.0f,                             // mipLODBias
		8);                               // maxAnisotropy

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicClamp(
		5, // shaderRegister
		D3D12_FILTER_ANISOTROPIC, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressW
		0.0f,                              // mipLODBias
		8);                                // maxAnisotropy

	return {
		pointWrap, pointClamp,
		linearWrap, linearClamp,
		anisotropicWrap, anisotropicClamp };
}

void InstancingAndCullingApp::BuildInsec()
{
}

