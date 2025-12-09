#include "../include/soa_image.hpp"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>

// Inicializamos los tres canales a 0
SOAImage::SOAImage(int width, int height) : width_(width), height_(height) {
  size_t const total = static_cast<size_t>(width) * static_cast<size_t>(height);
  red_channel_.resize(total, 0);
  green_channel_.resize(total, 0);
  blue_channel_.resize(total, 0);
}

// Ponemos el color de un píxel aplicando la corrección gamma
void SOAImage::setPixel(int row, int col, RGBColor const & color, double gamma) {
  size_t const idx    = index(row, col);
  red_channel_[idx]   = to_byte(color.r, gamma);
  green_channel_[idx] = to_byte(color.g, gamma);
  blue_channel_[idx]  = to_byte(color.b, gamma);
}

// Convertimos el valor [0, 1] a [0, 255] con corrección gamma
uint8_t SOAImage::to_byte(double value, double gamma) {
  // Primero cambiamos al rango
  double const clamped = std::clamp(value, 0.0, 1.0);

  // Aplicamos corrección
  double const correc = std::pow(clamped, 1.0 / gamma);

  // Convertimos a byte
  return static_cast<uint8_t>(correc * 255.0);
}

// Escribimos la imagen en un archivo PPM
void SOAImage::write_ppm(std::string const & filename) const {
  std::ofstream file(filename, std::ios::binary);
  if (!file.is_open()) {
    std::cerr << "Error: Cannot create output file " << filename << '\n';
    return;
  }

  file << "P6\n" << width_ << " " << height_ << "\n255\n";

  for (size_t i = 0; i < total_pixels(); ++i) {
    file.put(static_cast<char>(red_channel_[i]));
    file.put(static_cast<char>(green_channel_[i]));
    file.put(static_cast<char>(blue_channel_[i]));
  }

  file.close();
}
