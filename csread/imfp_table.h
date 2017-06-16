#ifndef __IMFP_TABLE_H_
#define __IMFP_TABLE_H_

/*
 * 1D table specifically intended for inverse mean free paths in the simulation loop.
 * The IMFP values are stored in log space and therefore get log interpolation.
 */

#include <limits>
#include <cmath>
#include "table/array1D_ax.h"
#include "table/ax_logspace.h"

template<typename real_type>
class imfp_table :
	private array1D_ax<real_type, ax_logspace<real_type>>
{
public:
	using value_type = real_type;
	using axis_type = ax_logspace<real_type>;
	using base_type = array1D_ax<value_type, axis_type>;

	imfp_table(base_type const & log_imfp_table) :
		base_type(log_imfp_table)
	{}

	value_type get(value_type K) const
	{
		return std::exp(base_type::at_linear(K));
	}

	// Note: base_type::operator() gets the LOG imfp.
	using base_type::operator();
	using base_type::get_x;

	imfp_table(imfp_table &&) = default;
	imfp_table& operator=(imfp_table &&) = default;

private:
	// This object is supposed to be used in the simulation loop.
	// Prevent idiots from copying around data.
	// (if you're here because of a compiler error: pass by (const) reference)
	imfp_table(imfp_table const &) = delete;
	imfp_table& operator=(imfp_table const &) = delete;
};

#endif