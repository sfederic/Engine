#pragma once


class ActorSystem
{
public:
	ActorSystem();
	void Serialise(std::ostream& os);
	void SerialiseAsTemplate(std::ostream& os);
	void Deserialise(std::istream& is);
	void DeserialiseAsTemplate(std::istream& is);
	virtual void Tick(float deltaTime);
	virtual void Start();
	virtual void SpawnActors(int numToSpawn);
	virtual Actor* SpawnActor(Transform transform);

	

	void RemoveActor(Actor* actor);
	void RemoveActor(int index);
	Actor* GetActor(unsigned int index);

	Buffer* GetVertexBuffer();
	Buffer* GetInstanceBuffer();
	Sampler* GetSamplerState();
	RasterizerState* GetRasterizerState();
	ShaderResourceView* GetShaderView();
	Texture* GetTexture();

	void SetVertexBuffer(Buffer* vertexBuffer);
	void SetIndexBuffer(Buffer* indexBuffer);
	void SetInstanceBuffer(Buffer* instanceBuffer);
	void SetSamplerState(Sampler* sampler);
	void SetRasterizerState(RasterizerState* rasterizerState);
	void SetShaderView(ShaderResourceView* shaderView);
	void SetTexture(Texture* texture);

	void CreateStructuredBuffer();

	void RecreateTexture();
	void RecreateShader();
	void RecreateModel();

	void Cleanup();


};
