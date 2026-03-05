#include "mp-units/systems/si/unit_symbols.h"
#include "wt/beam/beam.hpp"
#include "wt/bsdf/diffuse.hpp"
#include "wt/emitter/area.hpp"
#include "wt/math/defs.hpp"
#include "wt/math/frame.hpp"
#include "wt/math/quantity/defs.hpp"
#include "wt/math/quantity/math.hpp"
#include "wt/math/quantity/quantity_vector.hpp"
#include "wt/math/shapes/elliptic_cone.hpp"
#include "wt/math/shapes/ray.hpp"
#include "wt/math/unit_vector/unit_vector.hpp"
#include "wt/mesh/sphere.hpp"
#include "wt/sampler/uniform.hpp"
#include "wt/sensor/sensor/perspective.hpp"
#include "wt/sensor/sensor/virtual_plane_sensor.hpp"
#include "wt/spectrum/uniform.hpp"
#include "wt/texture/texture.hpp"

#include <iostream>

int main(int argc, char **argv) {

  auto ctx = wt::wt_context_t{};

  wt::QE_area_t scale = 1 * wt::area_t::unit;

  wt::frequency_t f = 100 * wt::u::Hz;
  auto c = 343.0 * (wt::u::m / wt::u::s); // speed of sound

  wt::wavelength_t lambda = 675 * wt::u::nm; // c / f;
  wt::wavenumber_t k = wt::wavelen_to_wavenum(lambda);

  std::cout << "Simulating at a wavelength of " << value_cast<wt::u::nm>(lambda)
            << std::endl;

  // ------------------------------ Setup emitter ---------------–––---------
    auto origin = wt::vec3_t(0) * wt::u::m;
    auto sphere = wt::mesh::sphere_t::create(
        "sphere", ctx, origin, 1. * wt::u::m, wt::transform_d_t{});
    auto spectrum_uniform =
        std::make_shared<wt::spectrum::uniform_t>("uniform", wt::f_t(1));
    auto specrum_uniform2 =
        std::make_shared<wt::spectrum::uniform_t>("uniform", wt::f_t(.5));
    auto constant_texture =
        std::make_shared<wt::texture::constant_t>("constant", spectrum_uniform);
    auto bsdf_diffuse =
        std::make_shared<wt::bsdf::diffuse_t>("diffuse", constant_texture);

    auto area_emitter = std::make_shared<wt::emitter::area_t>(
        "emitter1", nullptr, spectrum_uniform);

    auto area = wt::shape_t("area_emitter", bsdf_diffuse, area_emitter, sphere);

    // hack (actually private member)
    area_emitter->set_shape(ctx, &area);
  // ---------------------------------------------------------------------

  auto sampler = wt::sampler::uniform_t("sampler");

  auto emitter_sample = area_emitter->sample(sampler, k);
  auto beam = emitter_sample.beam;

  std::cout << "beam.dir() = [" << beam.dir().x << ", " << beam.dir().y << ", "
            << beam.dir().z << "]" << std::endl;
  std::cout << "cone.get_tan_alpha() = " << beam.get_envelope().get_tan_alpha()
            << std::endl;

  auto dist = 1000 * wt::u::m;
  auto fp = beam.footprint(dist) / wt::u::m; // make dimensionless

  std::cout << "Beam footprint (at " << dist << ") = [" << fp.x << ", " << fp.y
            << ", " << fp.z << "]" << std::endl;
};
