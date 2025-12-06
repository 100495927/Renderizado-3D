#ifndef SOA_COLOR_HPP
#define SOA_COLOR_HPP

#include <cstdint>

namespace color {

  struct Color {
    double r;
    double g;
    double b;

    Color() : r(0.0), g(0.0), b(0.0) { }

    Color(double red, double green, double blue) : r(red), g(green), b(blue) { }

    // Operadores aritméticos
    Color operator+(Color const & other) const { return {r + other.r, g + other.g, b + other.b}; }

    Color operator*(double scalar) const { return {r * scalar, g * scalar, b * scalar}; }
  };

  // Operador para multiplicar escalar por color
  inline Color operator*(double scalar, Color const & color) {
    return color * scalar;
  }

  // Calculamos el color del fondo según la dirección del rayo
  // Fórmula a aplicar (1-m)*claro + m*oscuro
  [[nodiscard]] Color background_color(double dir_y, Color const & light_color,
                                       Color const & dark_color);

  // Limitamos el valor al rango [min, max]
  [[nodiscard]] inline double clamp(double value, double min, double max) {
    if (value < min) {
      return min;
    }
    if (value > max) {
      return max;
    }
    return value;
  }

  // Converitmos un valor double [0, 1] a byte [0, 255] con corrección gamma
  [[nodiscard]] uint8_t to_byte(double value, double gamma);

}  // namespace color

#endif  // SOA_COLOR_HPP
