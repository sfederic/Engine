module;
#include <DirectXMath.h>
import <vector>;
export module RenderTypes;

using namespace DirectX;

export struct Vertex
{
	XMFLOAT3 pos;
	XMFLOAT2 uv;
	XMFLOAT3 normal;
};

export struct Line
{
	Vertex p1;
	Vertex p2;
};

export struct Matrices
{
	XMMATRIX model;
	XMMATRIX view;
	XMMATRIX proj;
	XMMATRIX mvp;
};

export struct InstanceData
{
	XMMATRIX model;
};

export struct ModelData
{
	std::vector<Vertex> verts;
	std::vector<uint16_t> indices;

	unsigned int GetByteWidth()
	{
		return (sizeof(Vertex) * verts.size());
	}

	void DeleteAll()
	{
		verts.clear();
		indices.clear();
	}

	bool CheckDuplicateVertices(Vertex& vert)
	{
		auto pos = XMLoadFloat3(&vert.pos);

		const int size = verts.size();
		for (int i = 0; i < size; i++)
		{
			XMVECTOR p = XMLoadFloat3(&verts[i].pos);
			if (VecEqual(p, pos, 0.0f))
			{
				return true;
			}
		}

		return false;
	}

	//Duplicate checks for indices only return true if the index is present in the array
	//more than once. Eg. For {2, 1, 0}, {3, 1, 2}, 2 and 1 are the duplicates.
	bool CheckDuplicateIndices(uint16_t index)
	{
		int duplicateCounter = 0;

		for (int i = 0; i < indices.size(); i++)
		{
			if (index == indices[i])
			{
				duplicateCounter++;
				if (duplicateCounter >= 2)
				{
					return true;
				}
			}
		}

		return false;
	}
};

export struct BoneWeights
{
	std::vector<float> weights;
	std::vector<uint32_t> boneIndex;
};
