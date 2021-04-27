#include "ToolbarDock.h"
#include <qlayout.h>
#include <qtoolbutton.h>
#include <qdockwidget.h>
#include "ActorSystemFactory.h"
#include <qpushbutton.h>
#include <assert.h>
#include <qlineedit.h>
#include <qdialog.h>
#include <qlabel.h>
#include <qcombobox.h>

ToolbarDock::ToolbarDock(const char* title) : QDockWidget(title)
{
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

    //Add Actorsystem button
    QPushButton* createActorSystemButton = new QPushButton();
    createActorSystemButton->setText("New Actor System");
    connect(createActorSystemButton, &QToolButton::clicked, this, &ToolbarDock::CreateNewActorSystem);
    toolbarHLayout->addWidget(createActorSystemButton);

    toolbarHLayout->addStretch(0);

    QWidget* toolbarWidget = new QWidget();
    toolbarWidget->setLayout(toolbarHLayout);

    setWidget(toolbarWidget);
    setMaximumHeight(90);
}

void ToolbarDock::Tick()
{

}

void ToolbarDock::CreateNewActorSystem()
{
    //Qt dialog to create system specifics
    auto createActorSystemWindow = new QDialog();
    createActorSystemWindow->setWindowTitle("Create New Actor System");

    auto grid = new QGridLayout();

    grid->addWidget(new QLabel("Actor System Name: "), 0, 0);
    grid->addWidget(new QLineEdit(), 0, 1);
    
    grid->addWidget(new QLabel("Actor Name: "), 1, 0);
    grid->addWidget(new QLineEdit(), 1, 1);

    grid->addWidget(new QLabel("Super class: "), 2, 0);
    grid->addWidget(new QComboBox(), 2, 1);

    auto createActorButton = new QPushButton("Create");
    grid->addWidget(createActorButton, 3, 1);

    createActorSystemWindow->setLayout(grid);
    createActorSystemWindow->show();
}
