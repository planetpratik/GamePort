#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace DX
{
	class Utility
	{
	public:
		static std::vector<std::uint8_t> LoadBinaryFile(const std::wstring& filename);

		Utility() = delete;
		Utility(const Utility&) = delete;
		Utility& operator=(const Utility&) = delete;
		Utility(Utility&&) = delete;
		Utility& operator=(Utility&&) = delete;
		~Utility() = default;;
	};
}