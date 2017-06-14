#ifndef __IONIZATION_TABLE_H_
#define __IONIZATION_TABLE_H_

/*
 * 2D table specifically intended for ionisation cross sections in the simulation loop.
 * This table does not bilinearly interpolate when a value is requested, rounding down
 * in both energy and P instead. This guarantees that physical binding energies are found.
 */

#include "table/array2D_ax.h"
#include "table/ax_logspace.h"
#include "table/ax_linspace.h"

template<typename real_type>
class ionization_table :
	private array2D_ax<real_type, ax_logspace<real_type>, ax_linspace<real_type>>
{
public:
	using value_type = real_type;
	using energy_axis_type = ax_logspace<real_type>;
	using probability_axis_type = ax_linspace<real_type>;
	using base_type = array2D_ax<value_type, energy_axis_type, probability_axis_type>;

	ionization_table(base_type const & ionization_table) :
		base_type(ionization_table)
	{}

	value_type get(value_type K, value_type P) const
	{
		const real_type true_x = base_type::_x_axis.find(K);
		const real_type true_y = base_type::_y_axis.find(P);

		// We DO NOT want to extrapolate on this end. Simply return "-1" binding energy.
		if (true_x < 0 || true_y < 0)
			return -1;

		// No interpolation: these are binding energies. K should be rounded down for obvious reasons.
		// P is also rounded down, consistent with e-scatter.
		const size_t K_index = std::min(static_cast<size_t>(true_x), base_type::width() - 1);
		const size_t P_index = std::min(static_cast<size_t>(true_y), base_type::height() - 1);
		return (*this)(K_index, P_index);
	}

	using base_type::operator();
	using base_type::get_x;
	using base_type::get_y;

	ionization_table(ionization_table &&) = default;
	ionization_table& operator=(ionization_table &&) = default;

private:
	// This object is supposed to be used in the simulation loop.
	// Prevent idiots from copying around data.
	// (if you're here because of a compiler error: pass by (const) reference)
	ionization_table(ionization_table const &) = delete;
	ionization_table& operator=(ionization_table const &) = delete;
};

#endif
