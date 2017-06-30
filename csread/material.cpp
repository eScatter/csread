#include <algorithm>
#include <cmath>
#include <tuple>
#include <H5Cpp.h>
#include "material.h"
#include "units/unit_parser.h"
#include "clamp.h"

/*
 * Helper functions for reading HDF5 data
 */

// Read an attribute
std::string h5_read_attribute(H5::H5Object const & object, std::string const & attribute_name)
{
	std::string value;
	H5::Attribute attribute = object.openAttribute(attribute_name);
	attribute.read(attribute.getStrType(), value);
	return value;
}

// Get the size of a 1D dataspace
hsize_t h5_get_1D_size(H5::DataSpace const & dataspace)
{
	if (dataspace.getSimpleExtentNdims() != 1)
		throw std::runtime_error("Table has unexpected dimension.");
	hsize_t dim;
	dataspace.getSimpleExtentDims(&dim);
	return dim;
}

// Get the size of a 2D dataspace
std::pair<hsize_t, hsize_t> h5_get_2D_size(H5::DataSpace const & dataspace)
{
	if (dataspace.getSimpleExtentNdims() != 2)
		throw std::runtime_error("Table has unexpected dimension.");
	hsize_t dim[2];
	dataspace.getSimpleExtentDims(dim);
	return{ dim[0], dim[1] };
}

// Load 1D table to std::vector of doubles
std::vector<double> h5_read_1D_table(H5::Group const & group, std::string const & dataset_name, dimension expected_dimensions)
{
	unit_parser _unit_parser;
	_unit_parser.add_default_units();

	H5::DataSet dataset = group.openDataSet(dataset_name);
	H5::DataSpace dataspace = dataset.getSpace();

	// Load units, check dimensionality
	const std::string unit_string = h5_read_attribute(dataset, "units");
	const quantity<double> unit_value = _unit_parser.parse_unit(unit_string);
	if (unit_value.units != expected_dimensions)
	{
		throw std::runtime_error("Unexpected dimensionality " + unit_string
			+ " for dataset " + dataset_name);
	}

	// Load data
	hsize_t dim = h5_get_1D_size(dataspace);
	std::vector<double> table(dim);
	dataset.read(table.data(), H5::PredType::NATIVE_DOUBLE);

	// Multiply by correct unit value
	for (double& d : table)
		d *= unit_value.value;

	return table;
}

// Load 2D data to std::vector of doubles, indexed as [y*width + x]
// Returns tuple(width, height, data)
std::tuple<size_t, size_t, std::vector<double>> h5_read_2D_table(H5::Group const & group, std::string const & dataset_name, dimension expected_dimensions)
{
	unit_parser _unit_parser;
	_unit_parser.add_default_units();

	H5::DataSet dataset = group.openDataSet(dataset_name);
	H5::DataSpace dataspace = dataset.getSpace();

	const std::string unit_string = h5_read_attribute(dataset, "units");
	const quantity<double> unit_value = _unit_parser.parse_unit(unit_string);
	if (unit_value.units != expected_dimensions)
	{
		throw std::runtime_error("Unexpected dimensionality " + unit_string
			+ " for dataset " + dataset_name);
	}

	// Load data
	std::pair<hsize_t, hsize_t> dim = h5_get_2D_size(dataspace);
	std::vector<double> table(dim.first * dim.second);
	dataset.read(table.data(), H5::PredType::NATIVE_DOUBLE);

	// Multiply by correct unit value
	for (double& d : table)
		d *= unit_value.value;

	return std::tuple<size_t, size_t, std::vector<double>>{ dim.first, dim.second, table };
}

// Read the "properties" dataset: array of compound datatype
// [string name] [float value] [string unit]
std::map<std::string, quantity<double>> h5_read_properties(H5::Group const & group)
{
	unit_parser _unit_parser;
	_unit_parser.add_default_units();

	H5::DataSet dataset = group.openDataSet("properties");
	H5::DataSpace dataspace = dataset.getSpace();

	// Define compound data type
	struct data_struct
	{
		char* name;
		double value;
		char* unit;
	};

	H5::CompType data_type(sizeof(data_struct));
	data_type.insertMember("name", HOFFSET(data_struct, name), dataset.getCompType().getMemberStrType(0));
	data_type.insertMember("value", HOFFSET(data_struct, value), H5::PredType::NATIVE_DOUBLE);
	data_type.insertMember("unit", HOFFSET(data_struct, unit), dataset.getCompType().getMemberStrType(2));

	// Read data
	hsize_t dim = h5_get_1D_size(dataspace);
	std::vector<data_struct> table(dim);
	dataset.read(table.data(), data_type);

	// Put the data into the desired map structure
	std::map<std::string, quantity<double>> property_map;
	for (auto property : table)
	{
		std::string name(property.name);
		quantity<double> value(property.value * _unit_parser.parse_unit(property.unit));
		property_map[name] = value;
	}

	// Free the dynamically-allocated memory for the strings
	dataset.vlenReclaim(table.data(), data_type, dataspace);

	return property_map;
}

std::pair<
	array1D_ax<double, ax_list<double>>,                     // cross_section(energy)
	array2D_ax<double, ax_list<double>, ax_linspace<double>> // angle_icdf(energy, P)
> read_elastic(H5::Group const & elastic_group)
{
	// Read energy axis
	ax_list<double> energy_axis(h5_read_1D_table(elastic_group, "energy", dimensions::energy));

	// Read cross sections
	std::vector<double> cross_section_table(h5_read_1D_table(elastic_group, "cross_section", dimensions::area));
	if (cross_section_table.size() != energy_axis.size())
		throw std::runtime_error("Cross section table has different size than energy table.");

	// Read inverse cumulative differential cross section
	size_t icdf_width, icdf_height;
	std::vector<double> icdf_table;
	std::tie(icdf_width, icdf_height, icdf_table) = h5_read_2D_table(elastic_group, "angle_icdf", dimensions::dimensionless); // radian
	if (icdf_width != energy_axis.size())
		throw std::runtime_error("ICDF table has different size than energy table.");

	// Assemble into arrays and we are done.
	return
	{
		{energy_axis, cross_section_table},
		{energy_axis, ax_linspace<double>(0, 1, icdf_height), icdf_table}
	};
}

std::pair<
	array1D_ax<double, ax_list<double>>,                     // cross_section(energy)
	array2D_ax<double, ax_list<double>, ax_linspace<double>> // w0_icdf(energy, P)
> read_inelastic(H5::Group const & inelastic_group)
{
	// Read energy axis
	ax_list<double> energy_axis(h5_read_1D_table(inelastic_group, "energy", dimensions::energy));

	// Read cross sections
	std::vector<double> cross_section_table(h5_read_1D_table(inelastic_group, "cross_section", dimensions::area));
	if (cross_section_table.size() != energy_axis.size())
		throw std::runtime_error("Cross section table has different size than energy table.");

	// Read inverse cumulative differential cross section
	size_t icdf_width, icdf_height;
	std::vector<double> icdf_table;
	std::tie(icdf_width, icdf_height, icdf_table) = h5_read_2D_table(inelastic_group, "w0_icdf", dimensions::energy);
	if (icdf_width != energy_axis.size())
		throw std::runtime_error("ICDF table has different size than energy table.");

	// Assemble into arrays and we are done.
	return
	{
		{ energy_axis, cross_section_table },
		{ energy_axis, ax_linspace<double>(0, 1, icdf_height), icdf_table }
	};
}

array2D_ax<double, ax_list<double>, ax_linspace<double>> // dE_icdf(energy, P)
read_ionization(H5::Group const & ionization_group)
{
	// Read energy axis
	ax_list<double> energy_axis(h5_read_1D_table(ionization_group, "energy", dimensions::energy));

	// Read inverse cumulative differential cross section
	size_t icdf_width, icdf_height;
	std::vector<double> icdf_table;
	std::tie(icdf_width, icdf_height, icdf_table) = h5_read_2D_table(ionization_group, "dE_icdf", dimensions::energy);
	if (icdf_width != energy_axis.size())
		throw std::runtime_error("ICDF table has different size than energy table.");

	// Assemble into arrays and we are done.
	return{ energy_axis, ax_linspace<double>(0, 1, icdf_height), icdf_table };
}

array1D_ax<double, ax_list<double>> read_electron_range(H5::Group const & electron_range_group)
{
	// Read energy axis
	ax_list<double> energy_axis(h5_read_1D_table(electron_range_group, "energy", dimensions::energy));

	// Read cross sections
	std::vector<double> range_table(h5_read_1D_table(electron_range_group, "range", dimensions::length));
	if (range_table.size() != energy_axis.size())
		throw std::runtime_error("Range table has different size than energy table.");

	// Assemble into arrays and we are done.
	return {energy_axis, range_table};
}

std::vector<double> read_outer_shells(H5::Group const & ionization_group)
{
	// Read outer shell energies
	return h5_read_1D_table(ionization_group, "outer_shells", dimensions::energy);
}

material::material(std::string const & filename)
{
	try
	{
		H5::Exception::dontPrint();
		H5::H5File hdf5_file(filename, H5F_ACC_RDONLY);

		std::tie(elastic_cross_section, elastic_angle_icdf) = read_elastic(hdf5_file.openGroup("elastic"));
		std::tie(inelastic_cross_section, inelastic_w0_icdf) = read_inelastic(hdf5_file.openGroup("inelastic"));
		ionization_dE_icdf = read_ionization(hdf5_file.openGroup("ionization"));
		outer_shells = read_outer_shells(hdf5_file.openGroup("ionization"));
		electron_range = read_electron_range(hdf5_file.openGroup("electron_range"));
		
		// Read a few properties
		name = h5_read_attribute(hdf5_file, "name");
		auto cnd_type_str = h5_read_attribute(hdf5_file, "conductor_type");
		auto property_map = h5_read_properties(hdf5_file);

		if (cnd_type_str == "metal")
			conductor_type = CND_METAL;
		else if (cnd_type_str == "insulator")
			conductor_type = CND_INSULATOR;
		else if (cnd_type_str == "semiconductor")
			conductor_type = CND_SEMICONDUCTOR;
		else
			throw std::runtime_error("Unknown conductor_type " + cnd_type_str);

		fermi = property_map.at("fermi");
		density = property_map.at("density");
		phonon_loss = property_map.at("phonon_loss");
		barrier = property_map.at("barrier");
		effective_A = property_map.at("effective_A");
		band_gap = (conductor_type == CND_METAL ? -1.*units::eV : property_map.at("band_gap"));
	}
	catch (H5::Exception const & error)
	{
		// HDF5 exception?
		// Simply rethrow as a std::runtime_error.
		throw std::runtime_error("Error encountered while reading HDF5 file: " + error.getDetailMsg());
	}
}

std::string material::get_name() const
{
	return name;
}
auto material::get_conductor_type() const -> conductor_type_t
{
	return conductor_type;
}
auto material::get_fermi() const -> quantity<intern_real>
{
	return fermi;
}
auto material::get_density() const -> quantity<intern_real>
{
	return density;
}
auto material::get_phonon_loss() const -> quantity<intern_real>
{
	return phonon_loss;
}
auto material::get_effective_A() const -> quantity<intern_real>
{
	return effective_A;
}
auto material::get_barrier() const -> quantity<intern_real>
{
	return barrier;
}
auto material::get_band_gap() const -> quantity<intern_real>
{
	return band_gap;
}

auto material::get_elastic_imfp(fast_real K_min, fast_real K_max, size_t N) const -> imfp_table_t
{
	const intern_real number_density = get_density().value;
	return to_fast_table(elastic_cross_section, K_min, K_max, N,
		[number_density](intern_table1D_t const & table, intern_real K) -> fast_real
		{
			const intern_real cross_section = table.at_loglog(K);
			return (fast_real)std::log(cross_section * number_density);
		});
}
auto material::get_elastic_angle_icdf(fast_real K_min, fast_real K_max, size_t N_K, size_t N_P) const -> icdf_table_t
{
	return to_fast_table(elastic_angle_icdf, K_min, K_max, N_K, N_P,
		[](intern_table2D_t const & table, intern_real K, intern_real P) -> fast_real
		{
			return (fast_real)table.at_linear(K, P);
		});
}
auto material::get_inelastic_imfp(fast_real K_min, fast_real K_max, size_t N) const -> imfp_table_t
{
	intern_real number_density = get_density().value;
	return to_fast_table(inelastic_cross_section, K_min, K_max, N,
		[number_density](intern_table1D_t const & table, intern_real K) -> fast_real
		{
			const intern_real cross_section = table.at_loglog(K);
			return (fast_real)std::log(cross_section * number_density);
		});
}
auto material::get_inelastic_w0_icdf(fast_real K_min, fast_real K_max, size_t N_K, size_t N_P) const -> icdf_table_t
{
	return to_fast_table(inelastic_w0_icdf, K_min, K_max, N_K, N_P,
		[](intern_table2D_t const & table, intern_real K, intern_real P) -> fast_real
		{
			return (fast_real)table.at_linear(K, P);
		});
}

auto material::get_ionization_icdf(fast_real K_min, fast_real K_max, size_t N_K, size_t N_P) const -> ionization_table_t
{
	return to_fast_table(ionization_dE_icdf, K_min, K_max, N_K, N_P,
		[](intern_table2D_t const & table, intern_real K, intern_real P) -> fast_real
		{
			intern_real binding = table.at_rounddown(K, P);

			if (!std::isfinite(binding))
				binding = -1;

			return (fast_real)binding;
		});
}

auto material::get_outer_shells() const -> outer_shell_table_t
{
	std::vector<fast_real> return_vector(outer_shells.size());
	for (size_t i = 0; i < outer_shells.size(); ++i)
		return_vector[i] = (fast_real)outer_shells[i];
	return return_vector;
}

auto material::get_electron_range(fast_real K_min, fast_real K_max, size_t N) const -> range_table_t
{
	return to_fast_table(electron_range, K_min, K_max, N,
		[](intern_table1D_t const & table, intern_real K) -> fast_real
		{
			const intern_real electron_range = table.at_loglog(K);
			return (fast_real)std::log(electron_range);
		});
}

auto material::get_elastic_energy_range() const -> std::pair<intern_real, intern_real>
{
	// Note: the energy axis is shared between the cross section and icdf tables.
	return elastic_cross_section.get_xrange();
}

auto material::get_inelastic_energy_range() const -> std::pair<intern_real, intern_real>
{
	// Note: the energy axis is shared between the cross section and icdf tables.
	return inelastic_cross_section.get_xrange();
}

auto material::get_ionization_energy_range() const -> std::pair<intern_real, intern_real>
{
	return ionization_dE_icdf.get_xrange();
}

template<typename conversion_func>
auto material::to_fast_table(intern_table1D_t const & intern,
	fast_real K_min, fast_real K_max, size_t N, conversion_func f) -> fast_table1D_t
{
	// Kinetic energy axis
	ax_logspace<fast_real> K_axis(K_min, K_max, N);

	// Values
	std::vector<fast_real> values(N);
	for (size_t i = 0; i < N; ++i)
	{
		values[i] = f(intern, K_axis[i]);
	}

	return{ K_axis, values };
}

template<typename conversion_func>
auto material::to_fast_table(intern_table2D_t const & intern,
	fast_real K_min, fast_real K_max, size_t N_K, size_t N_P, conversion_func f) -> fast_table2D_t
{
	// Kinetic energy axis
	ax_logspace<fast_real> K_axis(K_min, K_max, N_K);
	// Probability axis
	ax_linspace<fast_real> P_axis(0, 1, N_P);

	// Values
	std::vector<fast_real> values(N_K*N_P);
	for (size_t ik = 0; ik < N_K; ++ik)
	{
		for (size_t ip = 0; ip < N_P; ++ip)
		{
			values[ik*N_P + ip] = f(intern, K_axis[ik], P_axis[ip]);
		}
	}

	return{ K_axis, P_axis, values };
}
