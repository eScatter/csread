#ifndef __CLAMP_H_
#define __CLAMP_H_

/*
 * Not all compilers support the c++17-style std::clamp yet.
 * So we define our own clamp in the global namespace.
 * Since CUDA also puts a clamp() function in the global
 * namespace, we name it _clamp() for good measure.
 */

template <typename T>
T _clamp(const T& value, const T& lower, const T& upper)
{
	return std::max(lower, std::min(value, upper));
}

#endif
