module;
#include <QtWidgets/QMainWindow>
#include "ui_EditorMainWindow.h"
export module EditorMainWindow;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

enum class EEditorTheme
{
	Light, 
	Dark
};

export class EditorMainWindow : public QMainWindow
{
    Q_OBJECT

public:
	//Main Qt Application
	QApplication* gQApplication;

	//Docks
	ToolbarDock* toolbarDock;
	WorldDock* worldDock;
	PropertiesDock* propertiesDock;
	AssetDock* assetDock;
	ConsoleDock* consoleDock;

	RenderViewWidget* renderViewWidget;

	EDockFocus currentDockFocus;

    Ui::EditorMainWindowClass ui;

	//Tabs
	QTabWidget* editorTabWidget;
	QMainWindow* worldEditorWindow;

	EEditorTheme editorTheme = EEditorTheme::Dark;

    void Init(int argc)
    {
        gQApplication = new QApplication(argc, nullptr);
    }

    EditorMainWindow(QWidget* parent) : QMainWindow(parent)
    {
        ui.setupUi(this);

        if (editorTheme == EEditorTheme::Dark)
        {
            //Stole all the scrollbar stuff from https://stackoverflow.com/questions/54595957/how-to-set-a-stylesheet-for-the-qscrollbar-in-a-qscrollarea
            qApp->setStyleSheet(
                "QMainWindow { background-color: rgb(255, 0, 0); border: none; } "

                "QDockWidget { background-color: rgb(37, 33, 34); border:0px; } "

                "QWidget { background-color: rgb(40, 40, 40); border-color: rgb(11, 11, 11); border-width: 1.2px; border-style: inset; } "

                "QPushButton { min-height:30px; background-color: rgb(40, 40, 43); border-radius: 5px; } "
                "QPushButton:hover { background-color: rgb(80, 80, 80); } "
                "QPushButton:pressed { background-color: rgb(140, 140, 140); } "

                "QHeaderView { color: rgb(210, 210, 210); border: 0px; } "
                "QHeaderView:section { background-color: rgb(40, 40, 40); } "

                "QTabWidget { border: none; } "
                "QTabBar::tab { background: rgb(66, 66, 66); } "

                "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: none;  border: 0px; } "
                "QScrollBar:vertical { background-color: #2A2929; width: 15px; margin: 15px 3px 15px 3px; border: 1px transparent #2A2929; border-radius: 4px } "
                "QScrollBar::handle:vertical { background-color: rgb(11, 11, 11); min-height: 5px; border-radius: 4px; } "
                "QScrollBar::sub-line:vertical { margin: 3px 0px 3px 0px; height: 10px; border-image: url(:/qss_icons/rc/up_arrow_disabled.png); width: 10px; subcontrol-position: top; subcontrol-origin: margin; } "
                "QScrollBar::add-line:vertical { margin: 3px 0px 3px 0px; border-image: url(:/qss_icons/rc/down_arrow_disabled.png); height: 10px;width: 10px; subcontrol-position: bottom; subcontrol-origin: margin; } "

                "QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal { background: none;  border: 0px; } "
                "QScrollBar:horizontal { height: 15px; margin: 3px 15px 3px 15px; border: 1px transparent #2A2929; border-radius: 4px; } "
                "QScrollBar::handle:horizontal { background-color: rgb(11, 11, 11); min-width: 5px; border-radius: 4px; } "
                "QScrollBar::add-line:horizontal{ margin: 0px 3px 0px 3px; border-image: url(:/qss_icons/rc/right_arrow_disabled.png); width: 10px; height: 10px; subcontrol - position: right; subcontrol - origin: margin; } "
                "QScrollBar::sub-line:horizontal { margin: 0px 3px 0px 3px; border-image: url(:/qss_icons/rc/left_arrow_disabled.png); height: 10px; width: 10px; subcontrol-position: left; subcontrol-origin: margin; } "
            );

            //Set font
            QFont font("Din");
            font.setStyleHint(QFont::Monospace);
            font.setPixelSize(14);
            qApp->setFont(font);

            //Set global font colour
            QPalette p = qApp->palette();
            QColor fontColour = QColor::fromRgb(210, 210, 210);
            p.setColor(QPalette::Text, fontColour);
            p.setColor(QPalette::WindowText, fontColour);
            p.setColor(QPalette::ButtonText, fontColour);
            p.setColor(QPalette::BrightText, fontColour);
            QApplication::setPalette(p);
        }

        //Menu for recreating closed tabs and docks
        QMenu* windowMenu = menuBar()->addMenu("Window");
        auto console = new QAction("Console");
        connect(console, &QAction::triggered, this, &CreateConsoleDock);
        windowMenu->addAction(console);

        //WORLD EDITOR TAB
        editorTabWidget = new QTabWidget();
        connect(editorTabWidget, &QTabWidget::tabCloseRequested, this, &CloseTab);
        setCentralWidget(editorTabWidget);
        editorTabWidget->setTabsClosable(true);

        worldEditorWindow = new QMainWindow();
        editorTabWidget->addTab(worldEditorWindow, "World Editor");

        //Toolbar dock
        toolbarDock = new ToolbarDock("Toolbar");
        worldEditorWindow->addDockWidget(Qt::TopDockWidgetArea, toolbarDock);

        //World list
        worldDock = new WorldDock("World");
        worldEditorWindow->addDockWidget(Qt::LeftDockWidgetArea, worldDock);

        //Properties dock
        propertiesDock = new PropertiesDock("Properties");
        worldEditorWindow->addDockWidget(Qt::RightDockWidgetArea, propertiesDock);

        //Asset dock
        assetDock = new AssetDock("Assets");
        worldEditorWindow->addDockWidget(Qt::BottomDockWidgetArea, assetDock);

        //Console dock
        consoleDock = new ConsoleDock();
        worldEditorWindow->addDockWidget(Qt::BottomDockWidgetArea, consoleDock);
        //connect(consoleDock, &QDockWidget::close, this, &CloseConsoleDock);

        //Central widget - Viewport
        {
            renderViewWidget = new RenderViewWidget();
            worldEditorWindow->setCentralWidget(renderViewWidget);
            //centralWidget()->setFixedSize(QSize(1000, 600));
            QSize size = renderViewWidget->size();
            gCoreSystem.windowWidth = size.width();
            gCoreSystem.windowHeight = size.height();
        }

        //last setup stage
        setWindowState(Qt::WindowMaximized);
        show();
        setWindowTitle("VEngine");
    }

    void closeEvent(QCloseEvent* event)
    {
        gCoreSystem.bMainLoop = false;
    }


    bool nativeEvent(const QByteArray& eventType, void* message, long* result)
    {
        MSG* msg = (MSG*)message;

        if (ImGui_ImplWin32_WndProcHandler(msg->hwnd, msg->message, msg->wParam, msg->lParam))
        {
            return true;
        }

        switch (msg->message)
        {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        case WM_KEYDOWN:
            gInputSystem.StoreKeyDownInput(msg->wParam);

            //Close editor
            if (msg->wParam == VK_ESCAPE)
            {

            }

            break;

        case WM_KEYUP:
            gInputSystem.StoreKeyUpInput(msg->wParam);
            break;

        case WM_LBUTTONUP:
            gInputSystem.StoreMouseLeftUpInput();
            break;

        case WM_LBUTTONDOWN:
            gInputSystem.StoreMouseLeftDownInput();
            break;

        case WM_RBUTTONUP:
            gInputSystem.StoreMouseRightUpInput();
            break;

        case WM_MBUTTONDOWN:
            gInputSystem.StoreMouseMiddleDownInput();
            break;

        case WM_MBUTTONUP:
            gInputSystem.StoreMouseMiddleUpInput();
            break;

        case WM_RBUTTONDOWN:
            gInputSystem.StoreMouseRightDownInput();
            break;

        case WM_MOUSEWHEEL:
            if (GET_WHEEL_DELTA_WPARAM(msg->wParam) < 0)
            {
                gInputSystem.StoreMouseWheelDown();
            }
            else
            {
                gInputSystem.StoreMouseWheelUp();
            }
            break;
        }

        return false;
    }

    void Tick()
    {
        toolbarDock->Tick();
        assetDock->Tick();
        worldDock->Tick();
        propertiesDock->Tick();
        renderViewWidget->Tick();
    }

    void CloseTab(int tabIndex)
    {
        editorTabWidget->removeTab(tabIndex);
    }

    void CreateConsoleDock()
    {
        if (consoleDock == nullptr)
        {
            consoleDock = new ConsoleDock();
            consoleDock->setFloating(true);
            worldEditorWindow->addDockWidget(Qt::DockWidgetArea::NoDockWidgetArea, consoleDock);
        }
    }

    void CloseConsoleDock()
    {
        delete consoleDock;
        consoleDock = nullptr;
    }
};
