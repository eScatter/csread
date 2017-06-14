#include <algorithm>
#include "../clamp.h"
#include "array2D_ax.h"

template<typename datatype, typename ax_x, typename ax_y>
array2D_ax<datatype, ax_x, ax_y>::array2D_ax(ax_x x_axis, ax_y y_axis) :
	_x_axis(x_axis), _y_axis(y_axis)
{
	_data = new datatype[x_axis.size()*y_axis.size()]();
}
template<typename datatype, typename ax_x, typename ax_y>
array2D_ax<datatype, ax_x, ax_y>::array2D_ax(ax_x x_axis, ax_y y_axis, std::vector<value_type> values) :
	_x_axis(x_axis), _y_axis(y_axis)
{
	if (x_axis.size()*y_axis.size() != values.size())
		throw std::runtime_error("Unmatched dimensions between axes and values.");
	_data = new datatype[size()];
	std::copy(values.begin(), values.end(), _data);
}
template<typename datatype, typename ax_x, typename ax_y>
array2D_ax<datatype, ax_x, ax_y>::~array2D_ax()
{
	delete[] _data;
}
template<typename datatype, typename ax_x, typename ax_y>
array2D_ax<datatype, ax_x, ax_y>::array2D_ax(array2D_ax const & rhs) :
	_x_axis(rhs._x_axis), _y_axis(rhs._y_axis)
{
	const auto sz = size();
	_data = new datatype[sz];
	std::copy(rhs._data, rhs._data + sz, _data);
}
template<typename datatype, typename ax_x, typename ax_y>
auto array2D_ax<datatype, ax_x, ax_y>::operator=(array2D_ax const & rhs) -> array2D_ax&
{
	if (this != &rhs)
	{
		_x_axis = rhs._x_axis;
		_y_axis = rhs._y_axis;
		delete[] _data;

		const auto sz = size();
		_data = new datatype[sz];
		std::copy(rhs._data, rhs._data + sz, _data);
	}
	return *this;
}
template<typename datatype, typename ax_x, typename ax_y>
array2D_ax<datatype, ax_x, ax_y>::array2D_ax(array2D_ax && rhs) :
	_x_axis(std::move(rhs._x_axis)), _y_axis(std::move(rhs._y_axis)), _data(rhs._data)
{
	rhs._data = nullptr;
}
template<typename datatype, typename ax_x, typename ax_y>
auto array2D_ax<datatype, ax_x, ax_y>::operator=(array2D_ax && rhs) -> array2D_ax&
{
	if (this != &rhs)
	{
		_x_axis = std::move(rhs._x_axis);
		_y_axis = std::move(rhs._y_axis);

		delete[] _data;
		_data = rhs._data;
		rhs._data = nullptr;
	}
	return *this;
}

template<typename datatype, typename ax_x, typename ax_y>
auto array2D_ax<datatype, ax_x, ax_y>::operator()(size_t pos_x, size_t pos_y) -> value_type&
{
	return _data[pos_x*height() + pos_y];
}

template<typename datatype, typename ax_x, typename ax_y>
auto array2D_ax<datatype, ax_x, ax_y>::operator()(size_t pos_x, size_t pos_y) const -> value_type const &
{
	return _data[pos_x*height() + pos_y];
}

template<typename datatype, typename ax_x, typename ax_y>
auto array2D_ax<datatype, ax_x, ax_y>::get_x(size_t pos_x) const -> x_type
{
	return _x_axis[pos_x];
}
template<typename datatype, typename ax_x, typename ax_y>
auto array2D_ax<datatype, ax_x, ax_y>::get_y(size_t pos_y) const -> y_type
{
	return _y_axis[pos_y];
}

template<typename datatype, typename ax_x, typename ax_y>
auto array2D_ax<datatype, ax_x, ax_y>::find_x(x_type x) const -> x_type
{
	return _x_axis.find(x);
}

template<typename datatype, typename ax_x, typename ax_y>
auto array2D_ax<datatype, ax_x, ax_y>::find_y(y_type y) const -> y_type
{
	return _y_axis.find(y);
}

template<typename datatype, typename ax_x, typename ax_y>
auto array2D_ax<datatype, ax_x, ax_y>::at_linear(x_type x, y_type y) const -> value_type
{
	const x_type true_x = _x_axis.find(x);
	const y_type true_y = _y_axis.find(y);

	const size_t low_x = static_cast<size_t>(_clamp<value_type>(true_x, 0, _x_axis.size()-2));
	const x_type frac_x = true_x - low_x;
	const size_t low_y = static_cast<size_t>(_clamp<value_type>(true_y, 0, _y_axis.size()-2));
	const y_type frac_y = true_y - low_y;

	const datatype v00 = (*this)(low_x, low_y);
	const datatype v10 = (*this)(low_x + 1, low_y);
	const datatype v01 = (*this)(low_x, low_y + 1);
	const datatype v11 = (*this)(low_x + 1, low_y + 1);

	/*
	   FIXME: there is a potential problem if frac_x/y == 0 or 1
	   and values are infinite: 0 * inf == nan.
	   In all other cases, infinities are handled correctly.
	 */

	return (1 - frac_x)*(1 - frac_y)*v00
		+ frac_x*(1 - frac_y)*v10
		+ (1 - frac_x)*frac_y*v01
		+ frac_x*frac_y*v11;
}

template<typename datatype, typename ax_x, typename ax_y>
auto array2D_ax<datatype, ax_x, ax_y>::at_rounddown(x_type x, y_type y) const -> value_type
{
	const x_type true_x = _x_axis.find(x);
	const y_type true_y = _y_axis.find(y);

	size_t rounded_x = static_cast<size_t>(_clamp<value_type>(true_x, 0, _x_axis.size() - 1));
	size_t rounded_y = static_cast<size_t>(_clamp<value_type>(true_y, 0, _y_axis.size() - 1));

	return (*this)(rounded_x, rounded_y);
}

template<typename datatype, typename ax_x, typename ax_y>
size_t array2D_ax<datatype, ax_x, ax_y>::width() const
{
	return _x_axis.size();
}

template<typename datatype, typename ax_x, typename ax_y>
size_t array2D_ax<datatype, ax_x, ax_y>::height() const
{
	return _y_axis.size();
}

template<typename datatype, typename ax_x, typename ax_y>
size_t array2D_ax<datatype, ax_x, ax_y>::size() const
{
	return width() * height();
}

template<typename datatype, typename ax_x, typename ax_y>
auto array2D_ax<datatype, ax_x, ax_y>::get_xrange() const -> std::pair<value_type, value_type>
{
	return{ _x_axis[0], _x_axis[width() - 1] };
}

template<typename datatype, typename ax_x, typename ax_y>
auto array2D_ax<datatype, ax_x, ax_y>::get_yrange() const -> std::pair<value_type, value_type>
{
	return{ _y_axis[0], _y_axis[height() - 1] };
}
