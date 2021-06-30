module;
#include <DirectXMath.h>
import <string>;
import <vector>;
import Properties;
import Serialise;
export module Material;

using namespace DirectX;

export class Material
{
public:
	Properties GetProps()
	{
		Properties props;
		props.Add("Name", &name);
		props.Add("Colour", &colour);
		return props;
	}

	void SaveToFile()
	{
		std::string filename = "Materials/" + name;
		Serialiser s(filename, std::ios_base::out);

		std::ostream os(&s.fb);
		Serialiser::Serialise(GetProps(), os);
	}


	std::string name;
	XMFLOAT4 colour;
};

export class MaterialSystem
{
public:
	Material* CreateMaterialFromFile(const std::string& filename)
	{
		Material newMaterial = {};

		std::filebuf fb;
		fb.open(filename.c_str(), std::ios_base::in);
		std::istream is(&fb);

		materials.push_back(newMaterial);
		return &materials.back();
	}

	std::vector<Material> materials;
};
