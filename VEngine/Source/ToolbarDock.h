#pragma once

#include <qdockwidget.h>

class ToolbarDock : public QDockWidget
{
public:
	ToolbarDock(const char* title);
	void Tick();
	void SetPlayMode();
	void PauseViewportInPlayMode();
};
