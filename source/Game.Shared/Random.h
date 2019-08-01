#pragma once
#include <Random>
namespace DirectXGame
{
	class Random
	{
	public:
		int range_from = 0;
		int range_to = 10;

		float getRangedRandom(float min, float max);
		int getRangedRandom(int min, int max);
	};
}