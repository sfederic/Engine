#include "World.h"
#include "Actor.h"
#include "DebugMenu.h"
#include "CoreSystem.h"

World gCurrentWorld;

//Returns the current active world
World* GetWorld()
{
	return &gCurrentWorld;
}

void World::Load(std::string levelName)
{
	FILE* file;
	fopen_s(&file, levelName.c_str(), "rb");
	assert(file);

	fclose(file);
}

void World::TickAllActorSystems(float deltaTime)
{
	//Skip ticks if game is paused
	if (gCoreSystem.bGamePaused)
	{
		return;
	}

	//Skip ticks if game in-editor isn't running
	if (!gCoreSystem.bGamePlayOn)
	{
		return;
	}

	for (int asIndex = 0; asIndex < actorSystems.size(); asIndex++)
	{
		actorSystems[asIndex]->Tick(deltaTime);

		for (int actorIndex = 0; actorIndex < actorSystems[asIndex]->actors.size(); actorIndex++)
		{
			actorSystems[asIndex]->actors[actorIndex]->Tick(deltaTime);
		}
	}
}

void World::CleaupAllActors()
{
	actorSystems.clear();
	actorSystemsMap.clear();
}

void World::AddActorSystem(ActorSystem* actorSystem)
{
	auto actorSystemIt = actorSystemsMap.find(actorSystem->name);
	if (actorSystemIt == actorSystemsMap.end())
	{
		if (!actorSystem->bHasBeenInitialised)
		{
			actorSystem->SpawnActors(0);
		}

		actorSystems.push_back(actorSystem);
		actorSystemsMap.insert(std::pair(actorSystem->name, actorSystem));
	}
	else
	{
		gDebugMenu.AddNotification(L"Actor system could not be added to world");
	}
}

Actor* World::FindActorByString(const std::string& name)
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

ActorSystem* World::GetActorSystem(unsigned int id)
{
	if (id < actorSystems.size())
	{
		return actorSystems[id];
	}

	return nullptr;
}

Actor* World::GetActor(unsigned int systemId, unsigned int actorId)
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
