module;
import IEditorSystem;
export module NativeEditorSystem;

#include "IEditorSystem.h"

//For any 'native' editor (Win32) stuff for builds. 
//Maybe can even get away with using Imgui or D2D as a Qt replacement.
export class NativeEditorSystem : public IEditorSystem
{
public:
	virtual void NativeEditorSystem::Init(int argc, char** argv) override
	{
		gCoreSystem.windowWidth = 800;
		gCoreSystem.windowHeight = 600;
		gCoreSystem.SetupWindow(GetModuleHandle(NULL), SW_SHOW);
		mainWindow = gCoreSystem.mainWindow;
	}

	virtual void Tick() {}
	virtual void PopulateWorldList() {}
	virtual void PopulateActorSystemList() {}
	virtual void ProcessEvents() {}
	virtual void DisplayActorSystemProperties(Actor* actor) {}
	virtual void SetWindowWidthHeight() {}
	virtual void SetDockFocus(EDockFocus focus) {}
	virtual int GetDockFocus() { return 0; }
	virtual void GetMousePos(int* x, int* y) {}
	virtual void ToggleFullscreen() {}
	virtual void EnableEditorDocks() {}
	virtual void DisableEditorDocks() {}
	virtual void Print(const std::string& msg) {}
};
