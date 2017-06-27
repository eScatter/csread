#ifndef __ARRAY2D_AX_H_
#define __ARRAY2D_AX_H_

/*
 * Defines a 2D array with associated axis data.
 */

template<typename datatype, typename ax_x, typename ax_y>
class array2D_ax
{
public:
	using x_type = typename ax_x::value_type;
	using y_type = typename ax_y::value_type;
	using value_type = datatype;

// Constructors
	// Default-initialise data
	inline array2D_ax(ax_x x_axis, ax_y y_axis);
	// Copy data. Values are indexed as [x_index*height() + y_index]
	inline array2D_ax(ax_x x_axis, ax_y y_axis, std::vector<value_type> values);
	// Initialise to invalid state.
	inline array2D_ax() = default;

	inline ~array2D_ax();
	inline array2D_ax(array2D_ax const &);
	inline array2D_ax& operator=(array2D_ax const &);
	inline array2D_ax(array2D_ax &&);
	inline array2D_ax& operator=(array2D_ax &&);

// Element access
	// Direct access to an element, unchecked bounds
	inline value_type& operator()(size_t pos_x, size_t pos_y);
	inline value_type const & operator()(size_t pos_x, size_t pos_y) const;

	inline x_type get_x(size_t pos_x) const;
	inline y_type get_y(size_t pos_y) const;

	// Find the index corresponding to x and y.
	// This is the "true index", i.e. potentially fractional and out-of-range.
	inline x_type find_x(x_type x) const;
	inline y_type find_y(y_type y) const;

	// Find a value using linear interpolation, linearly extrapolating when out of range value is requested.
	inline value_type at_linear(x_type x, y_type y) const;
	// TODO: at_loglog
	// Same, but rounding to the largest stored element below the requested one. If x or y is below the range, round up.
	inline value_type at_rounddown(x_type x, y_type y) const;

// Capacity
	inline size_t width() const;
	inline size_t height() const;
	inline size_t size() const;

	inline std::pair<value_type, value_type> get_xrange() const;
	inline std::pair<value_type, value_type> get_yrange() const;

protected:
	ax_x _x_axis;
	ax_y _y_axis;

private:
	datatype* _data = nullptr;
};

#include "array2D_ax.inl"

#endif
