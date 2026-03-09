#include "mp-units/systems/si/unit_symbols.h"
#include "wt/beam/beam.hpp"
#include "wt/bsdf/diffuse.hpp"
#include "wt/emitter/area.hpp"
#include "wt/emitter/point.hpp"
#include "wt/math/defs.hpp"
#include "wt/math/frame.hpp"
#include "wt/math/quantity/defs.hpp"
#include "wt/math/quantity/math.hpp"
#include "wt/math/quantity/quantity_vector.hpp"
#include "wt/math/shapes/elliptic_cone.hpp"
#include "wt/math/shapes/ray.hpp"
#include "wt/math/unit_vector/unit_vector.hpp"
#include "wt/mesh/rectangle.hpp"
#include "wt/mesh/sphere.hpp"
#include "wt/sampler/uniform.hpp"
#include "wt/sensor/response/monochromatic.hpp"
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

  // Threepoint setup
  auto p_emitter = wt::vec3_t(0, 0, 2) * wt::u::m;  // Emitter
  auto p_sensor = wt::vec3_t(0, 0, -2) * wt::u::m;  // Sensor
  auto p_surface = wt::vec3_t(0, -1, 0) * wt::u::m; // Surface

  // ------------------------------ Setup emitter ---------------–––---------
  auto sphere = wt::mesh::sphere_t::create("sphere", ctx, p_emitter,
                                           1. * wt::u::mm, wt::transform_d_t{});

  auto spectrum_uniform =
      std::make_shared<wt::spectrum::uniform_t>("uniform", wt::f_t(1));

  auto point = wt::emitter::point_t("point_emitter", p_emitter,
                                    spectrum_uniform, std::nullopt);

  // ----------------------------- Sensor setup -----------------------------

  auto sensor_transform =
      wt::transform_t{}.translate(p_sensor); // Position away on Z-axis

  wt::pqvec2_t sensor_extent{1 * wt::u::m, 1 * wt::u::m};
  auto response = std::make_shared<wt::sensor::response::monochromatic_t>(
      "monochomatic", nullptr, spectrum_uniform);

  wt::sensor::film_t<2, false> film(ctx, {64, 64}, // width, height in pixels
                                    response, 1, {4, 4}, {false, false});

  auto virtual_sensor = std::make_shared<wt::sensor::virtual_plane_sensor_t>(
      ctx, "virtual_plane_sensor", sensor_transform, sensor_extent,
      std::move(film),
      1,     // samples_per_element
      false, // ray_trace
      std::nullopt);

  // ------------------- Intersection Surface setup ----------------------

  const auto floor_intersect =
      wt::intersection_surface_t(wt::dir3_t(0, 0, 1),
                                 p_surface); // Dummy floor pointing upwards

  // ----------------------------- Trace & measure --------------------------

  auto sampler = wt::sampler::uniform_t("sampler");

  auto wi = wt::m::normalize(p_sensor - p_surface);
  auto wo = wt::m::normalize(p_emitter - p_surface);

  auto sensor_sample = virtual_sensor->sample_toward(
      sampler, wt::vec3u32_t(0, 0, 0), p_surface, k);

  std::cout << "sensor_sample.beam.intensity() = "
            << sensor_sample.beam.intensity() << std::endl;

  auto emitter_sample = point.sample_direct(sampler, p_surface, k);

  std::cout << "emitter_sample.beam.intensity() = "
            << emitter_sample.beam.intensity() << std::endl;

  const auto reflectance = 1.0;
  const auto cos_theta = wt::m::dot(wo, wt::dir3_t(0, 0, 1));
  auto bsdf_result = wt::bsdf::bsdf_result_t{.M = cos_theta * wt::m::inv_pi * reflectance *
					     wt::mueller_operator_t::perfect_depolarizer()};

  sensor_sample.beam.transform_surface_interaction(floor_intersect, wo,
                                                   bsdf_result, 1);

    std::cout << "(after reflection) sensor_sample.beam.intensity() = "
            << sensor_sample.beam.intensity() << std::endl;


  std::cout << "wt::beam::integrate_beams(sensor_sample.beam, "
               "emitter_sample.beam).intensity() = "
            << wt::beam::integrate_beams(sensor_sample.beam,
                                         emitter_sample.beam)
                   .intensity()
            << std::endl;

  // auto dist = 1000 * wt::u::m;
  // auto fp = beam.footprint(dist); // / wt::u::m; // make dimensionless

  // std::cout << "Beam footprint (at " << dist << ") = [" << fp.x << ", " << fp.y
  //           << ", " << fp.z << "]" << std::endl;

  // // === QUERY RADIOMETRIC POWER ===
  // // Get emitter's spectral power at the wavenumber
  // wt::spectral_radiant_flux_t power_at_k = area_emitter->power(k);
  // std::cout << "\n=== RADIOMETRIC QUANTITIES ===" << std::endl;
  // std::cout << "Emitter spectral power at k: " << power_at_k << std::endl;

  // // Get emitter's total power over a wavenumber range
  // wt::wavenumber_t k_min = k * wt::f_t(0.9);
  // wt::wavenumber_t k_max = k * wt::f_t(1.1);
  // wt::range_t<wt::wavenumber_t> k_range{ k_min, k_max };

  // wt::radiant_flux_t total_power = area_emitter->power(k_range);
  // std::cout << "Emitter total power in range [" << k_min << ", " << k_max <<
  // "]: "
  //           << total_power << std::endl;

  // // === SENSOR CONNECTION (Simple Direct Connection) ===
  // // Check if a beam from the emitter hits the sensor
  // // Note: For a full connection, you would need to:
  // // 1. Trace the beam from emitter to sensor location
  // // 2. Use sensor->Si() method to compute the connection

  // std::cout << "\n=== SENSOR PROPERTIES ===" << std::endl;
  // std::cout << "Sensor extent: " << virtual_sensor->extent() << std::endl;
  // std::cout << "Sensor area: " << virtual_sensor->area() << std::endl;
  // std::cout << "Sensor resolution: " << virtual_sensor->resolution().x
  //           << "x" << virtual_sensor->resolution().y << std::endl;

  // // Query sensor sensitivity spectrum
  // const auto& sensor_spectrum = virtual_sensor->sensitivity_spectrum();
  // const auto sensor_power = sensor_spectrum.power();
  // std::cout << "Sensor sensitivity power: " << sensor_power << std::endl;
};
