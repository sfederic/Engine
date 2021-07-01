#pragma once

#include <unordered_map>
#include <typeindex>
#include "Actor.h"

//This is where every ActorSystem is registered at startup in a hashtable.
//For level loading, ActorSystemFactory is used to call Init()s and whatever else per actor system.
class ActorSystemFactory
{
public:
	template <class ActorSystemType>
	static void Register(ActorSystem* actorSystem)
	{
		//The maps are pointers because of static initialisation order, meaning they can be null at any point.
		if (IDToSystemMap == nullptr)
		{
			IDToSystemMap = new std::unordered_map<size_t, ActorSystem*>;
		}
		if (systemToIDMap == nullptr)
		{
			systemToIDMap = new std::unordered_map<ActorSystem*, size_t>;
		}
		if (nameToSystemMap == nullptr)
		{
			nameToSystemMap = new std::unordered_map<std::string, ActorSystem*>;
		}
		if (actorTypeToSystemMap == nullptr)
		{
			actorTypeToSystemMap = new std::unordered_map<std::type_index, ActorSystem*>;
		}

		size_t id = typeid(ActorSystemType).hash_code();
		IDToSystemMap->insert(std::pair(id, actorSystem));
		systemToIDMap->insert(std::pair(actorSystem, id));
		nameToSystemMap->insert(std::pair(actorSystem->name, actorSystem));
	}

	static size_t GetActorSystemID(ActorSystem* actorSystem);
	static ActorSystem* GetActorSystem(size_t id);
	static ActorSystem* GetActorSystem(const std::string& name);
	static ActorSystem* GetActorSystem(const std::type_index& actorType);
	static void GetAllActorSystems(std::vector<ActorSystem*>& actorSystems);
	static void SetCurrentActiveActorSystem(ActorSystem* actorSystem);
	static ActorSystem* GetCurrentActiveActorSystem();

	static std::unordered_map<size_t, ActorSystem*> *IDToSystemMap;
	static std::unordered_map<ActorSystem*, size_t> *systemToIDMap;
	static std::unordered_map<std::string, ActorSystem*>* nameToSystemMap;
	static std::unordered_map<std::type_index, ActorSystem*>* actorTypeToSystemMap;
};
