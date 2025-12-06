#ifndef RENDER_MATH_UTIL_HPP
#define RENDER_MATH_UTIL_HPP

#include "vector.hpp"
#include <cmath>
#include <iostream>
#include <random>

namespace render {

  // CLASE QUE DEFINE LA GENERACIÓN DE NÚMEROS ALEATORIOS
  class RNG {
  private:
    std::mt19937_64 generator;

  public:
    explicit RNG(uint64_t seed);
    [[nodiscard]] double random_double();
    [[nodiscard]] double random_double(double min, double max);
  };

  // vector aleatorio con componentes en el rango [min, max]
  direction_vector random_vec(RNG & rng, double min = -1.0, double max = 1.0);

  // PROCESADO DEL COLOR (3.2)
  void write_color(std::ostream & out, color_vector pixel_color, int samples_per_pixel,
                   double gamma_value);

}  // namespace render

#endif
