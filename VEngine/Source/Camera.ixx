module;
#include <omp.h>
#include <DirectXMath.h>
import CoreSystem;
import Actor;
import UISystem;
import Input;
import Console;
import WorldEditor;
import World;
export module Camera;

using namespace DirectX;

class Camera
{
public:
	Camera(XMVECTOR initialLocation)
	{
		forward = XMVectorSet(0.f, 0.f, 1.f, 0.f);
		right = XMVectorSet(1.f, 0.f, 0.f, 0.f);
		up = XMVectorSet(0.f, 1.f, 0.f, 0.f);

		worldUp = XMVectorSet(0.f, 1.f, 0.f, 0.f);
		focusPoint = XMVectorSet(0.f, 0.f, 0.f, 1.f);

		location = initialLocation;

		UpdateViewMatrix();
	}

	void Tick(float deltaTime)
	{
		//Old following code
		if (actorAttachedTo)
		{
			location = actorAttachedTo->GetPositionVector();
			location += attachedOffset;
		}

		if (bEditorCamera)
		{
			MouseMove(gUISystem.mousePos.x, gUISystem.mousePos.y);
			UpdateViewMatrix();

			//WASD MOVEMENT
			if (!gConsole.bConsoleActive)
			{
				if (gInputSystem.GetAsyncKey(Keys::RightMouse))
				{
					const float moveSpeed = 7.5f * deltaTime;

					if (GetAsyncKeyState('W'))
					{
						Move(moveSpeed, forward);
					}
					if (GetAsyncKeyState('S'))
					{
						Move(-moveSpeed, forward);
					}
					if (GetAsyncKeyState('D'))
					{
						Move(moveSpeed, right);
					}
					if (GetAsyncKeyState('A'))
					{
						Move(-moveSpeed, right);
					}
					if (GetAsyncKeyState('Q'))
					{
						Move(-moveSpeed, up);
					}
					if (GetAsyncKeyState('E'))
					{
						Move(moveSpeed, up);
					}
				}

				//Zoom onto selected actor
				if (gInputSystem.GetKeyUpState(Keys::F))
				{
					World* world = GetWorld();
					ZoomTo(gWorldEditor.pickedActor);
				}

				//MOUSE WHEEL ZOOM
				const float zoomSpeed = 65.f * deltaTime;

				if (gInputSystem.GetMouseWheelUp())
				{
					Move(zoomSpeed, editorCamera.forward);
				}
				else if (gInputSystem.GetMouseWheelDown())
				{
					Move(-zoomSpeed, editorCamera.forward);
				}
			}
		}
	}

	void UpdateViewMatrix()
	{
		forward = XMVector3Normalize(forward);
		up = XMVector3Normalize(XMVector3Cross(forward, right));
		right = XMVector3Cross(up, forward);

		float x = XMVectorGetX(XMVector3Dot(location, right));
		float y = XMVectorGetX(XMVector3Dot(location, up));
		float z = XMVectorGetX(XMVector3Dot(location, forward));

		view.r[0].m128_f32[0] = right.m128_f32[0];
		view.r[1].m128_f32[0] = right.m128_f32[1];
		view.r[2].m128_f32[0] = right.m128_f32[2];
		view.r[3].m128_f32[0] = -x;

		view.r[0].m128_f32[1] = up.m128_f32[0];
		view.r[1].m128_f32[1] = up.m128_f32[1];
		view.r[2].m128_f32[1] = up.m128_f32[2];
		view.r[3].m128_f32[1] = -y;

		view.r[0].m128_f32[2] = forward.m128_f32[0];
		view.r[1].m128_f32[2] = forward.m128_f32[1];
		view.r[2].m128_f32[2] = forward.m128_f32[2];
		view.r[3].m128_f32[2] = -z;

		view.r[0].m128_f32[3] = 0.0f;
		view.r[1].m128_f32[3] = 0.0f;
		view.r[2].m128_f32[3] = 0.0f;
		view.r[3].m128_f32[3] = 1.0f;
	}

	void Pitch(float angle)
	{
		XMMATRIX r = XMMatrixRotationAxis(right, angle);
		up = XMVector3TransformNormal(up, r);
		forward = XMVector3TransformNormal(forward, r);
	}

	void RotateY(float angle)
	{
		XMMATRIX r = XMMatrixRotationY(angle);
		up = XMVector3TransformNormal(up, r);
		right = XMVector3TransformNormal(right, r);
		forward = XMVector3TransformNormal(forward, r);
	}

	void MouseMove(int x, int y)
	{
		static POINT lastMousePos;

		if (GetAsyncKeyState(VK_RBUTTON) < 0)
		{
			float dx = XMConvertToRadians(0.25f * (float)(x - lastMousePos.x));
			float dy = XMConvertToRadians(0.25f * (float)(y - lastMousePos.y));

			Pitch(dy);
			RotateY(dx);
		}

		lastMousePos.x = x;
		lastMousePos.y = y;
	}

	void Move(float d, XMVECTOR axis)
	{
		XMVECTOR s = XMVectorReplicate(d);
		location = XMVectorMultiplyAdd(s, axis, location);
	}

	void ZoomTo(Actor* actor)
	{
		//Trace the camera down the line its pointing towards the actor
		XMVECTOR actorPos = actor->GetPositionVector() - (forward * 5.f);
		location = actorPos;
	}

	void FrustumCullTest(ActorSystem& system)
	{
		//OpenMP Test
#pragma omp parallel for
		for (int i = 0; i < system.actors.size(); i++)
		{
			XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);

			XMMATRIX world = system.actors[i]->GetTransformationMatrix();
			XMMATRIX invWorld = XMMatrixInverse(&XMMatrixDeterminant(world), world);

			XMMATRIX viewToLocal = XMMatrixMultiply(invView, invWorld);

			BoundingFrustum frustum, localSpaceFrustum;
			BoundingFrustum::CreateFromMatrix(frustum, proj);
			frustum.Transform(localSpaceFrustum, viewToLocal);

			system.boundingBox.Center = system.actors[i]->GetPositionFloat3();
			system.boundingBox.Extents = system.actors[i]->GetScale();

			if (localSpaceFrustum.Contains(system.boundingBox) == DirectX::DISJOINT)
			{
				system.actors[i]->bRender = false;
			}
			else
			{
				system.actors[i]->bRender = true;
			}
		}
	}

	void AttachTo(Actor* actor)
	{
		actorAttachedTo = actor;
		attachedOffset = this->location - actor->GetPositionVector();

		location = actor->GetPositionVector();
		location += attachedOffset;
	}

	XMMATRIX view, proj;

	XMVECTOR location;
	XMVECTOR focusPoint;
	XMVECTOR worldUp;
	XMVECTOR forward, up, right;

	XMVECTOR attachedOffset;
	class Actor* actorAttachedTo = nullptr;

	bool bEditorCamera = false;
};

export Camera editorCamera(XMVectorSet(0.f, 0.f, -5.f, 1.f));
export Camera playerCamera(XMVectorSet(0.f, 0.f, 1.f, 1.f));
export Camera* activeCamera;

export Camera* GetActiveCamera()
{
	return activeCamera;
}

export void SetActiveCamera(Camera* camera)
{
	activeCamera = camera;
}