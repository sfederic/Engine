module;
import <vector>;
import <string>;
export module DebugMenu;

enum class EMenuID
{
	ACTORS,
	ACTORSYSTEMS,
	RENDERING
};

struct MenuItem
{
	MenuItem(const wchar_t* initName, EMenuID id)
	{
		wcsncpy_s(name, initName, 32);
		menuID = id;
	}

	std::vector<const wchar_t*> subMenuItems;
	wchar_t name[32];
	EMenuID menuID;
};

struct DebugNotification
{
	DebugNotification(const wchar_t* note)
	{
		text = note;
		timeOnScreen = 0.f;
	}

	std::wstring text;
	float timeOnScreen;
};

//A stats UI system for showing debug-level information. Encapsulates Immediate Mode GUI as well.
export class DebugMenu
{
public:
	std::vector<MenuItem> menuItems;
	std::vector<DebugNotification> notifications;

	bool bProfileMenuOpen = false;
	bool bFPSMenuOpen = false;
	bool bGPUMenuOpen = false;
	bool bSnapMenuOpen = false;
	bool bActorStatsMenuOpen = false;
	bool bActorSpawnMenuOpen = false;
	bool bActorInspectMenuOpen = false;

	void Init()
	{
		//IMGUI setup
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;

		//Imgui has an .ini file to save previous ui positions and values.
		//Setting this to null removes this initial setup.
		io.IniFilename = nullptr;

		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

		ImGui::StyleColorsDark();
		ImGui_ImplWin32_Init((HWND)gEditorSystem->mainWindow);
		ImGui_ImplDX11_Init(gRenderSystem.device, gRenderSystem.context);
	}

	void Tick(World* world, float deltaTime)
	{
		//Start ImGui
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		RenderNotifications(deltaTime);

		//Keep in mind ImGuizmo has to be called here, it's part of ImGui
		gTransformGizmo.Tick();

		RenderFPSMenu(deltaTime);
		RenderGPUMenu();
		RenderProfileMenu();
		RenderSnappingMenu();
		RenderActorStatsMenu();
		RenderActorSpawnMenu();
		RenderActorInspectMenu();

		ImGui::EndFrame();
	}

	void Cleanup()
	{
		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}

	void AddNotification(const wchar_t* note)
	{
		notifications.push_back(DebugNotification(note));
	}

	//Handle notifications (eg. "Shaders recompiled", "ERROR: Not X", etc)
	void RenderNotifications(float deltaTime)
	{
		float textOffsetX = 20.f;

		const float notificationLifetime = 3.0f;
		for (int i = 0; i < notifications.size(); i++)
		{
			if (notifications[i].timeOnScreen < notificationLifetime)
			{
				notifications[i].timeOnScreen += deltaTime;

				float notificationOffsetY = 20.f * i;
				gUISystem.d2dRenderTarget->DrawTextA(notifications[i].text.c_str(), notifications[i].text.size(), gUISystem.textFormat,
					{ 0.f, notificationOffsetY, 1000.f, 1000.f }, gUISystem.brushText);
			}
			else
			{
				notifications.erase(notifications.begin() + i);
			}
		}
	}

	void RenderFPSMenu(float deltaTime)
	{
		if (bFPSMenuOpen)
		{
			ImGui::Begin("FPS");

			ImGui::Text("FPS: %d", gCoreSystem.finalFrameCount);
			ImGui::Text("GPU Render Time: %f", gRenderSystem.renderTime);
			ImGui::Text("Delta Time (ms): %f", deltaTime);
			ImGui::Text("Time Since Startup: %f", gCoreSystem.timeSinceStartup);

			ImGui::End();
		}
	}

	void RenderGPUMenu()
	{
		if (bGPUMenuOpen)
		{
			ImGui::Begin("GPU Info");

			DXGI_ADAPTER_DESC1 adapterDesc = gRenderSystem.adaptersDesc.front();

			ImGui::Text("Device: %ls", adapterDesc.Description);
			ImGui::Text("System Memory: %zu", adapterDesc.DedicatedSystemMemory);
			ImGui::Text("Video Memory: %zu", adapterDesc.DedicatedVideoMemory);
			ImGui::Text("Shared System Memory: %zu", adapterDesc.SharedSystemMemory);
			ImGui::Spacing();

			static bool showAllDevices;
			if (!showAllDevices && ImGui::Button("Show all Devices"))
			{
				showAllDevices = true;
			}
			else if (showAllDevices && ImGui::Button("Hide all Devices"))
			{
				showAllDevices = false;
			}

			if (showAllDevices)
			{
				for (int i = 1; i < gRenderSystem.adaptersDesc.size(); i++)
				{
					ImGui::Text("Device: %ls", gRenderSystem.adaptersDesc[i].Description);
					ImGui::Text("System Memory: %zu", gRenderSystem.adaptersDesc[i].DedicatedSystemMemory);
					ImGui::Text("Video Memory: %zu", gRenderSystem.adaptersDesc[i].DedicatedVideoMemory);
					ImGui::Text("Shared System Memory: %zu", gRenderSystem.adaptersDesc[i].SharedSystemMemory);
					ImGui::Spacing();
				}
			}

			ImGui::End();
		}
	}

	void RenderProfileMenu()
	{
		if (bProfileMenuOpen)
		{
			ImGui::Begin("Profiler Time Frames");

			for (auto& timeFrame : gProfiler.timeFrames)
			{
				ImGui::Text(timeFrame.first.c_str());
				double time = timeFrame.second->GetAverageTime();
				ImGui::Text(std::to_string(time).c_str());
			}

			ImGui::End();
		}
	}

	void RenderSnappingMenu()
	{
		if (bSnapMenuOpen)
		{
			ImGui::Begin("Snapping");

			ImGui::InputFloat("Translation", &gTransformGizmo.translateSnapValues[0]);
			gTransformGizmo.translateSnapValues[1] = gTransformGizmo.translateSnapValues[0];
			gTransformGizmo.translateSnapValues[2] = gTransformGizmo.translateSnapValues[0];

			ImGui::InputFloat("Rotation", &gTransformGizmo.rotationSnapValues[0]);
			gTransformGizmo.rotationSnapValues[1] = gTransformGizmo.rotationSnapValues[0];
			gTransformGizmo.rotationSnapValues[2] = gTransformGizmo.rotationSnapValues[0];

			ImGui::InputFloat("Scale", &gTransformGizmo.scaleSnapValues[0]);
			gTransformGizmo.scaleSnapValues[1] = gTransformGizmo.scaleSnapValues[0];
			gTransformGizmo.scaleSnapValues[2] = gTransformGizmo.scaleSnapValues[0];

			if (gTransformGizmo.currentTransformMode == ImGuizmo::MODE::LOCAL)
			{
				ImGui::Text("LOCAL");
			}
			else if (gTransformGizmo.currentTransformMode == ImGuizmo::MODE::WORLD)
			{
				ImGui::Text("WORLD");
			}

			ImGui::End();
		}
	}

	void RenderActorStatsMenu()
	{
		if (bActorStatsMenuOpen)
		{
			ImGui::Begin("Actor Stats");

			World* world = GetWorld();
			ImGui::Text("Num actors rendered: %d", world->GetNumOfActorsInWorld());

			ImGui::End();
		}
	}

	void RenderActorSpawnMenu()
	{
		if (bActorSpawnMenuOpen)
		{
			ImGui::Begin("Active Actor System");
			ImGui::SetWindowPos(ImVec2(10, 10));
			ImGui::SetWindowSize(ImVec2(200, 50));

			ActorSystem* activeAS = ActorSystemFactory::GetCurrentActiveActorSystem();
			if (activeAS)
			{
				ImGui::Text("%s", activeAS->name.c_str());
			}
			else
			{
				ImGui::Text("None");
			}

			ImGui::End();
		}
	}

	//Shows actor/system stats when hovering over actor (Idea stolen from Fledge Engine)
	//REF:https://www.youtube.com/watch?v=WjPiJn9dkxs
	void RenderActorInspectMenu()
	{
		if (bActorInspectMenuOpen)
		{
			ImGui::Begin("Actor Inspect");
			ImGui::SetWindowPos(ImVec2(gUISystem.mousePos.x, gUISystem.mousePos.y));
			ImGui::SetWindowSize(ImVec2(300, 250));

			Ray ray;
			if (RaycastAllFromScreen(ray))
			{
				Actor* actor = ray.hitActor;
				if (actor)
				{
					//TODO: this is just dummy data to see what the menu can do.
					//Come back here when there is more actor specific instance data to show
					//eg. materials, buffers, other weird things
					ImGui::Text("Linked System: %s", actor->linkedActorSystem->name.c_str());
					ImGui::Text("Shader: %s", actor->linkedActorSystem->shaderName.c_str());
					ImGui::Text("Texture: %s", actor->linkedActorSystem->textureName.c_str());
					ImGui::Text("Model: %s", actor->linkedActorSystem->modelName.c_str());
				}
			}

			ImGui::End();
		}
	}
};

export DebugMenu gDebugMenu;
