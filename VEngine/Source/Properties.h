#pragma once

#include <string>
#include <unordered_map>
#include <optional>
#include <typeindex>
#include "GlobalDefines.h"

//Lot of good references and ideas on serialisation. Heaps of ideas on reflection, custom compile stages, header parsing.
//REF:https://google.github.io/flatbuffers/flatbuffers_guide_tutorial.html
//REF:http://donw.io/post/reflection-cpp-2/
//REF:https://github.com/HeliumProject/Core/blob/master/Documentation/Module-3-Reflect.md

//Container for storing variable data and variable types against a name on registration.
//Used with Actors/Materials during serialisation and creating property widgets.
//Properties are created in GetProps() function blocks so that the maps can be cascaded down
//actor inheritence trees, meaning you have to define Properties by hand in that same function, then
//call it when needed. Maybe look into some way to cache it. Performance wasn't a huge issue in testing.
struct Properties
{
#ifndef GAME_RELEASE

	//PROPERTIES FOR TEXT
	template <typename T>
	void Add(const std::string& name, T* data)
	{
		dataMap[name] = data;
		typeMap[name] = typeid(T);
	}

	void* GetData(const std::string& dataName)
	{
		auto dataMapIt = dataMap.find(dataName);
		if (dataMapIt != dataMap.end())
		{
			return dataMapIt->second;
		}

		return nullptr; //Want to be able to fail finding keys when serialising.
	}

	std::type_index GetType(const std::string& typeName)
	{
		auto typeMapIt = typeMap.find(typeName);
		return typeMapIt->second.value(); //Type will usually be grabbed after data, failing here acts as a check.
	}

	std::unordered_map<std::string, void*> dataMap;
	std::unordered_map<std::string, std::optional<std::type_index>> typeMap;

#else //GAME_RELEASE

	//PROPERTIES FOR BINARY
	template <typename T>
	void Add(const std::string& name, T* data)
	{
		size_t hash = std::hash < std::string>{}(name);
		dataMap[hash] = data;
		typeMap[hash] = typeid(T);
	}

	void* GetData(size_t hash)
	{
		auto dataMapIt = dataMap.find(hash);
		if (dataMapIt != dataMap.end())
		{
			return dataMapIt->second;
		}

		return nullptr;
	}

	std::type_index GetType(size_t hash)
	{
		auto typeMapIt = typeMap.find(hash);
		return typeMapIt->second.value();
	}

	std::unordered_map<size_t, void*> dataMap;
	std::unordered_map<size_t, std::optional<std::type_index>> typeMap;

#endif
};
