#include "EditorMainWindow.h"
#include <qboxlayout.h>
#include <qtoolbutton.h>
#include <qdockwidget.h>
#include <qlistwidget.h>
#include "WorldWidget.h"
#include <qfilesystemmodel.h>
#include "RenderViewWidget.h"
#include "PropertiesWidget.h"
#include "CoreSystem.h"

EditorMainWindow::EditorMainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

    //Toolbar dock
    QHBoxLayout* toolbarHLayout = new QHBoxLayout();

    QToolButton* saveToolButton = new QToolButton();
    QPixmap saveToolPixmap = QPixmap("test.png");
    QIcon saveToolIcon = QIcon(saveToolPixmap);
    saveToolButton->setIcon(saveToolIcon);
    saveToolButton->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
    saveToolButton->setText("Save");
    toolbarHLayout->addWidget(saveToolButton, Qt::AlignLeft);

    QToolButton* playToolButton = new QToolButton();
    QPixmap playToolPixmap = QPixmap("play_icon.png");
    QIcon playToolIcon = QIcon(playToolPixmap);
    playToolButton->setIcon(playToolIcon);
    playToolButton->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
    playToolButton->setText("Play");
    toolbarHLayout->addWidget(playToolButton, Qt::AlignLeft);


    //toolbarHLayout->setSpacing(1);
    toolbarHLayout->addStretch(0);

    QWidget* toolbarWidget = new QWidget();
    toolbarWidget->setLayout(toolbarHLayout);


    QDockWidget* toolbarDock = new QDockWidget("Toolbar");
    toolbarDock->setWidget(toolbarWidget);
    toolbarDock->setMaximumHeight(90);
    addDockWidget(Qt::TopDockWidgetArea, toolbarDock);

    //World list
    WorldWidget* worldWidget = new WorldWidget();

    QDockWidget* levelLayoutDock = new QDockWidget("World");
    levelLayoutDock->setWidget(worldWidget);
    addDockWidget(Qt::LeftDockWidgetArea, levelLayoutDock);

    //Properties dock
    QDockWidget* propertiesDock = new QDockWidget("Properties");
    addDockWidget(Qt::RightDockWidgetArea, propertiesDock);

    //Asset dock
    QDirIterator assetDirectory(QDir::currentPath(), QDir::Dirs);

    QListWidget* assetList = new QListWidget();
    while (assetDirectory.hasNext())
    {
        assetList->addItem(assetDirectory.next());
    }

    connect(assetList, &QListWidget::itemClicked, this, &EditorMainWindow::AssetItemClicked);

    QListWidget* assetIcons = new QListWidget();
    QPixmap iconImage = QPixmap("Editor/Icons/test.png");
    QIcon icon = QIcon(iconImage);
    QListWidgetItem* iconItem = new QListWidgetItem(icon, "testIcon");

    QListWidgetItem* iconItem2 = new QListWidgetItem(icon, "testIcon");

    assetIcons->addItem(iconItem);
    assetIcons->addItem(iconItem2);

    assetIcons->setIconSize(QSize(75, 75));
    assetIcons->setViewMode(QListView::ViewMode::IconMode);

    QHBoxLayout* assetHBox = new QHBoxLayout();
    assetHBox->addWidget(assetList, Qt::AlignLeft);
    assetHBox->addWidget(assetIcons, Qt::AlignRight);

    QWidget* assetWidget = new QWidget();
    assetWidget->setLayout(assetHBox);

    QDockWidget* assetDock = new QDockWidget("Assets");
    assetDock->setWidget(assetWidget);
    addDockWidget(Qt::BottomDockWidgetArea, assetDock);


    QFileSystemModel* fileModel = new QFileSystemModel();
    fileModel->setRootPath(QDir::currentPath());

    //TODO: Console dock (tab it within asset/prefab window)

    //Central widget
    setCentralWidget(&mainWidget);
    centralWidget()->setFixedSize(QSize(1000, 600));
    QSize size = centralWidget()->size();
    gCoreSystem.windowWidth = size.width();
    gCoreSystem.windowHeight = size.height();

    //Properties dock
    propWidget = new PropertiesWidget();
    propertiesDock->setWidget(propWidget);
}

bool EditorMainWindow::nativeEvent(const QByteArray& eventType, void* message, long* result)
{
    MSG* msg = (MSG*)message;

    switch (msg->message)
    {
    case WM_DESTROY:
        PostQuitMessage(0);

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
        gInputSystem.StoreMouseLeftUpInput(msg->wParam);
        break;

    case WM_LBUTTONDOWN:
        gInputSystem.StoreMouseLeftDownInput(msg->wParam);
        break;

    case WM_RBUTTONUP:
        gInputSystem.StoreMouseRightUpInput(msg->wParam);
        break;

    case WM_RBUTTONDOWN:
        gInputSystem.StoreMouseRightDownInput(msg->wParam);
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

void EditorMainWindow::AssetItemClicked(QListWidgetItem* listWidgetItem)
{
    //TODO: set text into asset icon list etc.
     //listWidgetItem->text()
}