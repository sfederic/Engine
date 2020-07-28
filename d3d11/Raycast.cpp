#include "Raycast.h"
#include "RenderSystem.h"
#include "Camera.h"
#include "UISystem.h"
#include "Actor.h"
#include "DebugMenu.h"
#include "CoreSystem.h"
#include "Debug.h"
#include "World.h"

void DrawRayDebug(XMVECTOR rayOrigin, XMVECTOR rayDir, float distance, class ID3D11Buffer* debugBuffer)
{
	Vertex v1 = {}, v2 = {};
	XMStoreFloat3(&v1.pos, rayOrigin);
	if (distance <= 0.f)
	{
		distance = 10000.f;
	}
	XMVECTOR dist = rayDir * distance;
	XMVECTOR rayEnd = rayOrigin + dist;
	XMStoreFloat3(&v2.pos, rayEnd);

	gRenderSystem.debugLines.push_back(v1);
	gRenderSystem.debugLines.push_back(v2);

	gRenderSystem.context->UpdateSubresource(debugBuffer, 0, nullptr, gRenderSystem.debugLines.data(), 0, 0);
}

bool Raycast(Ray& ray, XMVECTOR origin, XMVECTOR direction, ActorSystem* actorSystem)
{
	ray.origin = origin;
	ray.direction = direction;

	Camera* camera = GetPlayerCamera();

	XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(camera->view), camera->view);
	XMMATRIX invModel = XMMatrixInverse(&XMMatrixDeterminant(XMMatrixIdentity()), XMMatrixIdentity());
	XMMATRIX toLocal = XMMatrixMultiply(invView, invModel);

	ray.origin = XMVector3TransformCoord(ray.origin, toLocal);
	ray.direction = XMVector3TransformNormal(ray.direction, toLocal);
	ray.direction = XMVector3Normalize(ray.direction);

	for (int i = 0; i < actorSystem->actors.size(); i++)
	{
		actorSystem->boundingBox.Center = actorSystem->actors[i].GetPositionFloat3();
		actorSystem->boundingBox.Extents = actorSystem->actors[i].GetScale();

		if (actorSystem->boundingBox.Intersects(ray.origin, ray.direction, ray.distance))
		{
			ray.actorIndex = i;
			DebugPrint("hit %d\n", ray.actorIndex);
			return true;
		}
	}

	return false;
}

bool RaycastAll(Ray& ray, XMVECTOR origin, XMVECTOR direction, World* world)
{
	for (int i = 0; i < world->actorSystems.size(); i++)
	{
		if (Raycast(ray, origin, direction, &world->actorSystems[i]))
		{
			ray.actorSystemIndex = i;
			return true;
		}
	}

	return false;
}

bool RaycastFromScreen(Ray& ray, int sx, int sy, Camera* camera, ActorSystem* actorSystem)
{
	float vx = (2.f * sx / coreSystem.windowWidth - 1.0f) / camera->proj.r[0].m128_f32[0];
	float vy = (-2.f * sy / coreSystem.windowHeight + 1.0f) / camera->proj.r[1].m128_f32[1];

	ray.origin = XMVectorSet(0.f, 0.f, 0.f, 1.f);
	ray.direction = XMVectorSet(vx, vy, 1.f, 0.f);

	return Raycast(ray, ray.origin, ray.direction, actorSystem);

	/*XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(camera->view), camera->view);
	XMMATRIX invModel = XMMatrixInverse(&XMMatrixDeterminant(XMMatrixIdentity()), XMMatrixIdentity());
	XMMATRIX toLocal = XMMatrixMultiply(invView, invModel);

	ray.origin = XMVector3TransformCoord(ray.origin, toLocal);
	ray.direction = XMVector3TransformNormal(ray.direction, toLocal);
	ray.direction = XMVector3Normalize(ray.direction);

	for (int i = 0; i < actorSystem->actors.size(); i++)
	{
		actorSystem->boundingBox.Center = actorSystem->actors[i].GetPositionFloat3();
		actorSystem->boundingBox.Extents = actorSystem->actors[i].GetScale();

		if (actorSystem->boundingBox.Intersects(ray.origin, ray.direction, ray.distance))
		{
			ray.actorIndex = i;
			DebugPrint("hit %d\n", ray.actorIndex);
			return true;
		}
	}

	return false;*/
}

bool RaycastAllFromScreen(Ray& ray, int sx, int sy, Camera* camera, World* world)
{
	for (int i = 0; i < world->actorSystems.size(); i++)
	{
		if (RaycastFromScreen(ray, sx, sy, camera, &world->actorSystems[i]))
		{
			ray.actorSystemIndex = i;
			return true;
		}
	}

	return false;
}
