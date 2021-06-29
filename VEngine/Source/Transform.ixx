module;
#include <DirectXMath.h>
export module Transform;

using namespace DirectX;

export class Transform
{
public:
	Transform::Transform()
	{
		scale = XMFLOAT3(1.f, 1.f, 1.f);
		euler = XMFLOAT3(0.f, 0.f, 0.f);
		quatRotation = XMFLOAT4(0.f, 0.f, 0.f, 1.f);
		position = XMFLOAT3(0.f, 0.f, 0.f);
	}

	XMMATRIX GetAffine()
	{
		return XMMatrixAffineTransformation(
			XMLoadFloat3(&scale),
			XMVectorSet(0.f, 0.f, 0.f, 1.f),
			XMLoadFloat4(&quatRotation),
			XMLoadFloat3(&position)
		);
	}

	//Helps with rotating child actors around their parent using Quaternions
	XMMATRIX GetAffineRotationOrigin(XMVECTOR rotOrigin)
	{
		return XMMatrixAffineTransformation(
			XMLoadFloat3(&scale),
			rotOrigin,
			XMLoadFloat4(&quatRotation),
			XMLoadFloat3(&position)
		);
	}

	void Decompose(XMMATRIX m)
	{
		XMVECTOR outScale, outQuat, outTrans;
		XMMatrixDecompose(&outScale, &outQuat, &outTrans, m);

		XMStoreFloat3(&scale, outScale);
		XMStoreFloat4(&quatRotation, outQuat);
		XMStoreFloat3(&position, outTrans);
	}

	XMMATRIX world = XMMatrixIdentity();
	XMMATRIX local = XMMatrixIdentity();

	XMFLOAT3 position;
	XMFLOAT3 scale;
	XMFLOAT3 euler;
	XMFLOAT4 quatRotation;
};
