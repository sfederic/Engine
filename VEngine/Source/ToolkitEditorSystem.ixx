module;
#include "../EditorMainWindow.h"
export module ToolkitEditorSystem;

QApplication* qApplication;
EditorMainWindow* editorMainWindow;

EDockFocus currentDockFocus;

//A huge wrapper around anything 'editor' (Qt) related so the engine can get around #ifdefs
//that handle game deployment and debugging in base windows (win32). 'Toolkit' here refers to Qt.
export class ToolkitEditorSystem : public IEditorSystem
{
public:
	virtual void Init(int argc, char** argv)
	{
		qApplication = new QApplication(argc, argv);
		editorMainWindow = new EditorMainWindow();
		mainWindow = (void*)editorMainWindow->renderViewWidget->winId();
	}

	virtual void Tick()
	{
		editorMainWindow->Tick();
		ToggleFullscreen();
	}

	virtual void PopulateWorldList()
	{
		editorMainWindow->worldDock->PopulateWorldList();
	}

	virtual void PopulateActorSystemList()
	{
		editorMainWindow->worldDock->PopulateActorSystemList();
	}

	virtual void ProcessEvents()
	{
		qApplication->processEvents();
	}

	virtual void DisplayActorSystemProperties(Actor* actor)
	{
		editorMainWindow->propertiesDock->DisplayActorSystemProperties(actor);
	}

	virtual void SetWindowWidthHeight()
	{
		gCoreSystem.windowWidth = editorMainWindow->renderViewWidget->size().width();
		gCoreSystem.windowHeight = editorMainWindow->renderViewWidget->size().height();
	}

	virtual void SetDockFocus(EDockFocus focus)
	{
		currentDockFocus = focus;
	}

	virtual int GetDockFocus()
	{
		return (int)currentDockFocus;
	}

	virtual void GetMousePos(int* x, int* y)
	{
		QPoint p = editorMainWindow->renderViewWidget->mapFromGlobal(QCursor::pos());
		*x = p.x();
		*y = p.y();
	}

	virtual void ToggleFullscreen()
	{
		static bool fullscreen;

		if (gInputSystem.GetKeyUpState(Keys::F11))
		{
			fullscreen = !fullscreen;

			if (fullscreen)
			{
				editorMainWindow->propertiesDock->hide();
				editorMainWindow->assetDock->hide();
				editorMainWindow->consoleDock->hide();
				editorMainWindow->toolbarDock->hide();
				editorMainWindow->worldDock->hide();
			}
			else if (!fullscreen)
			{
				editorMainWindow->propertiesDock->show();
				editorMainWindow->assetDock->show();
				editorMainWindow->consoleDock->show();
				editorMainWindow->toolbarDock->show();
				editorMainWindow->worldDock->show();
			}
		}
	}

	//With both of these functions, want to keep the console dock enabled coz debugging is sick
	virtual void EnableEditorDocks()
	{
		editorMainWindow->assetDock->setEnabled(true);
		editorMainWindow->propertiesDock->setEnabled(true);
		editorMainWindow->worldDock->setEnabled(true);
	}

	virtual void DisableEditorDocks()
	{
		editorMainWindow->assetDock->setEnabled(false);
		editorMainWindow->propertiesDock->setEnabled(false);
		editorMainWindow->worldDock->setEnabled(false);
	}

	virtual void Print(const std::string& msg)
	{
		editorMainWindow->consoleDock->consoleMessageBox->insertPlainText(QString(msg.c_str()));
		editorMainWindow->consoleDock->consoleMessageBox->moveCursor(QTextCursor::MoveOperation::End);
	}
};
