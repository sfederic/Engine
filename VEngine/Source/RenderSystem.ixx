#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

module;
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <dxgi1_6.h>
export module RenderSystem;

//TODO: want a Qt Widget where you can see that current global Rast/blend states and so on.

using namespace DirectX;

export class RenderSystem
{
public:
	std::vector<IDXGIAdapter1*> adapters;
	std::vector<DXGI_ADAPTER_DESC1> adaptersDesc;

	static const int frameCount = 2;

	D3D11_VIEWPORT viewport;

	ID3D11Device* device;
	ID3D11DeviceContext* context;
	IDXGISwapChain3* swapchain;
	ID3D11RenderTargetView* rtvs[frameCount];
	ID3D11DepthStencilView* dsv;
	ID3D11InputLayout* inputLayout;
	ID3D11VertexShader* vertexShader;
	ID3D11PixelShader* pixelShader;

	ID3D11RasterizerState* rastStateSolid;
	ID3D11RasterizerState* rastStateNoBackCull;
	ID3D11RasterizerState* rastStateWireframe;
	ID3D11RasterizerState* activeRastState;

	ID3D11BlendState* blendStateAlphaToCoverage;

	IDXGIFactory6* dxgiFactory;

	ID3D11Query* disjointQuery;
	ID3D11Query* startTimeQuery;
	ID3D11Query* endTimeQuery;

	ID3D11Buffer* cbMatrices;
	ID3D11Buffer* cbMaterial;

	ID3DBlob* vertexCode;
	ID3DBlob* pixelCode;

	ID3D11Buffer* debugLineBuffer;

	Matrices matrices;
	Material* material;

	D3D_FEATURE_LEVEL featureLevel;

	double renderTime;

	bool bDrawBoundingBoxes = false;
	bool bDrawBoundingSpheres = false;
	bool bQueryGPU = false;

	bool bQueryGPUInner = false;

	void Tick()
	{
		//Set wireframe on/off
		if (gInputSystem.GetKeyUpState(Keys::F1))
		{
			activeRastState = rastStateWireframe;
		}
		if (gInputSystem.GetKeyUpState(Keys::F2))
		{
			activeRastState = rastStateSolid;
		}

		if (gInputSystem.GetKeyUpState(Keys::B))
		{
			bDrawBoundingBoxes = !bDrawBoundingBoxes;
		}
		if (gInputSystem.GetKeyUpState(Keys::V))
		{
			bDrawBoundingSpheres = !bDrawBoundingSpheres;
		}
	}

	void Init(HWND window)
	{
		viewport = { 0.f, 0.f, (float)gCoreSystem.windowWidth, (float)gCoreSystem.windowHeight, 0.f, 1.f };

		CreateDevice();
		CreateSwapchain(window);
		CreateRTVAndDSV();
		CreateInputLayout();
		CreateRasterizerStates();
		CreateBlendStates();
		CreateMainConstantBuffers();

		//Check feature support (just for breakpoint checking for now)
		D3D11_FEATURE_DATA_THREADING threadFeature = {};
		device->CheckFeatureSupport(D3D11_FEATURE_THREADING, &threadFeature, sizeof(threadFeature));

		//Keep an eye on the size of this buffer. There are no checks in the code for it.
		debugLines.push_back(Line());
		debugLineBuffer = CreateDefaultBuffer(sizeof(Line) * 256, D3D11_BIND_VERTEX_BUFFER, debugLines.data());
		debugLines.clear();
	}

	void CreateAllShaders()
	{
		for (int i = 0; i < gShaderFactory.shaders.size(); i++)
		{
			HR(device->CreateVertexShader(
				gShaderFactory.shaders[i].vertexCode->GetBufferPointer(),
				gShaderFactory.shaders[i].vertexCode->GetBufferSize(),
				nullptr,
				&gShaderFactory.shaders[i].vertexShader));

			HR(device->CreatePixelShader(
				gShaderFactory.shaders[i].pixelCode->GetBufferPointer(),
				gShaderFactory.shaders[i].pixelCode->GetBufferSize(),
				nullptr,
				&gShaderFactory.shaders[i].pixelShader));
		}
	}

	void CreateDevice()
	{
		D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0 };

		IDXGIFactory* tmpFactory;
		HR(CreateDXGIFactory(IID_PPV_ARGS(&tmpFactory)));
		HR(tmpFactory->QueryInterface(&dxgiFactory));
		tmpFactory->Release();

		//Reference for EnumAdapterByGpuPerformance
		//REF:https://github.com/walbourn/directx-vs-templates/blob/master/d3d11game_win32_dr/DeviceResources.cpp

		IDXGIAdapter1* adapter = nullptr;
		for (int i = 0; dxgiFactory->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_MINIMUM_POWER, IID_PPV_ARGS(&adapter)) != DXGI_ERROR_NOT_FOUND; i++)
		{
			adapters.push_back(adapter);
			DXGI_ADAPTER_DESC1 desc = {};
			adapter->GetDesc1(&desc);
			adaptersDesc.push_back(desc);
		}

		//BGRA support needed for DirectWrite and Direct2D
		UINT createDeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifdef _DEBUG
		createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

		HR(D3D11CreateDevice(adapter, D3D_DRIVER_TYPE_HARDWARE, 0, createDeviceFlags,
			featureLevels, _countof(featureLevels), D3D11_SDK_VERSION, &device, &featureLevel, &context));


		gShaderFactory.CompileAllShadersFromFile();
		gRenderSystem.CreateAllShaders();
		gShaderFactory.InitHotLoading();

		D3D11_QUERY_DESC qd = {};
		qd.Query = D3D11_QUERY_TIMESTAMP;
		HR(device->CreateQuery(&qd, &startTimeQuery));
		HR(device->CreateQuery(&qd, &endTimeQuery));

		qd.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;
		HR(device->CreateQuery(&qd, &disjointQuery));
	}

	void CreateSwapchain(HWND windowHandle)
	{
		DXGI_SWAP_CHAIN_DESC sd = {};
		sd.BufferDesc = { (UINT)gCoreSystem.windowWidth, (UINT)gCoreSystem.windowHeight, {60, 1}, DXGI_FORMAT_R8G8B8A8_UNORM };
		sd.Windowed = TRUE;
		sd.SampleDesc.Count = 1;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.OutputWindow = windowHandle;
		sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		sd.BufferCount = frameCount;

		IDXGISwapChain* tmpSwapchain;
		HR(dxgiFactory->CreateSwapChain(device, &sd, &tmpSwapchain));
		HR(tmpSwapchain->QueryInterface(&swapchain));
		tmpSwapchain->Release();
	}

	void CreateRTVAndDSV()
	{
		for (int i = 0; i < frameCount; i++)
		{
			ID3D11Texture2D* backBuffer;
			swapchain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
			assert(backBuffer);
			HR(device->CreateRenderTargetView(backBuffer, nullptr, &rtvs[i]));
			backBuffer->Release();
		}

		D3D11_TEXTURE2D_DESC dsDesc = {};
		dsDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		dsDesc.ArraySize = 1;
		dsDesc.MipLevels = 1;
		dsDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		dsDesc.SampleDesc.Count = 1;
		dsDesc.Width = gCoreSystem.windowWidth;
		dsDesc.Height = gCoreSystem.windowHeight;

		ID3D11Texture2D* depthStencilBuffer;
		HR(device->CreateTexture2D(&dsDesc, nullptr, &depthStencilBuffer));
		assert(depthStencilBuffer);
		HR(device->CreateDepthStencilView(depthStencilBuffer, nullptr, &dsv));
		depthStencilBuffer->Release();
	}

	void CreateShaders()
	{
		vertexCode = CreateShaderFromFile(L"Shaders/shaders.hlsl", "VSMain", "vs_5_0");
		pixelCode = CreateShaderFromFile(L"Shaders/shaders.hlsl", "PSMain", "ps_5_0");

		HR(device->CreateVertexShader(vertexCode->GetBufferPointer(), vertexCode->GetBufferSize(), nullptr, &vertexShader));
		HR(device->CreatePixelShader(pixelCode->GetBufferPointer(), pixelCode->GetBufferSize(), nullptr, &pixelShader));
	}

	void CreateInputLayout()
	{
		D3D11_INPUT_ELEMENT_DESC inputDesc[] = {
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, pos), D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(Vertex, uv), D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, normal), D3D11_INPUT_PER_VERTEX_DATA, 0},
		};

		CreateShaders();

		HR(device->CreateInputLayout(inputDesc, _countof(inputDesc), vertexCode->GetBufferPointer(), vertexCode->GetBufferSize(), &inputLayout));
		context->IASetInputLayout(inputLayout);
	}

	void CreateRasterizerStates()
	{
		D3D11_RASTERIZER_DESC rastDesc = {};
		rastDesc.FillMode = D3D11_FILL_SOLID;
		rastDesc.CullMode = D3D11_CULL_BACK;
		rastDesc.DepthClipEnable = TRUE;
		rastDesc.FrontCounterClockwise = FALSE;

		//SOLID
		HR(device->CreateRasterizerState(&rastDesc, &rastStateSolid));
		activeRastState = rastStateSolid;
		context->RSSetState(activeRastState);

		//WIREFRAME
		rastDesc.FillMode = D3D11_FILL_WIREFRAME;
		rastDesc.CullMode = D3D11_CULL_NONE;
		HR(device->CreateRasterizerState(&rastDesc, &rastStateWireframe));

		//SOLID, NO BACK CULL
		rastDesc.CullMode = D3D11_CULL_NONE;
		rastDesc.FillMode = D3D11_FILL_SOLID;
		HR(device->CreateRasterizerState(&rastDesc, &rastStateNoBackCull));
	}

	void CreateBlendStates()
	{
		//Remember that MSAA has to be set for AlphaToCoverage to work.
		D3D11_BLEND_DESC alphaToCoverageDesc = {};
		alphaToCoverageDesc.AlphaToCoverageEnable = true;
		alphaToCoverageDesc.IndependentBlendEnable = false;
		alphaToCoverageDesc.RenderTarget[0].BlendEnable = false;
		alphaToCoverageDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

		HR(device->CreateBlendState(&alphaToCoverageDesc, &blendStateAlphaToCoverage));
	}

	void CreateMainConstantBuffers()
	{
		//Matrix constant buffer
		matrices.model = XMMatrixIdentity();
		matrices.view = XMMatrixIdentity();
		matrices.proj = XMMatrixPerspectiveFovLH(XM_PI / 3, gCoreSystem.GetAspectRatio(), 0.01f, 1000.f);

		editorCamera.proj = matrices.proj;
		matrices.mvp = matrices.model * matrices.view * matrices.proj;

		cbMatrices = CreateDefaultBuffer(sizeof(Matrices), D3D11_BIND_CONSTANT_BUFFER, &matrices);

		//Material constant buffer	
		//material = new Material();
		//material->colour = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
		//cbMaterial = CreateDefaultBuffer(sizeof(Material), D3D11_BIND_CONSTANT_BUFFER, nullptr);
	}

	//One vertex buffer per actor system
	void CreateVertexBuffer(UINT size, const void* data, ActorSystem* system)
	{
		Buffer* buffer = new Buffer();
		buffer->data = CreateDefaultBuffer(size, D3D11_BIND_VERTEX_BUFFER, data);
		system->SetVertexBuffer(buffer);
	}

	void CreateIndexBuffer(UINT size, const void* data, ActorSystem* actor)
	{
		Buffer* buffer = new Buffer();
		buffer->data = CreateDefaultBuffer(size, D3D11_BIND_INDEX_BUFFER, data);
		actor->SetIndexBuffer(buffer);
	}

	IDXGISwapChain3* GetSwapchain()
	{
		return swapchain;
	}

	void Present()
	{
		HR(swapchain->Present(1, 0));
	}

	void Flush()
	{
		//Empty
	}

	//Takes the actor system's texture and throws it into SRV to link with a shader.
	void CreateShaderView(ShaderResourceView* shaderView, Texture* texture)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC desc = {};
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		desc.Texture2D.MipLevels = 1;
		desc.Texture2D.MostDetailedMip = 0;
		desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;

		ID3D11ShaderResourceView* shaderResourceView = nullptr;
		HR(device->CreateShaderResourceView(texture->data, &desc, &shaderResourceView));
	}

	void CreateSamplerState(Sampler* sampler)
	{
		D3D11_SAMPLER_DESC sampDesc = {};
		sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;

		HR(device->CreateSamplerState(&sampDesc, &sampler->data));
	}

	void CreateTexture(ActorSystem* actorSystem)
	{
		if (actorSystem->textureName.empty())
		{
			DebugPrint("%s has no texture filename specified. Skipping texture creation.\n", actorSystem->name);
			return;
		}

		std::wstring textureFilename = L"Textures/";
		textureFilename += stows(actorSystem->textureName);

		ID3D11Resource* texture = nullptr;
		ID3D11ShaderResourceView* srv = nullptr;

		HR(CreateWICTextureFromFile(device, textureFilename.c_str(), &texture, &srv));
		if (texture && srv)
		{
			actorSystem->pso.texture->data = texture;
			actorSystem->pso.srv->data = srv;
		}
	}

	void RenderActorSystem(World* world)
	{
		for (int actorSystemIndex = 0; actorSystemIndex < world->actorSystems.size(); actorSystemIndex++)
		{
			ActorSystem* actorSystem = world->actorSystems[actorSystemIndex];

			//Set rastState
			if (actorSystem->GetRasterizerState())
			{
				//context->RSSetState(actorSystem->rastState);
			}
			else
			{
				context->RSSetState(activeRastState);
			}

			//Set blend
			{
				//TODO: Not really a blendstate problem as a whole, need to figure out how to do pixel clipping
				//for grass and fences and boxes and shit and make it work with a material constant buffer struct
				//for transparent images. All it is is just an if(transparentTexture) {clip(textureColour.a - 0.01f) }
				//const FLOAT blendState[4] = { 0.f };
				//context->OMSetBlendState(blendStateAlphaToCoverage, blendState, 0xFFFFFFFF);
			}


			//Set shaders
			auto shader = gShaderFactory.shaderMap.find(stows(actorSystem->shaderName));

			if (shader == gShaderFactory.shaderMap.end())
			{
				DebugPrint("vertex shader file name %ls not found\n", actorSystem->shaderName);
			}

			assert(shader->second->vertexShader);
			assert(shader->second->pixelShader);
			context->VSSetShader(shader->second->vertexShader, nullptr, 0);
			context->PSSetShader(shader->second->pixelShader, nullptr, 0);

			//Set all state
			ID3D11SamplerState* samplers[] =
			{
				actorSystem->GetSamplerState()->data
			};
			context->PSSetSamplers(0, _countof(samplers), samplers);

			ID3D11ShaderResourceView* shaderResourceViews[] =
			{
				actorSystem->GetShaderView()->data
			};
			context->PSSetShaderResources(0, _countof(shaderResourceViews), shaderResourceViews);
			context->VSSetShaderResources(3, 1, &actorSystem->pso.instancedDataSrv->data);

			ID3D11Buffer* vertexBuffers[] =
			{
				actorSystem->GetVertexBuffer()->data
			};
			context->IASetVertexBuffers(0, _countof(vertexBuffers), vertexBuffers, &strides, &offsets);

			context->IASetIndexBuffer(actorSystem->pso.indexBuffer->data, DXGI_FORMAT_R16_UINT, 0);

			//Constant buffer register values
			const int cbMatrixRegister = 0;
			const int cbMaterialRegister = 1;

			std::vector<InstanceData> instanceData;
			instanceData.reserve(actorSystem->actors.size());

			//Populate instance data
			//TODO: going to have to come back here at one point and make the instance data based on
			//which actors in a system hold reference to the actorsystem's vertexbuffer (model) and batch
			//the instance data per actor accordingly (material and texture wouldn't matter).
			//OR look into how other people have done batching by sorting.
			for (int actorIndex = 0; actorIndex < actorSystem->actors.size(); actorIndex++)
			{
				InstanceData data = {};

				//This is like a hack to get around hiding actors with instanced rendering
				if (actorSystem->actors[actorIndex]->bRender)
				{
					//data.model = actorSystem->actors[actorIndex]->transform.world;
					//data.model = actorSystem->actors[actorIndex]->GetTransformationMatrix();
					data.model = actorSystem->actors[actorIndex]->GetWorldMatrix();
				}

				instanceData.push_back(data);
			}

			//matrix cbuffer 
			matrices.view = GetActiveCamera()->view;
			context->UpdateSubresource(cbMatrices, 0, nullptr, &matrices, 0, 0);
			context->VSSetConstantBuffers(cbMatrixRegister, 1, &cbMatrices);

			//update structred buffer holding instance data
			if (!instanceData.empty())
			{
				context->UpdateSubresource(actorSystem->pso.instancedDataStructuredBuffer->data, 0, nullptr, instanceData.data(), 0, 0);
			}

			context->DrawIndexedInstanced(actorSystem->modelData.indices.size(),
				actorSystem->actors.size(), 0, 0, 0);
		}
	}

	//Bounding primitives here aren't using the same structured buffer DrawInstanced() as actors are, they're
	//just using Draw() because trying to keep up with actor counts and debug primitive structured buffer sizes
	//is a headache. You could just make these structured buffers huge, but they won't be rendered in gameplay anyway.
	void RenderBounds()
	{
		World* world = GetWorld();
		Camera* camera = GetActiveCamera();

		const int cbMatrixRegister = 0;
		const int cbMaterialRegister = 1;

		if (bDrawBoundingBoxes || bDrawBoundingSpheres)
		{
			context->RSSetState(rastStateWireframe);
		}

		if (bDrawBoundingBoxes)
		{
			auto boxIt = gShaderFactory.shaderMap.find(stows(debugBox.shaderName));
			context->VSSetShader(boxIt->second->vertexShader, nullptr, 0);
			context->PSSetShader(boxIt->second->pixelShader, nullptr, 0);

			context->IASetVertexBuffers(0, 1, &debugBox.GetVertexBuffer()->data, &strides, &offsets);
			context->VSSetConstantBuffers(cbMatrixRegister, 1, &cbMatrices);

			for (ActorSystem* actorSystem : world->actorSystems)
			{
				for (Actor* actor : actorSystem->actors)
				{
					matrices.model = actor->GetTransformationMatrix();
					matrices.view = GetActiveCamera()->view;
					matrices.mvp = matrices.model * matrices.view * matrices.proj;
					context->UpdateSubresource(cbMatrices, 0, nullptr, &matrices, 0, 0);

					context->Draw(debugBox.modelData.verts.size(), 0);
				}
			}
		}

		if (bDrawBoundingSpheres)
		{
			auto sphereIt = gShaderFactory.shaderMap.find(stows(debugSphere.shaderName));
			context->VSSetShader(sphereIt->second->vertexShader, nullptr, 0);
			context->PSSetShader(sphereIt->second->pixelShader, nullptr, 0);

			context->IASetVertexBuffers(0, 1, &debugSphere.GetVertexBuffer()->data, &strides, &offsets);
			context->VSSetConstantBuffers(cbMatrixRegister, 1, &cbMatrices);

			for (ActorSystem* actorSystem : world->actorSystems)
			{
				for (Actor* actor : actorSystem->actors)
				{
					XMMATRIX sphereBoundsMatrix = XMMatrixIdentity();

					XMVECTOR actorPos = actor->GetPositionVector();
					actorPos.m128_f32[3] = 1.0f;
					XMVECTOR actorScale = XMLoadFloat3(&actor->GetScale());

					//Grab the actors largest scale value and build the sphere's scale from that, making it uniform
					float highestScaleValue = FindMaxInVector(actorScale);
					XMVECTOR scale = XMVectorReplicate(highestScaleValue);
					scale.m128_f32[3] = 1.0f;

					sphereBoundsMatrix = XMMatrixScalingFromVector(scale);
					sphereBoundsMatrix.r[3] = actorPos;

					matrices.model = sphereBoundsMatrix;
					matrices.view = GetActiveCamera()->view;
					matrices.mvp = matrices.model * matrices.view * matrices.proj;
					context->UpdateSubresource(cbMatrices, 0, nullptr, &matrices, 0, 0);

					context->Draw(debugSphere.modelData.verts.size(), 0);
				}
			}
		}
	}

	void Render(float deltaTime)
	{
		PROFILE_START

			RenderActorSystem(GetWorld());
		RenderBounds();

		PROFILE_END
	}

	void RenderEnd(float deltaTime)
	{
		//TODO: put debug line buffers into rendersystem
		//DRAW DEBUG LINES
		if ((debugLineBuffer != nullptr) && (debugLines.size() > 0))
		{
			//Use the debug box's shader stuff for now
			auto boxIt = gShaderFactory.shaderMap.find(stows(debugBox.shaderName));
			context->VSSetShader(boxIt->second->vertexShader, nullptr, 0);
			context->PSSetShader(boxIt->second->pixelShader, nullptr, 0);

			context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
			context->IASetVertexBuffers(0, 1, &debugLineBuffer, &strides, &offsets);

			context->UpdateSubresource(debugLineBuffer, 0, nullptr, &debugLines[0], 0, 0);

			for (int i = 0; i < debugLines.size(); i++)
			{
				matrices.view = GetActiveCamera()->view;
				matrices.model = XMMatrixIdentity();
				matrices.mvp = matrices.model * matrices.view * matrices.proj;
				context->UpdateSubresource(cbMatrices, 0, nullptr, &matrices, 0, 0);
				context->VSSetConstantBuffers(0, 1, &cbMatrices);

				context->Draw(debugLines.size() * 2, 0);
			}

			ClearDebugRays();
		}


		//Draw debug primitives (seperate from bounds, bounds use actor transforms)
		if (debugSphere.debugSphereTransforms.size() > 0)
		{
			context->RSSetState(rastStateWireframe);

			auto sphereIt = gShaderFactory.shaderMap.find(stows(debugSphere.shaderName));
			context->VSSetShader(sphereIt->second->vertexShader, nullptr, 0);
			context->PSSetShader(sphereIt->second->pixelShader, nullptr, 0);

			context->IASetVertexBuffers(0, 1, (ID3D11Buffer**)debugSphere.GetVertexBuffer(), &strides, &offsets);

			for (Transform& transform : debugSphere.debugSphereTransforms)
			{
				matrices.view = GetActiveCamera()->view;
				matrices.model = transform.GetAffine();
				matrices.mvp = matrices.model * matrices.view * matrices.proj;
				context->UpdateSubresource(cbMatrices, 0, nullptr, &matrices, 0, 0);
				context->VSSetConstantBuffers(0, 1, &cbMatrices);

				context->Draw(debugSphere.modelData.verts.size(), 0);
			}

			//Need to clear out transforms at end
			debugSphere.debugSphereTransforms.clear();
		}

		if (debugBox.debugBoxTransforms.size() > 0)
		{
			context->RSSetState(rastStateWireframe);

			auto boxIt = gShaderFactory.shaderMap.find(stows(debugBox.shaderName));
			context->VSSetShader(boxIt->second->vertexShader, nullptr, 0);
			context->PSSetShader(boxIt->second->pixelShader, nullptr, 0);

			context->IASetVertexBuffers(0, 1, (ID3D11Buffer**)debugBox.GetVertexBuffer(), &strides, &offsets);

			for (Transform& transform : debugBox.debugBoxTransforms)
			{
				matrices.view = GetActiveCamera()->view;
				matrices.model = transform.GetAffine();
				matrices.mvp = matrices.model * matrices.view * matrices.proj;
				context->UpdateSubresource(cbMatrices, 0, nullptr, &matrices, 0, 0);
				context->VSSetConstantBuffers(0, 1, &cbMatrices);

				context->Draw(debugSphere.modelData.verts.size(), 0);
			}

			//Need to clear out transforms at end
			debugBox.debugBoxTransforms.clear();
		}


		//TODO: the GPU query timer stuff is heavy, looks like it drops the FPS by half because of those sleeps()s
		//down there.
		//END QUERY
		if (bQueryGPUInner)
		{
			context->End(endTimeQuery);
			context->End(disjointQuery);

			//POLL QUERY
			while (context->GetData(disjointQuery, nullptr, 0, 0) == S_FALSE)
			{
				Sleep(1);
			}


			D3D11_QUERY_DATA_TIMESTAMP_DISJOINT freq = {};
			HR(context->GetData(disjointQuery, &freq, sizeof(freq), 0));

			//Is the thread polling necessary?
			while (context->GetData(startTimeQuery, nullptr, 0, 0) == S_FALSE)
			{
				Sleep(1);
			}

			while (context->GetData(endTimeQuery, nullptr, 0, 0) == S_FALSE)
			{
				Sleep(1);
			}

			UINT64 endTime = 0, startTime = 0;
			HR(context->GetData(startTimeQuery, &startTime, sizeof(UINT64), 0));
			HR(context->GetData(endTimeQuery, &endTime, sizeof(UINT64), 0));

			UINT64 realTime = endTime - startTime;
			double tick = 1.0 / freq.Frequency;
			double time = tick * (realTime);

			renderTime = time;
		}
	}

	void RenderSetup(float deltaTime)
	{
		if (bQueryGPU)
		{
			context->Begin(disjointQuery);
			context->End(startTimeQuery);
			bQueryGPUInner = true;
		}
		else
		{
			bQueryGPUInner = false;
		}

		context->RSSetViewports(1, &viewport);

		const float clearColour[4] = { 0.2f, 0.2f, 0.2f, 1.f };
		UINT frameIndex = swapchain->GetCurrentBackBufferIndex();

		context->ClearRenderTargetView(rtvs[frameIndex], clearColour);
		context->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);

		context->OMSetRenderTargets(1, &rtvs[frameIndex], dsv);
		context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		context->RSSetState(activeRastState);

		//TODO: this needs to move somewhere else
		if (gInputSystem.GetKeyUpState(Keys::F3))
		{
			gShaderFactory.HotReloadShaders();
			gDebugMenu.notifications.push_back(DebugNotification(L"Shaders reloaded."));
		}
	}

	ID3DBlob* CreateShaderFromFile(const wchar_t* filename, const char* entry, const char* target)
	{
		UINT compileFlags = 0;
#ifdef _DEBUG
		compileFlags = D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_DEBUG;
#endif
		ID3DBlob* code;
		ID3DBlob* error;

		D3DCompileFromFile(filename, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, entry, target, compileFlags, 0, &code, &error);
		if (error)
		{
			const wchar_t* errMsg = (wchar_t*)error->GetBufferPointer();
			//OutputDebugString(errMsg);
			MessageBox(0, (char*)errMsg, entry, 0);
		}

		return code;
	}

	ID3D11Buffer* CreateDefaultBuffer(UINT byteWidth, UINT bindFlags, const void* initData)
	{
		ID3D11Buffer* buffer;

		D3D11_BUFFER_DESC desc = {};
		desc.ByteWidth = byteWidth;
		desc.BindFlags = bindFlags;

		D3D11_SUBRESOURCE_DATA data = {};
		data.pSysMem = initData;

		HR(device->CreateBuffer(&desc, &data, &buffer));

		return buffer;
	}

	ID3D11Buffer* CreateStructuredBuffer(UINT byteWidth, UINT byteStride, const void* initData)
	{
		ID3D11Buffer* buffer;

		D3D11_BUFFER_DESC desc = {};
		desc.ByteWidth = byteWidth;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.StructureByteStride = byteStride;
		desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

		D3D11_SUBRESOURCE_DATA data = {};
		data.pSysMem = initData;

		HR(device->CreateBuffer(&desc, &data, &buffer));

		return buffer;
	}

	ID3D11Buffer* CreateDynamicBuffer(UINT byteWidth, UINT bindFlags, const void* initData)
	{
		ID3D11Buffer* buffer;

		D3D11_BUFFER_DESC desc = {};
		desc.ByteWidth = byteWidth;
		desc.BindFlags = bindFlags;
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		D3D11_SUBRESOURCE_DATA data = {};
		data.pSysMem = initData;

		HR(device->CreateBuffer(&desc, &data, &buffer));

		return buffer;
	}
};

export RenderSystem gRenderSystem;