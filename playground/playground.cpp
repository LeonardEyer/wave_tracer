#include "mp-units/systems/si/unit_symbols.h"
#include "wt/beam/beam.hpp"
#include "wt/math/quantity/defs.hpp"
#include "wt/math/quantity/quantity_vector.hpp"
#include "wt/math/shapes/elliptic_cone.hpp"
#include "wt/math/shapes/ray.hpp"
#include "wt/math/unit_vector/unit_vector.hpp"
#include <iostream>

int main(int argc, char **argv) {

  wt::QE_area_t scale = 1 * wt::area_t::unit;
  wt::f_t tan_alpha = 0.1;
  wt::wavenumber_t k = 1 * wt::u::one / wt::u::mm;

  auto origin = wt::pqvec3_t::zero();
  auto direction = wt::dir3_t{0, 0, 1};

  auto ray = wt::ray_t{origin, direction};
  auto cone = wt::elliptic_cone_t{ray, tan_alpha};
  auto beam = wt::importance_intensity_beam_t{cone, scale, k};

  std::cout << "Beam size = " << beam.std_dev(1 * wt::u::m).x << std::endl;
};
