#ifndef SOA_IMAGE_HPP
#define SOA_IMAGE_HPP

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

// Estructura para representar un color
struct RGBColor {
  double r, g, b;

  RGBColor(double red, double green, double blue) : r(red), g(green), b(blue) { }
};

// Tres arrays separados para R, G, B

class SOAImage {
public:
  SOAImage(int width, int height);

  // Establecemos el color de un píxel con corrección gamma
  void setPixel(int row, int col, RGBColor const & color, double gamma);
  // Escribimos la imagen en un archivo PPM
  void write_ppm(std::string const & filename) const;

  [[nodiscard]] int width() const { return width_; }

  [[nodiscard]] int height() const { return height_; }

  [[nodiscard]] size_t total_pixels() const {
    return static_cast<size_t>(width_) * static_cast<size_t>(height_);
  }

private:
  int width_;
  int height_;

  // Arrays separados para cada canal de color
  std::vector<uint8_t> red_channel_;
  std::vector<uint8_t> green_channel_;
  std::vector<uint8_t> blue_channel_;

  // Calculamos coordenadas (row, col)
  [[nodiscard]] size_t index(int row, int col) const {
    return static_cast<size_t>(row) * static_cast<size_t>(width_) + static_cast<size_t>(col);
  }

  // Convierte un valor de color en double [0,1] a uint8_t [0,255] con corrección gamma
  [[nodiscard]] static uint8_t to_byte(double value, double gamma);
};

#endif  // SOA_IMAGE_HPP
