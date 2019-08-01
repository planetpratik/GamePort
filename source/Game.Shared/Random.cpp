#pragma once
#include "pch.h"
#include "Random.h"
#include <stdlib.h>

namespace DirectXGame
{
	float Random::getRangedRandom(float min, float max)
	{
		std::random_device randomDevice;
		std::mt19937 generator(randomDevice());
		std::uniform_real_distribution<> distribution(min, max);

		return static_cast<float>(distribution(generator));
	}
	int Random::getRangedRandom(int min, int max)
	{
		std::random_device randomDevice;
		std::mt19937 generator(randomDevice());
		std::uniform_int_distribution<> distribution(min, max);

		return distribution(generator);
	}
}