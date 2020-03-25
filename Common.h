#pragma once
#include <fstream>
#include <string_view>
#include <vector>

inline std::vector<char> ReadFromFile(std::string_view fileName, bool isBinary = true)
{
	auto fs = std::ifstream();
	auto data = std::vector<char>();

	std::ios_base::openmode open_mode = std::ios_base::in | std::ios_base::ate;
	if (isBinary) open_mode |= std::ios_base::binary;

	fs.open(fileName.data(), open_mode);

	if (fs.good())
	{
		auto size = fs.tellg();
		data.resize(size);
		fs.seekg(0, std::ios_base::beg);
		fs.read(data.data(), size);
	}

	fs.close();
	return data;
}