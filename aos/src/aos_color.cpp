#include "../include/aos_color.hpp"
#include <cmath>
#include <cstdint>

namespace color {

  Color background_color(double dir_y, Color const & light_color, Color const & dark_color) {
    double const m = (dir_y + 1.0) / 2.0;

    // color = (1-m)*light_color + m*dark_color
    return (1.0 - m) * dark_color + m * light_color;
  }

  uint8_t to_byte(double value, double gamma) {
    double const clamped   = clamp(value, 0.0, 1.0);
    double const corrected = std::pow(clamped, 1.0 / gamma);

    return static_cast<uint8_t>(corrected * 255.0);
  }

}  // namespace color
