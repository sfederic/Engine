#pragma once

#include <string>
#include <fstream>
#include "Properties.h"

struct Serialiser
{
	Serialiser(const std::string& file, std::ios_base::openmode mode);

	//TODO: I don't like these being static here.
	static void Serialise(const Properties& props, std::ostream& os);
	static void Deserialise(const Properties& props, std::istream& is);

	std::filebuf fb;
};
