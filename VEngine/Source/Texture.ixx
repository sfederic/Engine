export module Texture;

//DirectXTK WIC helper libs use ID3D11Resource
struct ID3D11Texture;
struct ID3D11Resource;

export class Texture
{
public:
	ID3D11Resource* data;
};
