#include "../include/math_utilities.hpp"
#include "../include/vector.hpp"
#include <cmath>
#include <cstdint>
#include <ostream>
#include <random>

namespace render {

  // RNG: constructor y generación de dobles
  // implementación del generador Mersenne-Twister y funciones auxiliares para obtener dobles en
  // [0,1) y en [min,max)
  RNG::RNG(uint64_t seed) : generator(seed) { }

  double RNG::random_double() {
    return std::generate_canonical<double, 10>(generator);
  }

  double RNG::random_double(double min, double max) {
    return min + (max - min) * random_double();
  }

  // generación de vector aleatorio
  // devuelve un vector cuyas componentes están uniformemente distribuidas en [min, max] usando el
  // RNG provisto
  direction_vector random_vec(RNG & rng, double min, double max) {
    return {rng.random_double(min, max), rng.random_double(min, max), rng.random_double(min, max)};
  }

  // escritura de color
  // aplica promedio por muestras y corrección gamma, luego escala a 0-255 y escribe en 'out' en
  // formato PPM (tripleta R G B)
  void write_color(std::ostream & out, color_vector pixel_color, int samples_per_pixel,
                   double gamma_value) {
    double const scale     = 1.0 / samples_per_pixel;
    double r               = pixel_color.r() * scale;
    double g               = pixel_color.g() * scale;
    double b               = pixel_color.b() * scale;
    double const gamma_inv = 1.0 / gamma_value;
    r                      = std::pow(r, gamma_inv);
    g                      = std::pow(g, gamma_inv);
    b                      = std::pow(b, gamma_inv);
    auto clamp_to_one      = [](double val) {
      if (val < 0.0) {
        return 0.0;
      }
      if (val > 1.0) {
        return 1.0;
      }
      return val;
    };
    int const r_out = static_cast<int>(256.0 * clamp_to_one(r));
    int const g_out = static_cast<int>(256.0 * clamp_to_one(g));
    int const b_out = static_cast<int>(256.0 * clamp_to_one(b));
    out << r_out << ' ' << g_out << ' ' << b_out << '\n';
  }

}  // namespace render
