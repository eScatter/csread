#ifndef __ICDF_TABLE_H_
#define __ICDF_TABLE_H_

/*
 * 2D table specifically intended for inverse cumulative distribution functions in the simulation loop.
 */

#include "table/array2D_ax.h"
#include "table/ax_logspace.h"
#include "table/ax_linspace.h"

template<typename real_type>
class icdf_table :
	private array2D_ax<real_type, ax_logspace<real_type>, ax_linspace<real_type>>
{
public:
	using value_type = real_type;
	using energy_axis_type = ax_logspace<real_type>;
	using probability_axis_type = ax_linspace<real_type>;
	using base_type = array2D_ax<value_type, energy_axis_type, probability_axis_type>;

	icdf_table(base_type const & icdf_table) :
		base_type(icdf_table)
	{}

	value_type get(value_type K, value_type P) const
	{
		return at_linear(K, P);
	}

	using base_type::operator();
	using base_type::get_x;
	using base_type::get_y;

	icdf_table(icdf_table &&) = default;
	icdf_table& operator=(icdf_table &&) = default;

private:
	// This object is supposed to be used in the simulation loop.
	// Prevent idiots from copying around data.
	// (if you're here because of a compiler error: pass by (const) reference)
	icdf_table(icdf_table const &) = delete;
	icdf_table& operator=(icdf_table const &) = delete;
};

#endif