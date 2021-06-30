module;
import <string>;
import <fstream>;
export module Serialise;

export struct Serialiser
{
	Serialiser(const std::string& file, std::ios_base::openmode mode)
	{
		if (fb.is_open())
		{
			fb.close();
		}

		fb.open(file.c_str(), mode);
	}

	void Serialise(const Properties& props, std::ostream& os)
	{
		for (auto& prop : props.dataMap)
		{
			std::type_index type = props.typeMap.find(prop.first)->second.value();

			if (type == typeid(float))
			{
				os << prop.first << "\n" << *(float*)prop.second << "\n";
			}
			else if (type == typeid(XMFLOAT3))
			{
				auto* value = (XMFLOAT3*)prop.second;
				os << prop.first << "\n" << value->x << " " << value->y << " " << value->z << "\n";
			}
			else if (type == typeid(XMFLOAT4))
			{
				auto* value = (XMFLOAT4*)prop.second;
				os << prop.first << "\n" << value->x << " " << value->y << " " << value->z << " " << value->w << "\n";
			}
			else if (type == typeid(bool))
			{
				os << prop.first << "\n" << *(bool*)prop.second << "\n";
			}
			else if (type == typeid(std::string))
			{
				auto str = (std::string*)prop.second;
				os << prop.first << "\n" << str->c_str() << "\n";
			}
		}

		os << "next\n"; //"next" moves the forloop onto the next 'Object'
	}

	void Deserialise(const Properties& props, std::istream& is)
	{
		char line[512];
		while (!is.eof())
		{
			is.getline(line, 512);

			std::string stringline = line;
			if (stringline.find("next") != stringline.npos) //Move to next Object
			{
				return;
			}

			auto prop = props.dataMap.find(line);
			if (prop == props.dataMap.end())
			{
				continue;
			}

			std::type_index type = props.typeMap.find(line)->second.value();

			if (type == typeid(float))
			{
				is >> *(float*)prop->second;
			}
			else if (type == typeid(XMFLOAT3))
			{
				auto float3 = (XMFLOAT3*)prop->second;
				is >> float3->x;
				is >> float3->y;
				is >> float3->z;
			}
			else if (type == typeid(XMFLOAT4))
			{
				auto float4 = (XMFLOAT4*)prop->second;
				is >> float4->x;
				is >> float4->y;
				is >> float4->z;
				is >> float4->w;
			}
			else if (type == typeid(bool))
			{
				is >> *(bool*)prop->second;
			}
			else if (type == typeid(std::string))
			{
				char actorString[512];
				is.getline(actorString, 512);
				auto str = (std::string*)prop->second;
				str->assign(actorString);
			}
		}
	}

	std::filebuf fb;
};
