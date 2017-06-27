#ifndef __AX_LIST_H_
#define __AX_LIST_H_

/*
 * Axis representation as array of consecutive values.
 */

#include <vector>
#include <algorithm>
#include "../clamp.h"

template<typename datatype>
class ax_list : public std::vector<datatype>
{
public:
	using base_type = std::vector<datatype>;
	using value_type = datatype;

	using base_type::size;

	ax_list() = default;
	ax_list(std::vector<datatype> const & data) :
		base_type(data)
	{}
	ax_list(size_t size) :
		base_type(size)
	{}

	// Find the position of x in this logspace.
	// Return [in range, fractional index]
	value_type find(datatype x) const
	{
		// Estimate true index, even if out of range.
		const auto high_iterator = std::lower_bound(base_type::begin(), base_type::end(), x);
		const size_t high_index = _clamp<size_t>(std::distance(base_type::begin(), high_iterator), 1, size() - 1);
		const value_type high_value = (*this)[high_index];
		const value_type low_value = (*this)[high_index - 1];
		const value_type true_index = high_index + (x - high_value) / (high_value - low_value);

		return true_index;
	}
};

#endif
