#ifndef AOS_CAMERA_HPP
#define AOS_CAMERA_HPP

#include "../../common/include/config.hpp"
#include <cmath>
#include <cstddef>
#include <random>
#include <vector>

namespace aos {

  class CameraAOS {
  public:
    explicit CameraAOS(ConfigParams const & config);

    // API moderna y eficiente
    void begin_frame(std::size_t width, std::size_t height);

    // Estructura para rayos (aos)
    struct RaySample {
      double origin_x, origin_y, origin_z;
      double dir_x, dir_y, dir_z;
    };

    struct RayStruct {
      double origin_x, origin_y, origin_z;
      double dir_x, dir_y, dir_z;
    };

    // Generar un solo rayo para un píxel específico
    RaySample make_ray(std::size_t i, std::size_t j, std::mt19937 & rng) const;

    // Generar todos los rayos para un píxel específico
    void generate_pixel_rays(std::size_t i, std::size_t j, std::size_t spp,
                             std::vector<RaySample> & pixel_rays) const;

    // Método legacy que pre-genera todos los rayos (para compatibilidad)
    void generate_primary_rays(std::size_t width, std::size_t height, std::size_t spp);

    // Datos públicos para acceso directo (filosofía AOS)
    std::vector<RayStruct> rays;

  private:
    // Parámetros de la cámara
    double OX{}, OY{}, OZ{};  // Posición
    double UX{}, UY{}, UZ{};  // Vector horizontal
    double VX{}, VY{}, VZ{};  // Vector vertical
    double WX{}, WY{}, WZ{};  // Vector de vista
    double DF{}, FOV{};       // Distancia focal y campo de visión
    int ray_seed_{};          // Semilla para RNG

    // Valores derivados por frame
    double p0x_{}, p0y_{}, p0z_{};                    // Origen de la ventana
    double delta_x_x_{}, delta_x_y_{}, delta_x_z_{};  // Incremento horizontal por píxel
    double delta_y_x_{}, delta_y_y_{}, delta_y_z_{};  // Incremento vertical por píxel
  };

}  // namespace aos

#endif
