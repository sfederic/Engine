export module PipelineView;

class Buffer;
class Sampler;
class RasterizerState;
class Texture;
class ShaderResourceView;
class BlendState;

//PipelineView is essentially trying to mimic D3D12's graphics pipeline object that attaches
//to each actor system. This sort of structure should make it easier to port graphics shit later on.
export struct PipelineView
{
	PipelineView::PipelineView()
	{
		vertexBuffer = new Buffer();
		indexBuffer = new Buffer();
		samplerState = new Sampler();
		rastState = new RasterizerState();
		texture = new Texture();
		srv = new ShaderResourceView();
		instancedDataStructuredBuffer = new Buffer();
		instancedDataSrv = new ShaderResourceView();
	}

	Buffer* vertexBuffer;
	Buffer* indexBuffer;
	Buffer* instanceBuffer;
	Sampler* samplerState;
	RasterizerState* rastState;
	Texture* texture;
	ShaderResourceView* srv;
	BlendState* blendState;
	Buffer* instancedDataStructuredBuffer;
	ShaderResourceView* instancedDataSrv;
};
