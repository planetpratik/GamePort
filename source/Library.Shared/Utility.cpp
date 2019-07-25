#include "pch.h"
#include "Utility.h"

using namespace std;

namespace DX
{
	vector<uint8_t> Utility::LoadBinaryFile(const wstring& filename)
	{
		ifstream file(filename.c_str(), ios::binary);
		if (!file.good())
		{
			throw exception("Could not open file.");
		}

		file.seekg(0, ios::end);
		uint32_t size = (uint32_t)file.tellg();

		vector<uint8_t> data;
		if (size > 0)
		{
			data.resize(size);
			file.seekg(0, ios::beg);
			file.read(reinterpret_cast<char*>(data.data()), size);
		}

		file.close();

		return data;
	}
}