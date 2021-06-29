module;
export module WorldDock;

export class WorldDock : public QDockWidget
{
public: 
    QListWidget* actorSystemList;
    QLineEdit* worldSearch;
    QStringList worldStringList;
    QList<QTreeWidgetItem*> worldTreeItems;
    QTreeWidget* worldTreeList;

    WorldDock(const char* title) : QDockWidget(title)
    {
        //Add/Create new actorsystem button
        auto newActorSystemButton = new QPushButton("New");
        connect(newActorSystemButton, &QPushButton::clicked, this, &CreateNewActorSystem);

        auto addActorSystemButton = new QPushButton("Add");
        connect(addActorSystemButton, &QPushButton::clicked, this, &AddExistingActorSystem);

        //Search bar
        worldSearch = new QLineEdit();
        worldSearch->setPlaceholderText("Search");
        connect(worldSearch, &QLineEdit::textChanged, this, &SearchWorldList);

        //Actor systems list to choose actor spawning 
        actorSystemList = new QListWidget();
        actorSystemList->setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);
        connect(actorSystemList, &QListWidget::itemClicked, this, &ClickOnActorSystem);

        PopulateActorSystemList();

        QVBoxLayout* worldVLayout = new QVBoxLayout();
        worldVLayout->addWidget(newActorSystemButton);
        worldVLayout->addWidget(addActorSystemButton);
        worldVLayout->addWidget(worldSearch);

        //currently active actor systems in world list
        worldTreeList = new QTreeWidget();

        connect(worldTreeList, &QTreeWidget::itemClicked, this, &ClickOnActor);
        worldTreeList->setColumnCount(1);
        worldTreeList->setHeaderLabels(QStringList("Active Actor Systems"));

        worldVLayout->addWidget(actorSystemList);
        worldVLayout->addWidget(worldTreeList);
        setLayout(worldVLayout);

        QWidget* worldWidget = new QWidget();
        worldWidget->setLayout(worldVLayout);
        setWidget(worldWidget);
    }

    void Tick()
    {

    }

    void PopulateActorSystemList()
    {
        actorSystemList->clear();
        worldStringList.clear();

        auto& actorSystems = GetWorld()->actorSystems;

        for (int i = 0; i < actorSystems.size(); i++)
        {
            QString name = QString::fromStdString(actorSystems[i]->name);
            worldStringList.append(name);
        }

        actorSystemList->addItems(worldStringList);
    }

    void PopulateWorldList()
    {
        worldTreeItems.clear();
        worldTreeList->clear();

        World* world = GetWorld();

        for (int i = 0; i < world->actorSystems.size(); i++)
        {
            worldTreeItems.append(new QTreeWidgetItem(worldTreeList, QStringList(QString::fromStdString(world->actorSystems[i]->name))));

            for (int actorIndex = 0; actorIndex < world->actorSystems[i]->actors.size(); actorIndex++)
            {
                QString actorName = QString::fromStdString(world->actorSystems[i]->actors[actorIndex]->name);

                QTreeWidgetItem* child = new QTreeWidgetItem();
                child->setText(0, actorName);
                //NOTE: make sure Qt is cleaning up children with TreeView. Don't know if it's a leak
                worldTreeItems[i]->addChild(child);
            }
        }

        worldTreeList->insertTopLevelItems(0, worldTreeItems);
    }

    void ClickOnActor(QTreeWidgetItem* listItem)
    {
        QString string = listItem->text(0);
        Actor* clickedActor = GetWorld()->FindActorByString(string.toStdString());
        if (clickedActor)
        {
            editorCamera.ZoomTo(clickedActor);
            gWorldEditor.pickedActor = clickedActor;
            gEditorSystem->DisplayActorSystemProperties(clickedActor);
        }
    }

    void ClickOnActorSystem(QListWidgetItem* listItem)
    {
        QString actorSystemName = listItem->text();
        ActorSystem* actorSystem = ActorSystemFactory::GetActorSystem(actorSystemName.toStdString().c_str());
        ActorSystemFactory::SetCurrentActiveActorSystem(actorSystem);
    }

    void SearchWorldList()
    {
        QString searchText = "*" + worldSearch->text() + "*"; //Wildcards include items with text at any point

        for (int i = 0; i < worldStringList.size(); i++)
        {
            if (worldStringList.indexOf(QRegExp(searchText, Qt::CaseInsensitive, QRegExp::Wildcard)) == 0)
            {
                actorSystemList->addItem(worldStringList.at(i));
            }
        }
    }

    void CreateNewActorSystem()
    {
        QDialog actorSystemPopup;
        actorSystemPopup.setWindowTitle("Add New Actor System");

        QGridLayout grid;

        QLineEdit name;

        //TODO: would be nice to have custom dropdowns that list all current models/shaders in the system,
        //but who's going to do that? That's a lot of work

        grid.addWidget(new QLabel("Name:"), 0, 0);
        grid.addWidget(&name, 0, 1);

        QDir shaderPath("Shaders/");
        QStringList shaderFiles = shaderPath.entryList(QDir::Files);
        QComboBox shadersCombo;
        shadersCombo.addItems(shaderFiles);

        grid.addWidget(new QLabel("Shader:"), 1, 0);
        grid.addWidget(&shadersCombo, 1, 1);

        QDir modelPath("Models/");
        QStringList modelFiles = modelPath.entryList(QDir::Files);
        QComboBox modelsCombo;
        modelsCombo.addItems(modelFiles);

        grid.addWidget(new QLabel("Model:"), 2, 0);
        grid.addWidget(&modelsCombo, 2, 1);

        QDir texturePath("Textures/");
        QStringList textureFiles = texturePath.entryList(QDir::Files);
        QComboBox texturesCombo;
        texturesCombo.addItems(textureFiles);

        grid.addWidget(new QLabel("Texture:"), 3, 0);
        grid.addWidget(&texturesCombo, 3, 1);

        QPushButton addButton("Add");
        connect(&addButton, &QPushButton::clicked, &actorSystemPopup, &QDialog::accept);
        grid.addWidget(&addButton, 4, 1);

        actorSystemPopup.setLayout(&grid);

        int ret = actorSystemPopup.exec();
        if (ret)
        {
            auto newActorSystem = new ActorSystem();
            newActorSystem->name = name.text().toStdString();
            newActorSystem->shaderName = shadersCombo.currentText().toStdString();
            newActorSystem->modelName = modelsCombo.currentText().toStdString();
            newActorSystem->textureName = texturesCombo.currentText().toStdString();

            GetWorld()->AddActorSystem(newActorSystem);

            PopulateActorSystemList();
            PopulateWorldList();

            AddActorSystemTemplate(newActorSystem);
        }
    }

    void AddExistingActorSystem()
    {
        std::vector<ActorSystem*> actorSystems;
        ActorSystemFactory::GetAllActorSystems(actorSystems);

        QComboBox* systemNames = new QComboBox();
        for (auto& system : actorSystems)
        {
            systemNames->addItem(system->name.c_str());
        }

        QDialog* addSystemDialog = new QDialog();
        addSystemDialog->setWindowTitle("Add Actor System");

        QGridLayout* grid = new QGridLayout();
        grid->addWidget(systemNames, 0, 0);

        QPushButton* addButton = new QPushButton("Add");
        connect(addButton, &QPushButton::clicked, addSystemDialog, &QDialog::accept);
        grid->addWidget(addButton, 1, 0);

        addSystemDialog->setLayout(grid);

        int ret = addSystemDialog->exec();
        if (ret)
        {
            auto actorSystem = ActorSystemFactory::GetActorSystem(systemNames->currentText().toStdString());
            assert(actorSystem && "Actor System not present in ActorSystemFactory maps");

            GetWorld()->AddActorSystem(actorSystem);
        }
    }

    //Adds a template to text file (Think blueprints/prefabs)
    void AddActorSystemTemplate(ActorSystem* actorSystem)
    {
        std::string path = "ActorSystemTemplates/" + actorSystem->name + ".ast"; //.ast = ActorSystemTemplate

        std::filebuf fb;
        fb.open(path, std::ios_base::out);
        std::ostream os(&fb);

        actorSystem->SerialiseAsTemplate(os);
    }

};
