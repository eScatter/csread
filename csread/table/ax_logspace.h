#ifndef __AX_LOGSPACE_H_
#define __AX_LOGSPACE_H_

/*
 * Axis representation as log spaced array.
 * logspace[0] = low, logspace[N-1] = high.
 * 
 * Note: due to round-off errors, logspace[N-1]
 * is not always exactly equal to high.
 */

#include <algorithm>

template<typename datatype>
class ax_logspace
{
public:
	using value_type = datatype;

	// Initialise to invalid state.
	ax_logspace()
		: _llow(0), _lstep(0), _N(0)
	{}

	ax_logspace(value_type low, value_type high, size_t N)
		: _llow(std::log(low)), _lstep(std::log(high/low) / (N - 1)), _N(N)
	{}

	datatype operator[](size_t pos) const
	{
		return std::exp(_llow + _lstep*pos);
	}

	size_t size() const
	{
		return _N;
	}

	// Find the position of x in this logspace.
	// Return fractional index, which is potentially out of range
	value_type find(value_type x) const
	{
		return (std::log(x) - _llow) / _lstep;
	}

private:
	value_type _llow;
	value_type _lstep;
	size_t _N;
};

#endif
