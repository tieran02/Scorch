#include <iostream>
#include <fstream>
#include <filesystem>
#include <regex>

#include "jaam.h"

#include "nlohmann/json.hpp"
#include "lz4.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "assetTexture.h"

namespace fs = std::filesystem;
using namespace Asset;

bool ConvertImage(const fs::path& input, const fs::path& output)
{
	int texWidth, texHeight, texChannels;

	stbi_uc* pixels = stbi_load(input.string().c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

	if (!pixels) {
		std::cout << "Failed to load texture file " << input << std::endl;
		return false;
	}

	int texture_size = texWidth * texHeight * 4;

	TextureInfo texinfo;
	texinfo.textureSize = texture_size;
	texinfo.pixelsize[0] = texWidth;
	texinfo.pixelsize[1] = texHeight;
	texinfo.textureFormat = TextureFormat::RGBA8;
	texinfo.originalFile = input.string();
	AssetFile newImage = PackTexture(&texinfo, pixels);

	stbi_image_free(pixels);

	SaveBinaryFile(output.string().c_str(), newImage);

	return true;
}


int main(int argc, char** argv)
{
	for (int i = 0; i < argc; ++i)
		std::cout << argv[i] << '\n';

	const fs::path path{ argv[1] };
	const fs::path output{ argv[2] };

	std::cout << "loading asset directory at " << path << std::endl;

	for (auto& p : fs::recursive_directory_iterator(path))
	{
		fs::path newpath = p.path().string().replace(0, path.string().length(), output.string()); // replace 'def' -> 'klm'
		fs::path newdir = newpath;

		fs::create_directories(newdir.remove_filename());

		if (p.path().extension() == ".png") {
			std::cout << "found a texture" << p << std::endl;

			newpath.replace_extension(".tx");
			ConvertImage(p.path(), newpath);
		}
		if (p.path().extension() == ".obj") {
			std::cout << "found a mesh" << p << std::endl;

			newpath.replace_extension(".mesh");
			//convert_mesh(p.path(), newpath);
		}
	}
}