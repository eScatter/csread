#ifndef __AX_LINSPACE_H_
#define __AX_LINSPACE_H_

/*
 * Axis representation as linearly spaced array.
 * Similar convention to numpy linspace:
 * linspace[0] = low, linspace[N-1] = high.
 * 
 * Note: due to round-off errors, linspace[N-1]
 * is not always exactly equal to high.
 */

#include <algorithm>

template<typename datatype>
class ax_linspace
{
public:
	using value_type = datatype;

	// Initialise to invalid state.
	ax_linspace()
		: _low(0), _step(0), _N(0)
	{}

	ax_linspace(value_type low, value_type high, size_t N)
		: _low(low), _step((high - low) / (N - 1)), _N(N)
	{}

	datatype operator[](size_t pos) const
	{
		return _low + _step*pos;
	}

	size_t size() const
	{
		return _N;
	}

	// Find the position of x in this linspace.
	// Return fractional index, which is potentially out of range
	value_type find(value_type x) const
	{
		return (x - _low) / _step;
	}

private:
	value_type _low;
	value_type _step;
	size_t _N;
};

#endif
