module;
import <iostream>;
import <string>;
export module FileSystem;

export class FileSystem
{
public:

	void Tick()
	{
		//actorsystem save/load input
		if (gInputSystem.GetAsyncKey(Keys::Ctrl))
		{
			if (gInputSystem.GetKeyUpState(Keys::S))
			{
				gFileSystem.WriteAllActorSystems(GetWorld(), "test");
			}
		}
	}

	void LoadWorld(const std::string& levelName)
	{
		World* world = GetWorld();
		world->CleaupAllActors();

		std::string path = levelName;

		Serialiser s(path, std::ios_base::in);
		std::istream is(&s.fb);

		while (!is.eof())
		{
			ActorSystem* actorSystem = nullptr;

			std::string actorSystemName;
			is >> actorSystemName;

			if (actorSystemName == "")
			{
				break;
			}

			size_t numActorsToSpawn = 0;
			is >> numActorsToSpawn;

			auto asIt = ActorSystemFactory::nameToSystemMap->find(actorSystemName);
			if (asIt == ActorSystemFactory::nameToSystemMap->end())
			{
				actorSystem = new ActorSystem();
				actorSystem->name.assign(actorSystemName);

				for (int i = 0; i < numActorsToSpawn; i++)
				{
					actorSystem->AddActor<Actor>(Transform());
				}

				actorSystem->Deserialise(is);
				actorSystem->SpawnActors(0);
			}
			else
			{
				actorSystem = asIt->second;
				actorSystem->Cleanup();

				actorSystem->SpawnActors(numActorsToSpawn);
				actorSystem->Deserialise(is);
			}

			world->AddActorSystem(actorSystem);
		}

		//Deselect any existing actors, because TransformGizmo will stay at previous positions.
		gWorldEditor.pickedActor = nullptr;

		gEditorSystem->PopulateWorldList();

		gDebugMenu.AddNotification(L"Level loaded.");
	}

	void WriteAllActorSystems(World* world, const std::string& levelName)
	{
		std::string file = "LevelSaves/" + levelName;
		file += ".sav"; //.sav files are still .txt files

		Serialiser s(file, std::ios_base::out);
		std::ostream os(&s.fb);

		for (ActorSystem* actorSystem : world->actorSystems)
		{
			actorSystem->Serialise(os);
		}

		DebugPrint("All actor systems saved.\n");
		gDebugMenu.AddNotification(L"All actor systems saved");
	}


	FILE* file;
};

export FileSystem gFileSystem;
