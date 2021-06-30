module;
#include <qdockwidget.h>
export module ToolbarDock;

export class ToolbarDock : public QDockWidget
{
public:
    QDialog* createActorSystemWindow;
    QLineEdit* actorSystemNameLineEdit;
    QLineEdit* actorNameLineEdit;

    ToolbarDock(const char* title) : QDockWidget(title)
    {
        QHBoxLayout* toolbarHLayout = new QHBoxLayout();

        QToolButton* saveToolButton = new QToolButton();
        QPixmap saveToolPixmap = QPixmap("test.png");
        QIcon saveToolIcon = QIcon(saveToolPixmap);
        saveToolButton->setIcon(saveToolIcon);
        saveToolButton->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
        saveToolButton->setText("Save");
        toolbarHLayout->addWidget(saveToolButton, Qt::AlignLeft);

        //Play button
        QToolButton* playToolButton = new QToolButton();
        QPixmap playToolPixmap = QPixmap("play_icon.png");
        QIcon playToolIcon = QIcon(playToolPixmap);
        playToolButton->setIcon(playToolIcon);
        playToolButton->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
        playToolButton->setText("Play");
        connect(playToolButton, &QToolButton::clicked, this, &ToolbarDock::SetPlayMode);
        toolbarHLayout->addWidget(playToolButton, Qt::AlignLeft);

        //Pause button
        QToolButton* pauseToolButton = new QToolButton();
        pauseToolButton->setText("Pause");
        connect(pauseToolButton, &QToolButton::clicked, this, &ToolbarDock::PauseViewportInPlayMode);

        toolbarHLayout->addStretch(0);

        QWidget* toolbarWidget = new QWidget();
        toolbarWidget->setLayout(toolbarHLayout);

        setWidget(toolbarWidget);
        setMaximumHeight(90);
    }

    void Tick()
    {

    }

    void SetPlayMode()
    {
        gCoreSystem.bGamePlayOn = !gCoreSystem.bGamePlayOn;

        if (gCoreSystem.bGamePlayOn)
        {
            gEditorSystem->DisableEditorDocks();

            GetWorld()->StartAllActorSystems();
            gUISystem.StartAllWidgets();
        }
        else if (!gCoreSystem.bGamePlayOn)
        {
            gEditorSystem->EnableEditorDocks();
        }
    }

    void PauseViewportInPlayMode()
    {
        if (gCoreSystem.bGamePlayOn)
        {
            gCoreSystem.bGamePaused = !gCoreSystem.bGamePaused;
        }
    }
};
