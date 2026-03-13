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

namespace glm {
template <std::size_t N, typename T>
inline std::ostream &operator<<(std::ostream &os, const glm::vec<N, T> &v) {
  os << '[';
  for (std::size_t i = 0; i < N; ++i) {
    if (i)
      os << ", ";
    os << v[i];
  }
  return os << ']';
}
} // namespace glm

namespace wt {

template <std::size_t N, wt::FloatingPoint T>
inline std::ostream &operator<<(std::ostream &os,
                                const wt::unit_vector<N, T> &v) {
  os << '[';
  for (std::size_t i = 0; i < N; ++i) {
    if (i)
      os << ", ";
    os << v[i];
  }
  return os << ']';
}

template <std::size_t N, wt::Quantity Q>
inline std::ostream &operator<<(std::ostream &os,
                                const wt::quantity_vector<N, Q> &v) {
  os << '[';
  for (std::size_t i = 0; i < N; ++i) {
    if (i)
      os << ", ";
    // print the raw numerical value; units are implicit from the type
    os << v[i].numerical_value_in(wt::u::m);
  }
  return os << ']';
}

inline std::ostream &operator<<(std::ostream &os, const wt::frame_t &f) {
  return os << "{t=" << f.t << ", b=" << f.b << ", n=" << f.n << '}';
}

} // namespace wt

int main(int argc, char **argv) {

  auto ctx = wt::wt_context_t{};
  const wt::QE_area_t scale = 1 * wt::area_t::unit;

  const wt::frequency_t f = 100 * wt::u::Hz;
  const auto c = 343.0 * (wt::u::m / wt::u::s); // speed of sound

  const wt::wavelength_t lambda = 675 * wt::u::nm; // c / f;
  const wt::wavenumber_t k = wt::wavelen_to_wavenum(lambda);

  std::cout << "Simulating at a wavelength of " << value_cast<wt::u::nm>(lambda)
            << std::endl;

  // Threepoint setup
  const auto p_emitter = wt::vec3_t(2, 0, 2) * wt::u::m; // Emitter
  const auto p_sensor = wt::vec3_t(-2, 0, 2) * wt::u::m; // Sensor
  const auto p_surface = wt::vec3_t(0, 0, 0) * wt::u::m; // Surface
  const auto n_surface = wt::dir3_t(0, 0, 1);

  // ------------------------------ Setup emitter ---------------–––---------

  const auto spectrum_uniform =
      std::make_shared<wt::spectrum::uniform_t>("uniform", wt::f_t(1));

  auto point = wt::emitter::point_t("point_emitter", p_emitter,
                                    spectrum_uniform, std::nullopt);

  // ----------------------------- Sensor setup -----------------------------

  const auto sensor_transform =
      wt::transform_t::translate(p_sensor) *
      wt::transform_t::rotate(wt::dir3_t{0, 0, 1}, wt::dir3_t{1, 0, 0});

  const wt::pqvec2_t sensor_extent{1 * wt::u::m, 1 * wt::u::m};
  const auto response = std::make_shared<wt::sensor::response::monochromatic_t>(
      "monochomatic", nullptr, spectrum_uniform);

  wt::sensor::film_t<2, false> film(ctx, {1, 1}, // width, height in pixels
                                    response, 1, {1, 1}, {false, false});

  const auto virtual_sensor =
      std::make_shared<wt::sensor::virtual_plane_sensor_t>(
          ctx, "virtual_plane_sensor", sensor_transform, sensor_extent,
          std::move(film),
          1,     // samples_per_element
          false, // ray_trace
          std::nullopt);
  const auto sensor_n = virtual_sensor->frame().n;

  std::cout << "virtual_sensor->frame().n() = " << sensor_n << std::endl;

  // ------------------- Intersection Surface setup ----------------------

  auto floor_intersect =
      wt::intersection_surface_t(n_surface,
                                 p_surface); // Dummy floor pointing upwards

  // ----------------------------- Trace & measure --------------------------

  auto sampler = wt::sampler::uniform_t("sampler");

  const auto wi = wt::m::normalize(p_sensor - p_surface);
  const auto wo = wt::m::normalize(p_emitter - p_surface);

  auto sensor_sample = virtual_sensor->sample_toward(
      sampler, wt::vec3u32_t(0, 0, 0), p_surface, k);

  std::printf("tan(alpha) = %f\n",
              sensor_sample.beam.get_envelope().get_tan_alpha());
  std::printf(
      "initial x = %f\n",
      sensor_sample.beam.get_envelope().x0().numerical_value_in(wt::u::m));

  std::cout << "sensor_sample.beam.intensity() = "
            << sensor_sample.beam.intensity() << std::endl;

  const auto emitter_sample = point.sample_direct(sampler, p_surface, k);

  std::cout << "emitter_sample.beam.intensity() = "
            << emitter_sample.beam.intensity() << std::endl;

  const auto reflectance = 1.0;
  const auto cos_theta = wt::m::dot(wo, wt::dir3_t(0, 0, 1));
  auto bsdf_result = wt::bsdf::bsdf_result_t{
      .M = cos_theta * wt::m::inv_pi * reflectance *
           wt::mueller_operator_t::perfect_depolarizer()};

  std::cout << "cos_theta * wt::m::inv_pi * reflectance = "
            << cos_theta * wt::m::inv_pi * reflectance << std::endl;

  // Does not affect the outcome here
  floor_intersect.footprint = sensor_sample.beam.surface_footprint_static(
      floor_intersect, wt::m::length(wi) * wt::u::m);

  std::cout << "floor_intersect.footprint.x = " << floor_intersect.footprint.x
            << std::endl;

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
};
