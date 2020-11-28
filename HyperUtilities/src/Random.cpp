#include "Random.h"

namespace HyperUtilities
{
	std::mt19937 Random::s_RandomEngine;

	void Random::Init()
	{
		s_RandomEngine.seed(std::random_device()());
	}

	int Random::Int(int start, int end)
	{
		std::uniform_int_distribution<std::mt19937::result_type> distribution(start, end);
		return static_cast<int>(distribution(s_RandomEngine));
	}

	int Random::Int()
	{
		std::uniform_int_distribution<std::mt19937::result_type> distribution;
		return static_cast<int>(distribution(s_RandomEngine));
	}

	float Random::Float(int start, int end)
	{
		return static_cast<float>(start + (Float() * static_cast<float>(end - start)));
	}

	float Random::Float()
	{
		std::uniform_int_distribution<std::mt19937::result_type> distribution;
		return static_cast<float>(distribution(s_RandomEngine)) / static_cast<float>(std::numeric_limits<uint32_t>::max());
	}

	double Random::Double(int start, int end)
	{
		return static_cast<double>(start + (Double() * static_cast<double>(end - start)));
	}

	double Random::Double()
	{
		std::uniform_int_distribution<std::mt19937::result_type> distribution;
		return static_cast<double>(distribution(s_RandomEngine)) / static_cast<double>(std::numeric_limits<uint32_t>::max());
	}

	bool Random::Bool()
	{
		return (Float() >= 0.5 ? true : false);
	}
}