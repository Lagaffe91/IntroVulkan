#include <fstream>

#include "Utils.h"

std::vector<char> ParseShaderFile(const char* p_fileName)
{
	std::ifstream file = std::ifstream(p_fileName, std::ios::ate | std::ios::binary);
	
	if (!file.is_open())
		return std::vector<char>(); //Empty one
	
	size_t fileSize = (size_t)file.tellg();

	std::vector<char> parsedFile = std::vector<char>(fileSize);

	file.seekg(0);
	file.read(parsedFile.data(), fileSize);

	file.close();

	return parsedFile;
}
