#include "AnimationStructures.h"

using namespace DirectX;

void Animation::Interpolate(float t, DirectX::XMFLOAT4X4& m)
{
	if (t <= frames.front().time)
	{
		XMVECTOR pos = XMLoadFloat3(&frames.front().pos);
		XMVECTOR scale = XMLoadFloat3(&frames.front().scale);
		XMVECTOR rot = XMLoadFloat4(&frames.front().rot);
		XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
		XMStoreFloat4x4(&m, XMMatrixAffineTransformation(scale, zero, rot, pos));
		return;
	}
	else if(t >= frames.back().time)
	{
		XMVECTOR pos = XMLoadFloat3(&frames.back().pos);
		XMVECTOR scale = XMLoadFloat3(&frames.back().scale);
		XMVECTOR rot = XMLoadFloat4(&frames.back().rot);
		XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
		XMStoreFloat4x4(&m, XMMatrixAffineTransformation(scale, zero, rot, pos));
		return;
	}

	for (int i = 0; i < (frames.size() - 1); i++)
	{
		if (t >= frames[i].time && t <= frames[i].time)
		{
			//Get the interpolation difference between input time and frame time.
			float lerpPercent = (t - frames[i].time) / (frames[i + 1].time - frames[i].time);

			XMVECTOR pos1 = XMLoadFloat3(&frames[i].pos);
			XMVECTOR pos2 = XMLoadFloat3(&frames[i+1].pos);

			XMVECTOR scale1 = XMLoadFloat3(&frames[i].scale);
			XMVECTOR scale2 = XMLoadFloat3(&frames[i+1].scale);

			XMVECTOR rot1 = XMLoadFloat4(&frames[i].rot);
			XMVECTOR rot2 = XMLoadFloat4(&frames[i+1].rot);

			XMVECTOR lerpedPos = XMVectorLerp(pos1, pos2, lerpPercent);    
			XMVECTOR lerpedScale = XMVectorLerp(scale1, scale2, lerpPercent);
			XMVECTOR lerpedRot = XMQuaternionSlerp(rot1, rot2, lerpPercent);  
			XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
			XMStoreFloat4x4(&m, XMMatrixAffineTransformation(lerpedScale, zero, lerpedRot, lerpedPos));

			return;
		}
	}
}