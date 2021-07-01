#include "Raycast.h"
#include "RenderSystem.h"
#include "Camera.h"
#include "UISystem.h"
#include "Actor.h"
#include "DebugMenu.h"
#include "CoreSystem.h"
#include "Debug.h"
#include "World.h"
#include "WorldEditor.h"
#include "PropertiesWidget.h"
#include "PropertiesDock.h"
#include "../EditorMainWindow.h"
#include "MathHelpers.h"

std::vector<Line> debugLines;

void DrawRayDebug(XMVECTOR rayOrigin, XMVECTOR rayDir, float distance)
{
	Line debugLine = {};
	XMStoreFloat3(&debugLine.p1.pos, rayOrigin);
	XMVECTOR pos = XMLoadFloat3(&debugLine.p1.pos);
	pos += GetActiveCamera()->right;
	XMStoreFloat3(&debugLine.p1.pos, pos);

	if (distance <= 0.f)
	{
		distance = 0.f;
	}

	XMVECTOR dist = rayDir * distance;
	XMVECTOR rayEnd = rayOrigin + dist;
	XMStoreFloat3(&debugLine.p2.pos, rayEnd);

	debugLines.push_back(debugLine);
}

void ClearDebugRays()
{
	debugLines.clear();
}

bool Raycast(Ray& ray, XMVECTOR origin, XMVECTOR direction, ActorSystem* actorSystem, bool fromScreen)
{
	ray.origin = origin;
	ray.direction = direction;

	//Calculate raycast from camera coords into world
	if (fromScreen)
	{
		Camera* camera = GetActiveCamera();

		XMVECTOR viewDet = XMMatrixDeterminant(camera->view);
		XMMATRIX invView = XMMatrixInverse(&viewDet, camera->view);

		XMVECTOR invModelDet = XMMatrixDeterminant(XMMatrixIdentity());
		XMMATRIX invModel = XMMatrixInverse(&invModelDet, XMMatrixIdentity());
		XMMATRIX toLocal = XMMatrixMultiply(invView, invModel);

		ray.origin = XMVector3TransformCoord(ray.origin, toLocal);
		ray.direction = XMVector3TransformNormal(ray.direction, toLocal);
		ray.direction = XMVector3Normalize(ray.direction);
	}

	std::vector<float> distances;

	bool bRayHit = false;
	ray.hitActors.clear();

	for (Actor* actor : actorSystem->actors)
	{
		if (!actor->bRender)
		{
			return false;
		}

		BoundingOrientedBox boundingBox = actorSystem->boundingBox;
		UpdateBoundingBox(boundingBox, actor);

		if (boundingBox.Intersects(ray.origin, ray.direction, ray.distance))
		{
			distances.push_back(ray.distance);
			ray.hitActors.push_back(actor);
			bRayHit = true;
		}
	}

	if (bRayHit)
	{
		float nearestDistance = D3D11_FLOAT32_MAX;
		for (int i = 0; i < distances.size(); i++)
		{
			if (distances[i] < nearestDistance)
			{
				nearestDistance = distances[i];
				ray.hitActor = ray.hitActors[i];
			}
		}

		ray.distance = nearestDistance;

		return true;
	}

	return false;
}

bool RaycastTriangleIntersect(Ray& ray)
{
	ActorSystem* actorSystem = GetWorld()->GetActorSystem(ray.actorSystemIndex);
	assert(actorSystem);

	std::vector<Ray> rays;
	
	for (Actor* actor : ray.hitActors)
	{
		XMMATRIX model = actor->GetTransformationMatrix();

		for (int i = 0; i < actorSystem->modelData.verts.size() / 3; i++)
		{
			uint32_t index0 = actorSystem->modelData.indices[i * 3];
			uint32_t index1 = actorSystem->modelData.indices[i * 3 + 1];
			uint32_t index2 = actorSystem->modelData.indices[i * 3 + 2];

			XMVECTOR v0 = XMLoadFloat3(&actorSystem->modelData.verts[index0].pos);
			v0 = XMVector3TransformCoord(v0, model);

			XMVECTOR v1 = XMLoadFloat3(&actorSystem->modelData.verts[index1].pos);
			v1 = XMVector3TransformCoord(v1, model);

			XMVECTOR v2 = XMLoadFloat3(&actorSystem->modelData.verts[index2].pos);
			v2 = XMVector3TransformCoord(v2, model);

			Ray tempRay = {};
			tempRay = ray;
			tempRay.hitActors.clear();

			if (DirectX::TriangleTests::Intersects(ray.origin, ray.direction, v0, v1, v2, tempRay.distance))
			{
				tempRay.modelDataIndex = i;

				XMVECTOR normal = XMVectorZero();
				normal += XMLoadFloat3(&actorSystem->modelData.verts[index0].normal);
				normal += XMLoadFloat3(&actorSystem->modelData.verts[index1].normal);
				normal += XMLoadFloat3(&actorSystem->modelData.verts[index2].normal);

				normal = XMVector3Normalize(normal);

				XMStoreFloat3(&tempRay.normal, normal);

				tempRay.hitActor = actor;

				rays.push_back(tempRay);
			}
		}
	}

	float lowestDistance = D3D11_FLOAT32_MAX;
	int rayIndex = -1;
	for(int i = 0; i < rays.size(); i++)
	{
		if (rays[i].distance < lowestDistance)
		{
			lowestDistance = rays[i].distance;
			rayIndex = i;
		}
	}

	if (rayIndex > -1)
	{
		ray = rays[rayIndex];
		DebugPrint("%s Triangle index %d hit.\n", ray.hitActor->name, rayIndex);

		return true;
	}

	return false;
}

bool RaycastAll(Ray& ray, XMVECTOR origin, XMVECTOR direction, World* world)
{
	for (int i = 0; i < world->actorSystems.size(); i++)
	{
		if (Raycast(ray, origin, direction, world->actorSystems[i]))
		{
			ray.actorSystemIndex = i;
			return true;
		}
	}

	return false;
}

bool RaycastFromScreen(Ray& ray, int sx, int sy, Camera* camera, ActorSystem* actorSystem)
{
	float vx = (2.f * sx / gCoreSystem.windowWidth - 1.0f) / camera->proj.r[0].m128_f32[0];
	float vy = (-2.f * sy / gCoreSystem.windowHeight + 1.0f) / camera->proj.r[1].m128_f32[1];

	ray.origin = XMVectorSet(0.f, 0.f, 0.f, 1.f);
	ray.direction = XMVectorSet(vx, vy, 1.f, 0.f);

	return Raycast(ray, ray.origin, ray.direction, actorSystem, true);
}

bool RaycastAllFromScreen(Ray& ray)
{
	World* world = GetWorld();

	for (int i = 0; i < world->actorSystems.size(); i++)
	{
		if (RaycastFromScreen(ray, gUISystem.mousePos.x, gUISystem.mousePos.y, 
			GetActiveCamera(), world->actorSystems[i]))
		{
			ray.actorSystemIndex = i;

			return true;
		}
	}

	return false;
}
