#pragma once
#include <string>
#include <vector>

namespace Asset
{
	struct AssetFile
	{
		char type[8];
		int version;
		std::string json;
		std::vector<char> binaryBlob;
	};

}