#ifndef __ARRAY1D_AX_H_
#define __ARRAY1D_AX_H_

/*
 * Defines a 1D array with associated axis data.
 */

#include <vector>

template<typename datatype, typename ax>
class array1D_ax
{
public:
	using x_type = typename ax::value_type;
	using value_type = datatype;

// Constructors & assignment
	// Default-initialise data
	inline array1D_ax(ax x_axis);
	// Copy data
	inline array1D_ax(ax x_axis, std::vector<value_type> values);
	// Initialise to invalid state.
	inline array1D_ax() = default;

	inline ~array1D_ax();
	inline array1D_ax(array1D_ax const &);
	inline array1D_ax& operator=(array1D_ax const &);
	inline array1D_ax(array1D_ax &&);
	inline array1D_ax& operator=(array1D_ax &&);

// Element access
	// Direct access to an element, unchecked bounds
	inline value_type& operator()(size_t pos);
	inline value_type const & operator()(size_t pos) const;

	inline x_type get_x(size_t pos) const;

	// Find the index corresponding to x.
	// This is the "true index", i.e. potentially fractional and out-of-range.
	inline x_type find_index(x_type x) const;

	// Find a value using linear interpolation, linearly extrapolating when out of range value is requested.
	inline value_type at_linear(x_type x) const;
	// Same, for log-log interpolation
	inline value_type at_loglog(x_type x) const;
	// Same, but rounding to the largest stored element below x. If x is below the range, round up.
	inline value_type at_rounddown(x_type x) const;

// Capacity
	inline size_t size() const;
	inline std::pair<value_type, value_type> get_xrange() const;

private:
	datatype* _data = nullptr;
	ax _x_axis;
};

#include "array1D_ax.inl"

#endif
