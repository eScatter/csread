#include <algorithm>
#include <cmath>
#include <tuple>
#include "../clamp.h"
#include "array1D_ax.h"

template<typename datatype, typename ax>
array1D_ax<datatype, ax>::array1D_ax(ax x_axis) :
	_x_axis(x_axis)
{
	_data = new datatype[size()]();
}
template<typename datatype, typename ax>
array1D_ax<datatype, ax>::array1D_ax(ax x_axis, std::vector<value_type> values) :
	_x_axis(x_axis)
{
	if (x_axis.size() != values.size())
		throw std::runtime_error("Unmatched dimensions between axis and values.");
	_data = new datatype[size()];
	std::copy(values.begin(), values.end(), _data);
}
template<typename datatype, typename ax>
array1D_ax<datatype, ax>::~array1D_ax()
{
	delete[] _data;
}
template<typename datatype, typename ax>
array1D_ax<datatype, ax>::array1D_ax(array1D_ax const & rhs) :
	_x_axis(rhs._x_axis)
{
	const auto sz = size();
	_data = new datatype[sz];
	std::copy(rhs._data, rhs._data + sz, _data);
}
template<typename datatype, typename ax>
auto array1D_ax<datatype, ax>::operator=(array1D_ax const & rhs) -> array1D_ax&
{
	if (this != &rhs)
	{
		_x_axis = rhs._x_axis;
		delete[] _data;

		const auto sz = size();
		_data = new datatype[sz];
		std::copy(rhs._data, rhs._data + sz, _data);
	}
	return *this;
}
template<typename datatype, typename ax>
array1D_ax<datatype, ax>::array1D_ax(array1D_ax && rhs) :
	_x_axis(std::move(rhs._x_axis)), _data(rhs._data)
{
	rhs._data = nullptr;
}
template<typename datatype, typename ax>
auto array1D_ax<datatype, ax>::operator=(array1D_ax && rhs) -> array1D_ax&
{
	if (this != &rhs)
	{
		_x_axis = std::move(rhs._x_axis);

		delete[] _data;
		_data = rhs._data;
		rhs._data = nullptr;
	}
	return *this;
}

template<typename datatype, typename ax>
auto array1D_ax<datatype, ax>::operator()(size_t pos) -> value_type&
{
	return _data[pos];
}

template<typename datatype, typename ax>
auto array1D_ax<datatype, ax>::operator()(size_t pos) const -> value_type const &
{
	return _data[pos];
}

template<typename datatype, typename ax>
auto array1D_ax<datatype, ax>::get_x(size_t pos) const -> x_type
{
	return _x_axis[pos];
}

template<typename datatype, typename ax>
auto array1D_ax<datatype, ax>::find_index(x_type x) const -> x_type
{
	return _x_axis.find(x);
}

template<typename datatype, typename ax>
auto array1D_ax<datatype, ax>::at_linear(x_type x) const -> value_type
{
	const x_type true_index = _x_axis.find(x);

	const size_t low_index = static_cast<size_t>(_clamp<value_type>(true_index, 0, _x_axis.size() - 2));
	const x_type frac_index = true_index - low_index;
	const datatype low_value = _data[low_index];
	const datatype high_value = _data[low_index + 1];

	/*
	   FIXME: there is a potential problem if frac_index == 0 and high_value is infinite
	   or frac_index == 1 and low_value is infinite: 0 * inf == nan.
	   In all other cases, infinities are handled correctly.
	 */
	return (1 - frac_index)*low_value + frac_index*high_value;
}

template<typename datatype, typename ax>
auto array1D_ax<datatype, ax>::at_loglog(x_type x) const -> value_type
{
	const x_type true_index = _x_axis.find(x);
	const size_t low_index = static_cast<size_t>(_clamp<value_type>(true_index, 0, _x_axis.size() - 2));

	const x_type frac_index = std::log(x / _x_axis[low_index]) / std::log(_x_axis[low_index + 1] / _x_axis[low_index]);
	const datatype low_value = std::log(_data[low_index]);
	const datatype high_value = std::log(_data[low_index + 1]);

	/*
	   FIXME: there is a potential problem if frac_index == 0 and high_value is infinite
	   or frac_index == 1 and low_value is infinite: 0 * inf == nan.
	   In all other cases, infinities are handled correctly.
	*/
	return std::exp((1 - frac_index)*low_value + frac_index*high_value);
}

template<typename datatype, typename ax>
auto array1D_ax<datatype, ax>::at_rounddown(x_type x) const -> value_type
{
	const x_type true_index = _x_axis.find(x);
	const size_t rounded_index = static_cast<size_t>(_clamp<value_type>(true_index, 0, _x_axis.size() - 1));
	return _data[rounded_index];
}

template<typename datatype, typename ax>
size_t array1D_ax<datatype, ax>::size() const
{
	return _x_axis.size();
}

template<typename datatype, typename ax>
auto array1D_ax<datatype, ax>::get_xrange() const -> std::pair<value_type, value_type>
{
	return{ _x_axis[0], _x_axis[size() - 1] };
}
