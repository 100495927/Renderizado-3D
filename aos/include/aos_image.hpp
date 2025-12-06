#ifndef AOS_IMAGE_HPP
#define AOS_IMAGE_HPP

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace aos {

  // Estructura para representar un color
  struct RGBColor {
    double r, g, b;

    RGBColor(double red, double green, double blue) : r(red), g(green), b(blue) { }
  };

  // Estructura para representar un píxel en formato AOS
  struct Pixel {
    uint8_t r, g, b;

    Pixel() : r(0), g(0), b(0) { }

    Pixel(uint8_t red, uint8_t green, uint8_t blue) : r(red), g(green), b(blue) { }
  };

  // Un único array de estructuras Pixel

  class AOSImage {
  public:
    AOSImage(int width, int height);

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

    // Único array de estructuras Pixel
    std::vector<Pixel> pixels_;

    // Calculamos coordenadas (row, col)
    [[nodiscard]] size_t index(int row, int col) const {
      return static_cast<size_t>(row) * static_cast<size_t>(width_) + static_cast<size_t>(col);
    }

    // Convierte un valor de color en double [0,1] a uint8_t [0,255] con corrección gamma
    [[nodiscard]] static uint8_t to_byte(double value, double gamma);
  };

}  // namespace aos
#endif  // AOS_IMAGE_HPP
