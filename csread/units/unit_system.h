#ifndef __UNIT_SYSTEM_H_
#define __UNIT_SYSTEM_H_

/*
 * A numerical choice for fundamental and derived units.
 * 
 * TODO: use variable templates here, but VS does not seem to like that yet.
 */

#include "quantity.h"

namespace units
{
	// Base units
	constexpr quantity<double> dimensionless{ 1, dimensions::dimensionless };
	constexpr quantity<double> eV{ 1, dimensions::energy };      // electronvolts
	constexpr quantity<double> nm{ 1, dimensions::length };      // nanometers
	constexpr quantity<double> ns{ 1, dimensions::time };        // nanoseconds
	constexpr quantity<double> K { 1, dimensions::temperature }; // Kelvin
	constexpr quantity<double> e { 1, dimensions::charge };      // electron charge


	// Auxiliary units
	constexpr quantity<double> s  { 1e9, dimensions::time };         // second
	constexpr quantity<double> m  { 1e9, dimensions::length };       // meter
	constexpr quantity<double> cm { 1e7, dimensions::length };       // centimeter
	constexpr quantity<double> Clb{ 6.2415e18, dimensions::charge }; // Coulomb
	constexpr quantity<double> g  { 6.2415e15, dimensions::mass };   // gram
};

#endif
