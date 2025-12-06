#include "../include/aos_image.hpp"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>

namespace aos {

  AOSImage::AOSImage(int width, int height) : width_(width), height_(height) {
    size_t const total = static_cast<size_t>(width) * static_cast<size_t>(height);
    pixels_.resize(total, Pixel());
  }

  // Cambiar el orden de las coordenadas
  void AOSImage::setPixel(int row, int col, RGBColor const & color, double gamma) {
    // Invertir la coordenada Y para que la imagen no salga al revés
    int const actual_row = height_ - 1 - row;

    size_t const idx = index(actual_row, col);  // Usar fila invertida
    pixels_[idx].r   = to_byte(color.r, gamma);
    pixels_[idx].g   = to_byte(color.g, gamma);
    pixels_[idx].b   = to_byte(color.b, gamma);
  }

  uint8_t AOSImage::to_byte(double value, double gamma) {
    double const clamped   = std::clamp(value, 0.0, 1.0);
    double const corrected = std::pow(clamped, 1.0 / gamma);
    return static_cast<uint8_t>(corrected * 255.0);
  }

  void AOSImage::write_ppm(std::string const & filename) const {
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
      std::cerr << "Error: Cannot create output file " << filename << '\n';
      return;
    }

    file << "P6\n" << width_ << " " << height_ << "\n255\n";

    for (size_t i = 0; i < total_pixels(); ++i) {
      file.put(static_cast<char>(pixels_[i].r));
      file.put(static_cast<char>(pixels_[i].g));
      file.put(static_cast<char>(pixels_[i].b));
    }

    file.close();
  }

}  // namespace aos
