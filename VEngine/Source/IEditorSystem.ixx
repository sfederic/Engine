module;
import <string>;
export module IEditorSystem;

enum class EDockFocus
{
	None,
	Properties,
	RenderView,
	Console,
	Assets,
	WorldList
};

class Actor;

//Interface to allow for swapping between editor and 'OS window only' functions.
export class IEditorSystem
{
public:
	virtual void Init(int argc, char** argv) = 0;
	virtual void Tick() = 0;
	virtual void PopulateWorldList() = 0;
	virtual void PopulateActorSystemList() = 0;
	virtual void ProcessEvents() = 0;
	virtual void DisplayActorSystemProperties(Actor* actor) = 0;
	virtual void SetWindowWidthHeight() = 0;
	virtual void SetDockFocus(EDockFocus focus) = 0;
	virtual int GetDockFocus() = 0;
	virtual void GetMousePos(int* x, int* y) = 0;
	virtual void ToggleFullscreen() = 0;
	virtual void EnableEditorDocks() = 0;
	virtual void DisableEditorDocks() = 0;
	virtual void Print(const std::string& msg) = 0;

	//For the current setup, this is just going to be cast as a HWND everywhere
	void* mainWindow;
};
