#include "FileSystem.h"
#include "Actor.h"
#include "World.h"
#include "DebugMenu.h"
#include "Input.h"
#include "Debug.h"
#include "UISystem.h"

FileSystem g_FileSystem;

void FileSystem::Tick()
{
	//world load file handling.
	if (gUISystem.bEditUIActive)
	{
		if (inputSystem.GetKeyUpState(VK_F4))
		{
			g_FileSystem.WriteAllActorSystems(GetWorld(), "LevelSaves/test.sav");
		}

		if (inputSystem.GetKeyUpState(VK_F5))
		{
			g_FileSystem.ReadAllActorSystems(GetWorld(), "LevelSaves/test.sav");
		}
	}
}

void FileSystem::WriteAllActorSystems(World* world, const char* filename)
{
	//TODO: make filename work with current World name
	fopen_s(&file, filename, "w");
	assert(file);

	int numActorSystems = world->actorSystems.size();
	fwrite(&numActorSystems, sizeof(int), 1, file);

	for (int systemIndex = 0; systemIndex < world->actorSystems.size(); systemIndex++)
	{
		fwrite(&world->actorSystems[systemIndex]->id, sizeof(int), 1, file);

		int numOfActors = world->actorSystems[systemIndex]->actors.size();
		fwrite(&numOfActors, sizeof(int), 1, file);

		for (int actorIndex = 0; actorIndex < world->actorSystems[systemIndex]->actors.size(); actorIndex++)
		{
			//fwrite without the for loop(SOA) was about 0.01 ms faster with around 60,000 actors. Surprising
			fwrite(&world->actorSystems[systemIndex]->actors[actorIndex].transform, sizeof(XMMATRIX), 1, file);
		}
	}

	DebugPrint("All actor systems saved.\n");
	debugMenu.notifications.push_back(DebugNotification(L"All actor systems saved"));

	fclose(file);
}

void FileSystem::ReadAllActorSystems(World* world, const char* filename)
{
	fopen_s(&file, filename, "r");
	assert(file);

	World newWorld = {};
	int numActorSystems = 0;
	fread(&numActorSystems, sizeof(int), 1, file);

	for (int systemIndex = 0; systemIndex < numActorSystems; systemIndex++)
	{
		//TODO: fix this
		newWorld.actorSystems.push_back(&ActorSystem());

		fread(&newWorld.actorSystems[systemIndex]->id, sizeof(int), 1, file);

		int numActorsToSpawn = 0;
		fread(&numActorsToSpawn, sizeof(int), 1, file);

		switch (newWorld.actorSystems[systemIndex]->id)
		{
		case EActorSystemID::Actor:
			newWorld.actorSystems[systemIndex]->CreateActors(&gRenderSystem, numActorsToSpawn);
		}

		for (int actorIndex = 0; actorIndex < world->actorSystems[systemIndex]->actors.size(); actorIndex++)
		{
			fread(&newWorld.actorSystems[systemIndex]->actors[actorIndex].transform, sizeof(XMMATRIX), 1, file);
		}

	}

	gCurrentWorld.CleaupAllActors();

	gCurrentWorld = newWorld;

	DebugPrint("All actor systems loaded.\n");
	debugMenu.notifications.push_back(DebugNotification(L"All actor systems loaded."));

	fclose(file);
}