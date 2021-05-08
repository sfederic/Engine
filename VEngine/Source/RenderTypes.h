#pragma once

#include <DirectXMath.h>
#include <vector>

using namespace DirectX;

struct Vertex
{
	XMFLOAT3 pos;
	XMFLOAT2 uv;
	XMFLOAT3 normal;
};

struct InstanceData
{
	XMFLOAT4X4 model;
};

struct Matrices
{
	XMMATRIX model;
	XMMATRIX view;
	XMMATRIX proj;
	XMMATRIX mvp;
};

struct Material
{
	XMFLOAT4 baseColour;
};

struct ModelData
{
	unsigned int GetByteWidth()
	{
		return (sizeof(Vertex) * verts.size());
	}

	void DeleteAll()
	{
		verts.clear();
		indices.clear();
	}

	std::vector<Vertex> verts;
	std::vector<uint16_t> indices;
};

struct BoneWeights
{
	std::vector<float> weights;
	std::vector<uint32_t> boneIndex;
};
