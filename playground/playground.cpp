#include "mp-units/systems/si/unit_symbols.h"
#include "wt/beam/beam.hpp"
#include "wt/emitter/area.hpp"
#include "wt/math/defs.hpp"
#include "wt/math/frame.hpp"
#include "wt/math/quantity/defs.hpp"
#include "wt/math/quantity/math.hpp"
#include "wt/math/quantity/quantity_vector.hpp"
#include "wt/math/shapes/elliptic_cone.hpp"
#include "wt/math/shapes/ray.hpp"
#include "wt/math/unit_vector/unit_vector.hpp"
#include "wt/sensor/sensor/perspective.hpp"
#include "wt/sensor/sensor/virtual_plane_sensor.hpp"
#include "wt/spectrum/uniform.hpp"
#include "wt/texture/texture.hpp"

#include <iostream>

int main(int argc, char **argv) {

  wt::QE_area_t scale = 1 * wt::area_t::unit;

  wt::frequency_t f = 100 * wt::u::Hz;
  auto c = 343.0 * (wt::u::m / wt::u::s); // speed of sound

  wt::wavelength_t lambda = 675 * wt::u::nm;//c / f;
  wt::wavenumber_t k = wt::wavelen_to_wavenum(lambda);
  
  std::cout << "Simulating at a wavelength of " << value_cast<wt::u::m>(lambda) << " (" << value_cast<wt::u::Hz>(f) <<")" << std::endl;

  auto area_emitter = wt::emitter::area_t("emitter1", nullptr, nullptr);
  
  auto sourcing_geometry = area_emitter.sourcing_geometry(k);

  std::cout << "sourcing_geometry.initial_spatial_lengths = ["
            << sourcing_geometry.initial_spatial_lengths.x << ", "
            << sourcing_geometry.initial_spatial_lengths.y << "]" << std::endl;

  auto origin = wt::pqvec3_t::zero();
  auto direction = wt::dir3_t{0, 0, 1};

  auto ray = wt::ray_t{origin, direction};

  wt::length_t self_intersection_distance = 0 * wt::u::m;
  auto cone = sourcing_geometry.envelope(ray, self_intersection_distance);

  std::cout << "cone.get_tan_alpha() = " << cone.get_tan_alpha() << std::endl;

  auto beam = wt::importance_intensity_beam_t{cone, scale, k};

  auto dist = 1000 * wt::u::m;
  auto fp = beam.footprint(dist) / wt::u::m; // make dimensionless

  std::cout << "Beam footprint (at " << dist << ") = [" << fp.x << ", " << fp.y
            << ", " << fp.z << "]" << std::endl;
};
