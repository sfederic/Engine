module;
import <unordered_map>;
import <typeindex>;
import Actor;
export module ActorSystemFactory;

//This is where every ActorSystem is registered at startup in a hashtable.
//For level loading, ActorSystemFactory is used to call Init()s and whatever else per actor system.
export class ActorSystemFactory
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

	static std::unordered_map<size_t, ActorSystem*>* IDToSystemMap;
	static std::unordered_map<ActorSystem*, size_t>* systemToIDMap;
	static std::unordered_map<std::string, ActorSystem*>* nameToSystemMap;
	static std::unordered_map<std::type_index, ActorSystem*>* actorTypeToSystemMap;

	ActorSystem* activeActorSystemSpawner;

	size_t GetActorSystemID(ActorSystem* actorSystem)
	{
		auto ID = systemToIDMap->find(actorSystem);
		return ID->second;
	}

	ActorSystem* GetActorSystem(size_t id)
	{
		auto actorSystem = IDToSystemMap->find(id);
		return actorSystem->second;
	}

	ActorSystem* GetActorSystem(const std::string& name)
	{
		auto actorSystem = nameToSystemMap->find(name);
		return actorSystem->second;
	}

	ActorSystem* GetActorSystem(const std::type_index& actorType)
	{
		auto actorSystemIt = actorTypeToSystemMap->find(typeid(actorType));
		if (actorSystemIt != actorTypeToSystemMap->end())
		{
			return actorSystemIt->second;
		}

		return nullptr;
	}

	void GetAllActorSystems(std::vector<ActorSystem*>& actorSystems)
	{
		for (auto& as : *IDToSystemMap)
		{
			actorSystems.push_back(as.second);
		}
	}

	void SetCurrentActiveActorSystem(ActorSystem* actorSystem)
	{
		activeActorSystemSpawner = actorSystem;
	}

	ActorSystem* GetCurrentActiveActorSystem()
	{
		return activeActorSystemSpawner;
	}
};
