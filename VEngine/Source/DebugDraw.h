module;
#include <DirectXMath.h>
import Transform;
import RenderTypes;
import DebugDraw;
import DebugSphere;
import DebugBox;
import Raycast;
export module DebugDraw;

using namespace DirectX;

export namespace DebugDraw
{
	//Scale will always be uniform for debug spheres, doesn't need rotation
	void DebugDraw::DrawSphere(XMFLOAT3 pos, float scale)
	{
		Transform transform;
		transform.position = pos;
		transform.scale = XMFLOAT3(scale, scale, scale);

		debugSphere.debugSphereTransforms.push_back(transform);
	}

	//I guess you can't call it DrawCube. DrawRectangle?
	void DebugDraw::DrawBox(Transform transform)
	{
		debugBox.debugBoxTransforms.push_back(transform);
	}

	//TODO: debug ray drawing sucks. You gotta fix this up.
	void DebugDraw::DrawRay(XMVECTOR rayOrigin, XMVECTOR rayDir, float distance)
	{
		DrawRayDebug(rayOrigin, rayDir, distance);
	}
};
