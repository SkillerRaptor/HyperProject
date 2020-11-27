#pragma once

#include <random>

class Random
{
private:
	static std::mt19937 s_RandomEngine;

public:
	static void Init()
	{
		s_RandomEngine.seed(std::random_device()());
	}

	static int Int()
	{
		std::uniform_int_distribution<std::mt19937::result_type> distribution;
		return static_cast<int>(distribution(s_RandomEngine));
	}

	static int Int(int start, int end)
	{
		std::uniform_int_distribution<std::mt19937::result_type> distribution(start, end);
		return static_cast<int>(distribution(s_RandomEngine));
	}

	static float Float()
	{
		std::uniform_int_distribution<std::mt19937::result_type> distribution;
		return static_cast<float>(distribution(s_RandomEngine)) / static_cast<float>(std::numeric_limits<uint32_t>::max());
	}

	static float Float(int start, int end)
	{
		return static_cast<float>(start + (Float() * static_cast<float>(end - start)));
	}

	static double Double()
	{
		std::uniform_int_distribution<std::mt19937::result_type> distribution;
		return static_cast<double>(distribution(s_RandomEngine)) / static_cast<double>(std::numeric_limits<uint32_t>::max());
	}

	static double Double(int start, int end)
	{
		return static_cast<double>(start + (Double() * static_cast<double>(end - start)));
	}

	static bool Bool()
	{
		return (Float() >= 0.5 ? true : false);
	}
};