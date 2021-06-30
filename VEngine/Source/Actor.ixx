module;
#include <DirectXMath.h>
#include <DirectXCollision.h>
import Transform;
import Material;
import PipelineView;
import MathHelpers;
import Properties;
import AnimationStructures;
import <string>;
import <vector>;
export module Actor;

using namespace DirectX;

export class Actor
{
public:
	Transform transform;
	Material* material;
	std::string name;
	ActorSystem* linkedActorSystem;
	std::vector<Actor*> children;
	Actor* parent = nullptr;
	double currentAnimationTime = 0.0;
	bool bRender = true;
	bool isRoot = true;

	Actor()
	{

	}

	void Tick(float deltaTime)
	{
	}

	void Start()
	{
	}

	Properties GetSaveProps()
	{
		Properties props;
		props.Add("Name", &name);
		props.Add("Position", &transform.position);
		props.Add("Scale", &transform.scale);
		props.Add("RotQuat", &transform.quatRotation);
		return props;
	}

	Properties GetEditorProps()
	{
		Properties props;
		props.Add("Render", &bRender);
		props.Add("Position", &transform.position);
		props.Add("Scale", &transform.scale);
		return props;
	}

	XMVECTOR GetPositionVector()
	{
		return XMLoadFloat3(&transform.position);
	}

	XMFLOAT3 GetPositionFloat3()
	{
		return transform.position;
	}

	void SetPosition(XMVECTOR v)
	{
		v.m128_f32[3] = 1.0f; //Set the W component just in case

		XMVECTOR offset = XMVectorSet(0.f, 0.f, 0.f, 1.f);

		if (parent)
		{
			offset = XMLoadFloat3(&parent->transform.position);
		}

		XMVECTOR newVec = v - offset;
		//transform.local.r[3] = newVec;
		//transform.local.r[3].m128_f32[3] = 1.0f;

		XMStoreFloat3(&transform.position, newVec);
	}

	void SetPosition(float x, float y, float z)
	{
		XMVECTOR pos = XMVectorSet(x, y, z, 1.0f);
		SetPosition(pos);
	}

	void SetPosition(XMFLOAT3 pos)
	{
		XMVECTOR vecpos = XMVectorSet(pos.x, pos.y, pos.z, 1.0f);
		SetPosition(vecpos);
	}

	void SetRotation(XMVECTOR quaternion)
	{
		XMStoreFloat4(&transform.quatRotation, quaternion);
	}

	void SetRotation(XMVECTOR axis, float angle)
	{
		if (XMVector3Equal(XMVectorZero(), axis))
		{
			transform.quatRotation = XMFLOAT4(0.f, 0.f, 0.f, 1.f);
			return;
		}

		float andleRadians = XMConvertToRadians(angle);
		XMStoreFloat4(&transform.quatRotation, XMQuaternionRotationAxis(axis, andleRadians));
	}

	void SetRotation(float pitch, float yaw, float roll)
	{
		transform.euler = XMFLOAT3(pitch, yaw, roll);

		float pitchRadians = XMConvertToRadians(pitch);
		float yawRadians = XMConvertToRadians(yaw);
		float rollRadians = XMConvertToRadians(roll);
		XMStoreFloat4(&transform.quatRotation, XMQuaternionRotationRollPitchYaw(pitchRadians, yawRadians, rollRadians));
	}

	void SetRotation(XMFLOAT3 euler)
	{
		transform.euler = euler;

		float pitchRadians = XMConvertToRadians(euler.x);
		float yawRadians = XMConvertToRadians(euler.y);
		float rollRadians = XMConvertToRadians(euler.z);
		XMStoreFloat4(&transform.quatRotation, XMQuaternionRotationRollPitchYaw(pitchRadians, yawRadians, rollRadians));
	}

	XMFLOAT4 GetRotationQuat()
	{
		return transform.quatRotation;
	}

	XMMATRIX GetTransformationMatrix()
	{
		XMVECTOR rotationOffset = XMVectorSet(0.f, 0.f, 0.f, 1.f);

		if (parent)
		{
			rotationOffset = parent->transform.world.r[3];
		}

		return XMMatrixAffineTransformation(
			XMLoadFloat3(&transform.scale),
			rotationOffset,
			XMLoadFloat4(&transform.quatRotation),
			XMLoadFloat3(&transform.position)
		);
	}

	XMMATRIX GetWorldMatrix()
	{
		XMMATRIX parentWorld = XMMatrixIdentity();

		if (parent)
		{
			parentWorld = parent->GetWorldMatrix();
		}

		UpdateTransform(parentWorld);

		return transform.world;
	}

	void UpdateTransform(XMMATRIX parentWorld)
	{
		XMMATRIX world = GetTransformationMatrix() * parentWorld;

		for (Actor* child : children)
		{
			child->UpdateTransform(world);
		}

		transform.world = world;
	}

	XMFLOAT3 GetPitchYawRoll()
	{
		return PitchYawRollFromMatrix(GetTransformationMatrix());
	}

	XMFLOAT3 GetScale()
	{
		return transform.scale;
	}

	void SetScale(float x, float y, float z)
	{
		transform.scale = XMFLOAT3(x, y, z);
	}

	void SetScale(XMVECTOR scale)
	{
		XMStoreFloat3(&transform.scale, scale);
	}

	void SetScale(XMFLOAT3 scale)
	{
		transform.scale = scale;
	}

	XMVECTOR GetForwardVector()
	{
		XMMATRIX m = XMMatrixAffineTransformation(
			XMLoadFloat3(&transform.scale),
			XMVectorSet(0.f, 0.f, 0.f, 1.f),
			XMLoadFloat4(&transform.quatRotation),
			XMLoadFloat3(&transform.position)
		);

		return m.r[2];
	}

	XMVECTOR GetRightVector()
	{
		XMMATRIX m = XMMatrixAffineTransformation(
			XMLoadFloat3(&transform.scale),
			XMVectorSet(0.f, 0.f, 0.f, 1.f),
			XMLoadFloat4(&transform.quatRotation),
			XMLoadFloat3(&transform.position)
		);

		return m.r[0];
	}

	XMVECTOR GetUpVector()
	{
		XMMATRIX m = XMMatrixAffineTransformation(
			XMLoadFloat3(&transform.scale),
			XMVectorSet(0.f, 0.f, 0.f, 1.f),
			XMLoadFloat4(&transform.quatRotation),
			XMLoadFloat3(&transform.position)
		);

		return m.r[1];
	}

	void Move(float d, XMVECTOR direction)
	{
		XMVECTOR s = XMVectorReplicate(d);
		XMVECTOR loc = GetPositionVector();
		loc = XMVectorMultiplyAdd(s, direction, loc);
		SetPosition(loc);
	}

	void Destroy()
	{
		assert(linkedActorSystem);
		linkedActorSystem->RemoveActor(this);
	}

	void AddChild(Actor* child)
	{
		assert(child);
		child->parent = this;
		child->isRoot = false;
		children.push_back(child);
	}
};

export class ActorSystem
{
public:
	ModelData modelData;
	//Animation animData;
	Material* material;
	PipelineView pso;

	//Everything's a BoundingOrientedBox, but maybe just a BoundingBox is faster in some places.
	//REF:https://www.gamasutra.com/view/feature/131833/when_two_hearts_collide_.php
	BoundingOrientedBox boundingBox;
	BoundingSphere boundingSphere;

	std::vector<Actor*> actors;

	std::string shaderName;
	std::string textureName;
	std::string name;
	std::string modelName;

	uint32_t sizeofActor = 0;
	size_t numVertices;

	bool bAnimated = false;
	bool bRender = true;
	bool bHasBeenInitialised = false;

	template <class ActorType>
	void SetActorSize()
	{
		sizeofActor = sizeof(ActorType);
	}

	template <class ActorType>
	Actor* AddActor(Transform transform)
	{
		ActorType* actor = new ActorType();
		actor->transform = transform;
		actor->material = this->material;
		actor->linkedActorSystem = this;

		actors.push_back(actor);

		actor->name = this->name + std::to_string(actors.size() - 1);

		if (bHasBeenInitialised)
		{
			//Structured buffer needs to be rebuilt
			//TODO: Maybe look into doing some std::vector-tier allocation on Buffer where reallocation
			//happens when it passes a threshold
			CreateStructuredBuffer();
		}

		return actor;
	}

	template <class ActorType>
	void Init(int numActorsToSpawn)
	{
		std::string filename = "Models/";
		filename += modelName;

		if (FBXImporter::Import(filename.c_str(), modelData, this))
		{
			//Spawn actors, setup components
			actors.reserve(numActorsToSpawn);
			for (int i = 0; i < numActorsToSpawn; i++)
			{
				AddActor<ActorType>(Transform());
			}

			//Setup buffers
			UINT byteWidth = modelData.GetByteWidth();
			numVertices = (byteWidth * actors.size()) / sizeof(Vertex);

			gRenderSystem.CreateVertexBuffer(byteWidth, modelData.verts.data(), this);
			gRenderSystem.CreateIndexBuffer(sizeof(uint16_t) * modelData.indices.size(),
				modelData.indices.data(), this);

			CreateStructuredBuffer();

			//Sampler, texture setup
			gRenderSystem.CreateSamplerState(GetSamplerState());
			gRenderSystem.CreateTexture(this);

			size_t stride = sizeof(Vertex);

			//Bounding setup
			BoundingOrientedBox::CreateFromPoints(boundingBox, modelData.verts.size(), &modelData.verts[0].pos, stride);
			BoundingSphere::CreateFromPoints(boundingSphere, modelData.verts.size(), &modelData.verts[0].pos, stride);

			bHasBeenInitialised = true;
		}
		else
		{
			DebugPrint("Actors failed to load");
			bHasBeenInitialised = false;
		}
	}

	void Serialise(std::ostream& os)
	{
		os << name << "\n"; //Use actorsystem name to create again from ActorSystemFactory on Deserialise
		os << actors.size() << "\n"; //Write out num of actors to load the same amount on Deserialise

		//Writing these three out is more about instancing prefabs off of ActorSystem so that you're not 
		//defining these again by hand in new cpp files.
		os << modelName.c_str() << "\n";
		os << textureName.c_str() << "\n";
		os << shaderName.c_str() << "\n";

		for(Actor* actor : actors)
		{
			//Serialiser::Serialise(actor->GetSaveProps(), os);
		}
	}

	void SerialiseAsTemplate(std::ostream& os)
	{
		os << name << "\n";
		os << modelName.c_str() << "\n";
		os << textureName.c_str() << "\n";
		os << shaderName.c_str() << "\n";
	}

	void Deserialise(std::istream& is)
	{
		char name[512];
		is.getline(name, 512);
		is.getline(name, 512);
		modelName.assign(name);

		is.getline(name, 512);
		textureName.assign(name);

		is.getline(name, 512);
		shaderName.assign(name);

		for(Actor* actor : actors)
		{
			Serialiser::Deserialise(actor->GetSaveProps(), is);
		}
	}

	void DeserialiseAsTemplate(std::istream& is)
	{
		is >> name;
		is >> modelName;
		is >> textureName;
		is >> shaderName;
	}

	void Tick(float deltaTime)
	{

	}

	void Start()
	{
	}

	void SpawnActors(int numToSpawn)
	{
		Init<Actor>(numToSpawn);
	}

	Actor* SpawnActor(Transform transform)
	{
		return AddActor<Actor>(transform);
	}

	Buffer* GetVertexBuffer()
	{
		if (pso.vertexBuffer)
		{
			return pso.vertexBuffer;
		}

		return nullptr;
	}

	Buffer* GetInstanceBuffer()
	{
		if (pso.instanceBuffer)
		{
			return pso.instanceBuffer;
		}

		return nullptr;
	}

	void RemoveActor(Actor* actor)
	{
		//TODO: this looks bad (but honestly, I don't think the performance is too bad).
		//Either way look into shared_ptr and how it can be set into the actors vector
		for(int i = 0; i < actors.size(); i++)
		{
			if (&actor[i] == actor)
			{
				actors.erase(actors.begin() + i);
			}
		}
	}

	void RemoveActor(int index)
	{
		assert(index < actors.size());
		actors.erase(actors.begin() + index);
	}

	Actor* GetActor(unsigned int index)
	{
		assert(index < actors.size());
		return actors[index];
	}

	Sampler* GetSamplerState()
	{
		return pso.samplerState;
	}

	RasterizerState* GetRasterizerState()
	{
		if (pso.rastState)
		{
			return pso.rastState;
		}

		return nullptr;
	}

	ShaderResourceView* GetShaderView()
	{
		if (pso.srv)
		{
			return pso.srv;
		}

		return nullptr;
	}

	Texture* GetTexture()
	{
		if (pso.texture)
		{
			return pso.texture;
		}

		return nullptr;
	}

	void SetTexture(Texture* texture)
	{
		pso.texture = texture;
	}

	void CreateStructuredBuffer()
	{
		std::vector<XMMATRIX> actorModelMatrices;
		actorModelMatrices.reserve(actors.size());
		for (int i = 0; i < actors.size(); i++)
		{
			if (i < actors.size())
			{
				actorModelMatrices.push_back(actors[i]->GetTransformationMatrix());
			}
			else
			{
				actorModelMatrices.push_back(XMMatrixIdentity());
			}
		}

		if (pso.instancedDataStructuredBuffer)
		{
			//TODO: gotta fix this up. Maybe wrap this in a ComPtr.
			//instancedDataStructuredBuffer->Release();
		}


		D3D11_SHADER_RESOURCE_VIEW_DESC sbDesc = {};
		sbDesc.Format = DXGI_FORMAT_UNKNOWN;
		sbDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;

		if (actors.size() > 0)
		{
			sbDesc.BufferEx.NumElements = actors.size();
			pso.instancedDataStructuredBuffer->data = gRenderSystem.CreateStructuredBuffer(sizeof(InstanceData) * actors.size(), sizeof(InstanceData), actorModelMatrices.data());
		}
		else
		{
			sbDesc.BufferEx.NumElements = 1;

			actorModelMatrices.push_back(XMMATRIX());
			pso.instancedDataStructuredBuffer->data = gRenderSystem.CreateStructuredBuffer(sizeof(InstanceData), sizeof(InstanceData), actorModelMatrices.data());
		}

		HR(gRenderSystem.device->CreateShaderResourceView(pso.instancedDataStructuredBuffer->data, &sbDesc, &pso.instancedDataSrv->data));
	}

	//For when texture file is changed in-editor.
	void RecreateTexture()
	{
		pso.texture->data->Release();
		pso.srv->data->Release();
		gRenderSystem.CreateTexture(this);
	}

	void RecreateModel()
	{
		pso.vertexBuffer->data->Release();

		modelData.DeleteAll();

		std::string filename = "Models/";
		filename += modelName;
		if (FBXImporter::Import(filename.c_str(), modelData, this))
		{
			gRenderSystem.CreateVertexBuffer(modelData.GetByteWidth(), modelData.verts.data(), this);

			size_t stride = sizeof(Vertex);
			BoundingOrientedBox::CreateFromPoints(boundingBox, modelData.verts.size(), &modelData.verts[0].pos, stride);
			BoundingSphere::CreateFromPoints(boundingSphere, modelData.verts.size(), &modelData.verts[0].pos, stride);
		}
	}

	void Cleanup()
	{
		actors.clear();
	}

	void SetVertexBuffer(Buffer* vertexBuffer)
	{
		pso.vertexBuffer = vertexBuffer;
	}

	void SetIndexBuffer(Buffer* indexBuffer)
	{
		pso.indexBuffer = indexBuffer;
	}

	void SetInstanceBuffer(Buffer* instanceBuffer)
	{
		pso.instanceBuffer = instanceBuffer;
	}

	void SetSamplerState(Sampler* sampler)
	{
		pso.samplerState = sampler;
	}

	void SetRasterizerState(RasterizerState* rasterizerState)
	{
		pso.rastState = rasterizerState;
	}

	void SetShaderView(ShaderResourceView* shaderView)
	{
		pso.srv = shaderView;
	}
};
