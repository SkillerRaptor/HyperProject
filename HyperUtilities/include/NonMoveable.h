#pragma once

namespace HyperUtilities
{
	class NonMoveable
	{
	protected:
		NonMoveable() = default;
		~NonMoveable() = default;

		NonMoveable(const NonMoveable&&) = delete;
		NonMoveable& operator=(const NonMoveable&&) = delete;
	};
}