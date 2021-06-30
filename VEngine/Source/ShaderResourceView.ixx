export module ShaderResourceView;

struct ID3D11ShaderResourceView;

//TODO: need to throw all of these into one big "Rendertypes"tier files to help with modules.

export class ShaderResourceView
{
public:
	ID3D11ShaderResourceView* data;
};
