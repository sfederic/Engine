#include "D3D12RenderSystem.h"
#include "CoreSystem.h"
#include "ShaderFactory.h"
#include "World.h"
#include "Camera.h"
#include "IBuffer.h"
#include "ITexture.h"

void D3D12RenderSystem::Tick()
{
}

ID3DBlob* D3D12RenderSystem::CreateShaderFromFile(const wchar_t* filename, const char* entry, const char* target)
{
	UINT compileFlags = 0;
#ifdef _DEBUG
	compileFlags = D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_DEBUG;
#endif
	ID3DBlob* code;
	ID3DBlob* error;

	D3DCompileFromFile(filename, nullptr, nullptr, entry, target, compileFlags, 0, &code, &error);
	if (error)
	{
		const wchar_t* errMsg = (wchar_t*)error->GetBufferPointer();
		//OutputDebugString(errMsg);
		//MessageBox(0, errMsg, entry, 0);
	}

	return code;
}

void D3D12RenderSystem::CreateShaders()
{
	//TODO: can just never get this shit to work.
	//DXC COMPILER EXAMPLE
	/*HR(DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&dxcLibrary)));
	HR(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler)));

	const wchar_t* pArgs[] =
	{
		L"-Zpr",			//Row-major matrices
		L"-WX",				//Warnings as errors
#ifdef _DEBUG
		L"-Zi",				//Debug info
		L"-Qembed_debug",	//Embed debug info into the shader
		L"-Od",				//Disable optimization
#else
		L"-O3",				//Optimization level 3
#endif
	};

	std::vector<DxcDefine> defines;

	IDxcOperationResult* result = nullptr;
	HR(dxcCompiler->Compile(vertexCode, L"Shaders/shaders.hlsl", L"VSMain", L"vs_6_0", &pArgs[0], sizeof(pArgs) / sizeof(pArgs[0]), defines.data(), defines.size(), nullptr, &result));
	HR(result->GetResult(&vertexCode));

	HR(dxcCompiler->Compile(pixelCode, L"Shaders/shaders.hlsl", L"PSMain", L"ps_6_0", &pArgs[0], sizeof(pArgs) / sizeof(pArgs[0]), nullptr, 0, nullptr, &result));
	HR(result->GetResult(&pixelCode));*/

	UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
	ID3DBlob* error = nullptr;

	HR(D3DCompileFromFile(L"Shaders/shaders.hlsl", nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &vertexCode, &error));
	if (error)
	{
		LPCSTR errMsg = (LPCSTR)error->GetBufferPointer();
		OutputDebugString(errMsg);
		MessageBox(0, errMsg, "VSMain", 0);
	}
	error = nullptr;
	HR(D3DCompileFromFile(L"Shaders/shaders.hlsl", nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &pixelCode, &error));
	if (error)
	{
		LPCSTR errMsg = (LPCSTR)error->GetBufferPointer();
		OutputDebugString(errMsg);
		MessageBox(0, errMsg, "PSMain", 0);
	}
}

void D3D12RenderSystem::Init(HWND window)
{
	//Viewport and rect
	viewport = { 0.f, 0.f, (float)gCoreSystem.windowWidth, (float)gCoreSystem.windowHeight, 0.f, 1.f };
	scissorRect = { 0, 0, gCoreSystem.windowWidth, gCoreSystem.windowHeight };

	ID3D12Debug* debug;
	HR(D3D12GetDebugInterface(IID_PPV_ARGS(&debug)));
	debug->EnableDebugLayer();
	debug->Release();

	//DEVICE & FACTORY
	IDXGIFactory* tmpFactory = nullptr;
	HR(CreateDXGIFactory(IID_PPV_ARGS(&tmpFactory)));
	tmpFactory->QueryInterface(&factory);
	tmpFactory->Release();

	HR(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&device)));

	//CMD Q
	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {};
	cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	HR(device->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&cmdQueue)));

	//SWAPCHAIN
	DXGI_SWAP_CHAIN_DESC sd = {};
	sd.BufferDesc = { (UINT)gCoreSystem.windowWidth, (UINT)gCoreSystem.windowHeight, {60, 1}, DXGI_FORMAT_R8G8B8A8_UNORM };
	sd.Windowed = TRUE;
	sd.SampleDesc.Count = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = window;
	sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	sd.BufferCount = swapchainCount;

	IDXGISwapChain* tmpSwapchain;
	HR(factory->CreateSwapChain(cmdQueue, &sd, &tmpSwapchain));
	tmpSwapchain->QueryInterface<IDXGISwapChain3>(&swapchain);
	currentBackBufferIndex = swapchain->GetCurrentBackBufferIndex();

	//DESC HEAPS
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = swapchainCount;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	HR(device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvHeap)));

	rtvHeapSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	assert(rtvHeapSize > 0);

	//CONSTANT DESC HEAP
	D3D12_DESCRIPTOR_HEAP_DESC cbHeapDesc = {};
	cbHeapDesc.NumDescriptors = 2;
	cbHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	cbHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	HR(device->CreateDescriptorHeap(&cbHeapDesc, IID_PPV_ARGS(&cbHeap)));


	//D3D11on12 (For D2D/DWrite)
	//NOTE: With D3D11on12 and all the debug information, some builds were giving 500MB for hello world-tier programs
	{
		D3D_FEATURE_LEVEL levels[] = {
			D3D_FEATURE_LEVEL_11_0
		};
		D3D_FEATURE_LEVEL level;

		UINT d3dDeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
		HR(D3D11On12CreateDevice(device, d3dDeviceFlags, levels, _countof(levels), (IUnknown**)&cmdQueue,
			1, 0, &d3d11Device, &d3d11DeviceContext, &level));
		HR(d3d11Device->QueryInterface(&d3d11On12Device));
	}

	InitD2D();

	//RTV
	for (int i = 0; i < swapchainCount; i++)
	{
		HR(swapchain->GetBuffer(i, IID_PPV_ARGS(&rtvs[i])));
		rtvHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();
		rtvHandle.ptr += (i * rtvHeapSize);
		device->CreateRenderTargetView(rtvs[i], nullptr, rtvHandle);
	}

	//CMD ALLOC
	HR(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&cmdAlloc)));

	//INPUT ELEMENTS	
	D3D12_INPUT_ELEMENT_DESC inputElements[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, pos), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(Vertex, uv), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, normal), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
	};

	//BLEND DESC
	D3D12_BLEND_DESC blendDesc = {};
	blendDesc.IndependentBlendEnable = false;
	blendDesc.RenderTarget[0].BlendEnable = false;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
	blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;

	//RASTERIZER
	D3D12_RASTERIZER_DESC rastDesc = {};
	rastDesc.FrontCounterClockwise = FALSE;
	rastDesc.CullMode = D3D12_CULL_MODE_BACK;
	rastDesc.FillMode = D3D12_FILL_MODE_SOLID;
	rastDesc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
	rastDesc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
	rastDesc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
	rastDesc.DepthClipEnable = TRUE;
	rastDesc.MultisampleEnable = FALSE;
	rastDesc.AntialiasedLineEnable = FALSE;
	rastDesc.ForcedSampleCount = 0;
	rastDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	//DEPTHSTENCIL
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc = {};
	depthStencilDesc.DepthEnable = TRUE;
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	depthStencilDesc.StencilEnable = FALSE;
	depthStencilDesc.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
	depthStencilDesc.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
	const D3D12_DEPTH_STENCILOP_DESC defaultStencilOp =
	{ D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_COMPARISON_FUNC_ALWAYS };
	depthStencilDesc.FrontFace = defaultStencilOp;
	depthStencilDesc.BackFace = defaultStencilOp;

	//Test shaders
	CreateShaders();

	//ROOT SIGNATURE
	//NOTE: Samplers are not allowed in the same descriptor table as CBV/UAV/SRVs

	D3D12_DESCRIPTOR_RANGE descRangeCBV = {};
	descRangeCBV.BaseShaderRegister = 0;
	descRangeCBV.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	descRangeCBV.NumDescriptors = 1;

	D3D12_DESCRIPTOR_RANGE descRangeSRV = {};
	descRangeSRV.BaseShaderRegister = 0;
	descRangeSRV.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descRangeSRV.NumDescriptors = 1;

	D3D12_ROOT_PARAMETER rootParamCBV = {};
	rootParamCBV.DescriptorTable = { 1, &descRangeCBV };
	rootParamCBV.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParamCBV.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	D3D12_ROOT_PARAMETER rootParamSRV = {};
	rootParamSRV.DescriptorTable = { 1, &descRangeSRV };
	rootParamSRV.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParamSRV.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_ROOT_PARAMETER rootParams[] =
	{
		rootParamCBV, rootParamSRV
	};

	//CREATE STATIC SAMPLER
	D3D12_STATIC_SAMPLER_DESC samplerDesc = {};
	samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
	samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	samplerDesc.ShaderRegister = 0;
	samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_ROOT_SIGNATURE_DESC rootDesc = {};
	rootDesc.pParameters = rootParams;
	rootDesc.NumParameters = _countof(rootParams);
	rootDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	rootDesc.NumStaticSamplers = 1;
	rootDesc.pStaticSamplers = &samplerDesc;

	ID3DBlob* rootBlob = nullptr;
	ID3DBlob* rootError = nullptr;
	HR(D3D12SerializeRootSignature(&rootDesc, D3D_ROOT_SIGNATURE_VERSION_1, &rootBlob, &rootError));
	if (rootError)
	{
		MessageBox(0, (char*)rootError->GetBufferPointer(), "Root error", 0);
	}
	HR(device->CreateRootSignature(0, rootBlob->GetBufferPointer(), rootBlob->GetBufferSize(), IID_PPV_ARGS(&rootSig)));

	//PIPELINE
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout = { inputElements, _countof(inputElements) };
    psoDesc.pRootSignature = rootSig;     
	psoDesc.VS = { reinterpret_cast<UINT8*>(vertexCode->GetBufferPointer()), vertexCode->GetBufferSize() };  
	psoDesc.PS = { reinterpret_cast<UINT8*>(pixelCode->GetBufferPointer()), pixelCode->GetBufferSize() };  
	psoDesc.RasterizerState = rastDesc;
	psoDesc.BlendState = blendDesc;
	psoDesc.DepthStencilState = depthStencilDesc;
	psoDesc.SampleMask = UINT_MAX;      
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;     
	psoDesc.NumRenderTargets = 1;    
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;   
	psoDesc.SampleDesc.Count = 1;     
	HR(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState)));

	//CMDLIST
	HR(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, cmdAlloc, pipelineState, IID_PPV_ARGS(&cmdList)));
	HR(cmdList->Close());

	HR(cmdAlloc->Reset());
	HR(cmdList->Reset(cmdAlloc, pipelineState));

	//FENCE
	HR(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));
	fenceVal = 1;

	fenceEvent = CreateEvent(nullptr, false, false, nullptr);

	//CONSTANT BUFFERS
	matrices.model = XMMatrixIdentity();
	matrices.view = XMMatrixIdentity();
	matrices.proj = XMMatrixPerspectiveFovLH(XM_PI / 3, gCoreSystem.GetAspectRatio(), 0.01f, 1000.f);

	editorCamera.proj = matrices.proj;
	matrices.mvp = matrices.model * matrices.view * matrices.proj;

	cbUploadBuffer = CreateConstantBuffer(sizeof(matrices), &matrices);
	constantBufferView.SizeInBytes = 256;
	constantBufferView.BufferLocation = cbUploadBuffer->GetGPUVirtualAddress();
	device->CreateConstantBufferView(&constantBufferView, cbHeap->GetCPUDescriptorHandleForHeapStart());
}

void D3D12RenderSystem::RenderSetup(float deltaTime)
{
}

void D3D12RenderSystem::RenderActorSystem(World* world)
{
	for (int i = 0; i < world->actorSystems.size(); i++)
	{
		ActorSystem* actorSystem = world->actorSystems[i];

		//TODO: change to actor system pso
		cmdList->SetPipelineState(pipelineState);

		//Constant buffer register values
		const int cbMatrixRegister = 0;
		const int cbMaterialRegister = 1;

		ID3D12Resource* vertexBufferResource = (ID3D12Resource*)actorSystem->pso.vertexBuffer->data;
		vertexBufferView.BufferLocation = vertexBufferResource->GetGPUVirtualAddress();
		vertexBufferView.SizeInBytes = actorSystem->pso.vertexBuffer->size;
		vertexBufferView.StrideInBytes = sizeof(Vertex);
		cmdList->IASetVertexBuffers(0, 1, &vertexBufferView);

		for (int i = 0; i < actorSystem->actors.size(); i++)
		{
			if (actorSystem->actors[i]->bRender)
			{
				//Set Matrix constant buffer
				matrices.view = GetActiveCamera()->view;
				matrices.model = actorSystem->actors[i]->transform;
				matrices.mvp = matrices.model * matrices.view * matrices.proj;

				UpdateBuffer(cbUploadBuffer.Get(), sizeof(matrices), &matrices);

				cmdList->DrawInstanced(actorSystem->modelData.verts.size(), 1, 0, 0);
			}
		}
	}
}

void D3D12RenderSystem::Render(float deltaTime)
{
	HR(cmdAlloc->Reset());
	HR(cmdList->Reset(cmdAlloc, pipelineState));

	cmdList->SetGraphicsRootSignature(rootSig);

	ID3D12DescriptorHeap* descHeaps[] =
	{
		cbHeap
	};
	cmdList->SetDescriptorHeaps(_countof(descHeaps), descHeaps);

	cmdList->SetGraphicsRootDescriptorTable(0, cbHeap->GetGPUDescriptorHandleForHeapStart());

	cmdList->RSSetViewports(1, &viewport);
	cmdList->RSSetScissorRects(1, &scissorRect);

	cmdList->ResourceBarrier(1, &ResourceBarrier::Transition(rtvs[currentBackBufferIndex],
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	rtvHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();
	rtvHandle.ptr += (currentBackBufferIndex * rtvHeapSize);
	cmdList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

	const float clearColour[4] = { 0.25f, 0.25f, 0.25f, 1.f };
	cmdList->ClearRenderTargetView(rtvHandle, clearColour, 0, nullptr);
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	RenderActorSystem(GetWorld());

	//Resource Barrier gone with D3D11on12, barrier is with wrappedBackBuffer creation
	cmdList->ResourceBarrier(1, &ResourceBarrier::Transition(rtvs[currentBackBufferIndex],
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));


	//UI
	//d3d11On12Device->AcquireWrappedResources(&wrappedBackBuffers[currentBackBufferIndex], 1);

	//I don't what the fuck this used to be
	/*d2dDeviceContext->SetTarget(d2dRenderTargets[currentBackBufferIndex]);
	d2dDeviceContext->BeginDraw();
	d2dDeviceContext->DrawTextA(L"hello", 5, textFormat, { 0.f, 0.f, 100.f, 100.f }, brushText);
	d2dDeviceContext->EndDraw();*/

	//d3d11On12Device->ReleaseWrappedResources(&wrappedBackBuffers[currentBackBufferIndex], 1);
	//d3d11DeviceContext->Flush();
}

void D3D12RenderSystem::RenderEnd(float deltaTime)
{
}

void D3D12RenderSystem::CreateShaderView(IShaderView* shaderView, ITexture* texture)
{
}

//TODO: shares some code with createvertexBuffer(). delete one or the other.
ID3D12Resource* D3D12RenderSystem::CreateDefaultBuffer(unsigned int size, const void* data, ID3D12Resource* uploadBuffer)
{
	D3D12_RESOURCE_DESC desc = {};
	desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	desc.DepthOrArraySize = 1;
	desc.Format = DXGI_FORMAT_UNKNOWN;
	desc.MipLevels = 1;
	desc.SampleDesc = { 1, 0 };
	desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	desc.Height = 1;
	desc.Width = size;

	D3D12_HEAP_PROPERTIES uploadHeapProps = {};
	uploadHeapProps.Type = D3D12_HEAP_TYPE_UPLOAD;

	HR(device->CreateCommittedResource(&uploadHeapProps, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&uploadBuffer)));

	D3D12_HEAP_PROPERTIES heapProps = {};
	heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;

	ID3D12Resource* defaultBuffer = nullptr;
	HR(device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&defaultBuffer)));

	//Upload from CPU to UploadBuffer
	UINT8* uploadData = nullptr;
	uploadBuffer->Map(0, nullptr, (void**)&uploadData);
	memcpy(uploadData, data, size);
	uploadBuffer->Unmap(0, nullptr);

	cmdList->CopyResource(defaultBuffer, uploadBuffer);

	cmdList->ResourceBarrier(1, &ResourceBarrier::Transition(defaultBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));

	return defaultBuffer;
}

//Remember that constant buffers need to be multiples of 256 bytes wide...
ID3D12Resource* D3D12RenderSystem::CreateConstantBuffer(unsigned int size, const void* data)
{
	D3D12_RESOURCE_DESC desc = {};
	desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	desc.DepthOrArraySize = 1;
	desc.Format = DXGI_FORMAT_UNKNOWN;
	desc.MipLevels = 1;
	desc.SampleDesc = { 1, 0 };
	desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	desc.Height = 1;
	desc.Width = 256;

	D3D12_HEAP_PROPERTIES heapProps = {};
	heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;

	ID3D12Resource* constantBuffer;
	HR(device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&constantBuffer)));

	UpdateBuffer(constantBuffer, 256, &data);

	return constantBuffer;
}

void D3D12RenderSystem::CreateVertexBuffer(unsigned int size, const void* data, ActorSystem* actorSystem)
{
	actorSystem->pso.vertexBuffer->size = size;
	actorSystem->pso.vertexBuffer->data = CreateDefaultBuffer(size, data, uploadBuffer);
}

void D3D12RenderSystem::CreateSamplerState(ISampler* sampler)
{
}

void D3D12RenderSystem::CreateTexture(ActorSystem* actorSystem)
{
}

void D3D12RenderSystem::CreateVertexShader()
{
}

void D3D12RenderSystem::CreatePixelShader()
{
}

void D3D12RenderSystem::CreateAllShaders()
{
}

IDXGISwapChain3* D3D12RenderSystem::GetSwapchain()
{
	return swapchain;
}

void D3D12RenderSystem::Present()
{
	HR(swapchain->Present(1, 0));

	WaitForPreviousFrame();
	currentBackBufferIndex = swapchain->GetCurrentBackBufferIndex();
}

void D3D12RenderSystem::Flush()
{
	ExecuteCommandLists();
}

void D3D12RenderSystem::ExecuteCommandLists()
{
	HR(cmdList->Close());

	ID3D12CommandList* cmdLists[] = { cmdList };
	cmdQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);
}

void D3D12RenderSystem::InitD2D()
{
	D2D1_FACTORY_OPTIONS options;
	options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
	HR(D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED, options, &d2dFactory)); 

	HR(d3d11On12Device->QueryInterface(&dxgiDevice));

	assert(dxgiDevice);
	HR(d2dFactory->CreateDevice(dxgiDevice, &d2dDevice));
	assert(d2dDevice);

	HR(d2dDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_ENABLE_MULTITHREADED_OPTIMIZATIONS, &d2dDeviceContext));

	D2D1_RENDER_TARGET_PROPERTIES rtProps = D2D1::RenderTargetProperties(
		D2D1_RENDER_TARGET_TYPE_DEFAULT,
		D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED));

	HR(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(writeFactory), (IUnknown**)(&writeFactory)));

	//DirectWrite Init
	HR(writeFactory->CreateTextFormat(L"Terminal", NULL, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL, 14.f, L"en-us", &textFormat));

	HR(writeFactory->CreateTextFormat(L"Terminal", NULL, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL, 14.f, L"en-us", &textFormat));

	D2D1_COLOR_F color = { 1.f, 0.f, 0.f, 1.f };
	HR(d2dDeviceContext->CreateSolidColorBrush(color, &brushText));
}

void D3D12RenderSystem::UpdateBuffer(ID3D12Resource* buffer, int byteWidth, void* initData)
{
	UINT8* data = nullptr;
	buffer->Map(0, nullptr, (void**)&data);
	memcpy(data, initData, byteWidth);
	buffer->Unmap(0, nullptr);
}

void D3D12RenderSystem::WaitForPreviousFrame()
{
	const UINT64 tempFenceVal = fenceVal;
	cmdQueue->Signal(fence, tempFenceVal);
	fenceVal++;

	if (fence->GetCompletedValue() < tempFenceVal)
	{
		fence->SetEventOnCompletion(tempFenceVal, fenceEvent);
		WaitForSingleObject(fenceEvent, INFINITE);
	}
}
