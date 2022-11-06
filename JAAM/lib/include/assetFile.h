#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace Asset
{
	struct AssetFile
	{
		char type[4];
		uint32_t version;
		std::string json;
		std::vector<char> binaryBlob;
	};

	enum class CompressionMode : uint32_t 
	{
		None,
		LZ4
	};

	bool SaveBinaryFile(const char* path, const AssetFile& file);
	bool LoadBinaryFile(const char* path, AssetFile& outputFile);

	CompressionMode ParseCompression(const char* f);
}