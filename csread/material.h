#ifndef __MATERIAL_H_
#define __MATERIAL_H_

/*
 * General material class, holds all data there is to know about the material.
 * 
 * The imfp and icdf tables are simply stored as numbers, with their units dropped.
 * When read from the hdf5 file, it is ensured that the dimensions are correct.
 * The values are then converted to base units as defined in unit_system.h, so,
 * for example, energies are in eV and lengths are in nm.
 * 
 * The material property list does retain units.
 */

#include <string>
#include <map>
#include "imfp_table.h"
#include "icdf_table.h"
#include "ionization_table.h"
#include "table/array1D_ax.h"
#include "table/array2D_ax.h"
#include "table/ax_list.h"
#include "table/ax_linspace.h"
#include "table/ax_logspace.h"
#include "units/quantity.h"

class material
{
public:
	using intern_real = double;
	using fast_real = float;

	using imfp_table_t = imfp_table<fast_real>;
	using icdf_table_t = icdf_table<fast_real>;
	using ionization_table_t = ionization_table<fast_real>;
	using outer_shell_table_t = std::vector<fast_real>;

	// Different types of conductor
	enum conductor_type_t
	{
		CND_METAL,
		CND_SEMICONDUCTOR,
		CND_INSULATOR
	};

	// Load material from hdf5 file.
	// May throw std::runtime_error exceptions.
	material(std::string const & filename);

	// Access some properties
	std::string get_name() const;
	conductor_type_t get_conductor_type() const;
	quantity<intern_real> get_fermi() const;
	quantity<intern_real> get_density() const;
	quantity<intern_real> get_phonon_loss() const;
	quantity<intern_real> get_barrier() const;
	quantity<intern_real> get_band_gap() const; // -1 eV if metal

	// Build tables that can be used in a simulation.
	// Units are as described above: energy in eV, angles in radian.
	imfp_table_t get_elastic_imfp(fast_real K_min, fast_real K_max, size_t N) const;
	icdf_table_t get_elastic_angle_icdf(fast_real K_min, fast_real K_max, size_t N_K, size_t N_P) const;
	imfp_table_t get_inelastic_imfp(fast_real K_min, fast_real K_max, size_t N) const;
	icdf_table_t get_inelastic_w0_icdf(fast_real K_min, fast_real K_max, size_t N_K, size_t N_P) const;
	ionization_table_t get_ionization_icdf(fast_real K_min, fast_real K_max, size_t N_K, size_t N_P) const;
	outer_shell_table_t get_outer_shells() const;

	// Get energy range. Units are as defined in unit_system.h, which is eV.
	std::pair<intern_real, intern_real> get_elastic_energy_range() const;
	std::pair<intern_real, intern_real> get_inelastic_energy_range() const;
	std::pair<intern_real, intern_real> get_ionization_energy_range() const;

private:
	using intern_table1D_t = array1D_ax<intern_real, ax_list<intern_real>>;
	using intern_table2D_t = array2D_ax<intern_real, ax_list<intern_real>, ax_linspace<intern_real>>;
	using fast_table1D_t = array1D_ax<fast_real, ax_logspace<fast_real>>;
	using fast_table2D_t = array2D_ax<fast_real, ax_logspace<fast_real>, ax_linspace<fast_real>>;

	std::string name;
	conductor_type_t conductor_type;
	quantity<intern_real> fermi;
	quantity<intern_real> density;
	quantity<intern_real> phonon_loss;
	quantity<intern_real> barrier;
	quantity<intern_real> band_gap; // -1 eV if conductor_type == CND_METAL

	intern_table1D_t elastic_cross_section;
	intern_table2D_t elastic_angle_icdf;

	intern_table1D_t inelastic_cross_section;
	intern_table2D_t inelastic_w0_icdf;

	intern_table2D_t ionization_dE_icdf;
	std::vector<intern_real> outer_shells;

	template<typename conversion_func>
	static fast_table1D_t to_fast_table(intern_table1D_t const & intern,
		fast_real K_min, fast_real K_max, size_t N, conversion_func f);
	template<typename conversion_func>
	static fast_table2D_t to_fast_table(intern_table2D_t const & intern,
		fast_real K_min, fast_real K_max, size_t N_K, size_t N_P, conversion_func f);
};

#endif