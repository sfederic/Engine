#include "RenderSystem.h"
#include "CoreSystem.h"
#include "UISystem.h"
#include "Obj.h"
#include "Camera.h"
#include "Audio.h"
#include "AudioSystem.h"
#include "Input.h"
#include "Actor.h"
#include <thread>
#include "ShaderFactory.h"
#include <omp.h>
#include "DebugMenu.h"
#include "Physics.h"
#include "World.h"
#include "FileSystem.h"
#include "Debug.h"
#include "FBXImporter.h"

int __stdcall WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR cmdLine, int cmdShow)
{
	FBXImporter::Init();

	coreSystem.SetupWindow(instance, cmdShow);
	coreSystem.SetTimerFrequency();
	renderSystem.Init();
	audioSystem.Init();
	uiSystem.Init();

	//D3D11Buffer* debugLinesBuffer = renderSystem.CreateDefaultBuffer(sizeof(Vertex) * 1024, D3D11_BIND_VERTEX_BUFFER, debugLineData);

	//ACTOR SYSTEM TESTING
	ActorSystem system;
	system.modelName = "cube.fbx";
	system.CreateActors(&renderSystem, 1);

	//World data testing
	World* world = GetWorld();
	world->AddActorSystem(system);

	//MAIN LOOP
	while (coreSystem.msg.message != WM_QUIT)
	{
		const float deltaTime = coreSystem.deltaTime;

		coreSystem.StartTimer();
		coreSystem.HandleMessages();

		g_FileSystem.Tick();
		uiSystem.Tick();
		editorCamera.Tick(deltaTime);

		if (inputSystem.GetKeyUpState(VK_UP))
		{
			world->AddActorSystem(system);
		}
		if (inputSystem.GetKeyUpState(VK_DOWN))
		{
			world->RemoveActorSystem(EActorSystemID::Actor);
		}

		renderSystem.Tick();
		renderSystem.RenderSetup(deltaTime);
		for (int i = 0; i < world->actorSystems.size(); i++)
		{
			renderSystem.RenderActorSystem(&world->actorSystems[i]);
		}
		renderSystem.RenderBounds();
		renderSystem.RenderEnd(deltaTime);

		inputSystem.InputReset();

		coreSystem.EndTimer();
	}

	//TODO: Add the rest of cleanup code
	//So far only D2D gives out mem leaks
	uiSystem.Cleanup();

	return (int)coreSystem.msg.wParam;
}
