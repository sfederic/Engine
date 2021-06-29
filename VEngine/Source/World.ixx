module;
import <vector>;
import <unordered_map>;
import <string>;
export module World;

export World* GetWorld()
{
	return &gCurrentWorld;
}

export class World
{
public:

	template <class ActorType>
	Actor* SpawnActor(Transform transform)
	{
		ActorSystem* actorSystem = ActorSystemFactory::GetActorSystem(typeid(ActorType));
		if (actorSystem)
		{
			actorSystem->SpawnActor(transform);
		}
	}

	template <class ActorType>
	void SpawnActors(std::vector<Transform>& transforms, uint64_t numActorsToSpawn)
	{
		ActorSystem* actorSystem = ActorSystemFactory::GetActorSystem(typeid(ActorType));
		if (actorSystem)
		{
			actorSystem->SpawnActors(transforms, numActorsToSpawn);
		}
	}


	void Load(std::string levelName)
	{
		FILE* file;
		fopen_s(&file, levelName.c_str(), "rb");
		assert(file);

		fclose(file);
	}

	void TickAllActorSystems(float deltaTime)
	{
		PROFILE_START

			//Skip ticks if game is paused
			if (gCoreSystem.bGamePaused)
			{
				//return;
			}

		//Skip ticks if game in-editor isn't running
		if (!gCoreSystem.bGamePlayOn)
		{
			//return;
		}

		for (ActorSystem* actorSystem : actorSystems)
		{
			actorSystem->Tick(deltaTime);

			for (Actor* actor : actorSystem->actors)
			{
				actor->Tick(deltaTime);

				if (actor->parent == nullptr)
				{
					actor->UpdateTransform(XMMatrixIdentity());
				}
			}
		}

		PROFILE_END
	}

	void StartAllActorSystems()
	{
		for (auto& as : actorSystems)
		{
			as->Start();

			for (auto& actor : as->actors)
			{
				actor->Start();
			}
		}
	}

	void CleaupAllActors()
	{
		actorSystems.clear();
		actorSystemsMap.clear();
	}

	void AddActorSystem(ActorSystem* actorSystem)
	{
		auto actorSystemIt = actorSystemsMap.find(actorSystem->name);
		if (actorSystemIt == actorSystemsMap.end())
		{
			actorSystem->SpawnActors(1); //Spawn one dummy actor so that buffers set up properly
			actorSystems.push_back(actorSystem);
			actorSystemsMap.insert(std::pair(actorSystem->name, actorSystem));

			auto asIt = ActorSystemFactory::nameToSystemMap->find(actorSystem->name);
			if (asIt == ActorSystemFactory::nameToSystemMap->end())
			{
				//This is to spawn template actorsystems that won't exist in the factory
				ActorSystemFactory::Register<ActorSystem>(actorSystem);
			}
		}

		gEditorSystem->PopulateActorSystemList();
		gEditorSystem->PopulateWorldList();
	}

	Actor* FindActorByString(const std::string& name)
	{
		Actor* foundActor = nullptr;

		for (int i = 0; i < actorSystems.size(); i++)
		{
			for (int j = 0; j < actorSystems[i]->actors.size(); j++)
			{
				if (actorSystems[i]->actors[j]->name == name)
				{
					foundActor = actorSystems[i]->GetActor(j);
					return foundActor;
				}
			}
		}

		return foundActor;
	}

	ActorSystem* GetActorSystem(unsigned int id)
	{
		if (id < actorSystems.size())
		{
			return actorSystems[id];
		}

		return nullptr;
	}

	Actor* GetActor(unsigned int systemId, unsigned int actorId)
	{
		if (systemId < actorSystems.size())
		{
			if (actorId < actorSystems[systemId]->actors.size())
			{
				return actorSystems[systemId]->actors[actorId];
			}
		}

		return nullptr;
	}

	uint64_t GetNumOfActorsInWorld()
	{
		uint64_t numOfActorsInWorld = 0;

		for (auto& actorSystem : actorSystems)
		{
			numOfActorsInWorld += actorSystem->actors.size();
		}

		return numOfActorsInWorld;
	}


	std::vector<ActorSystem*> actorSystems;
	std::unordered_map<std::string, ActorSystem*> actorSystemsMap;

	char name[64];
};

export World gCurrentWorld;
