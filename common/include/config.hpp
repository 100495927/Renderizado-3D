#ifndef CONFIG_HPP
#define CONFIG_HPP

// Definimos la estructura para todos los parámetros de configuración del proyecto[2]
struct ConfigParams {
  int aspect_width  = 16;
  int aspect_height = 9;
  int image_width   = 1'920;
  double gamma      = 2.2;
  double camera_x = 0.0, camera_y = 0.0, camera_z = -10.0;
  double target_x = 0.0, target_y = 0.0, target_z = 0.0;
  double north_x = 0.0, north_y = 1.0, north_z = 0.0;
  double field_of_view           = 90.0;
  int samples_per_pixel          = 20;
  int max_depth                  = 5;
  int material_rng_seed          = 13;
  int ray_rng_seed               = 19;
  double background_dark_color_r = 0.25, background_dark_color_g = 0.5, background_dark_color_b = 1;
  double background_light_color_r = 1, background_light_color_g = 1, background_light_color_b = 1;

  [[nodiscard]] int get_image_height() const {
    double const aspect_ratio = static_cast<double>(aspect_width) / aspect_height;
    return static_cast<int>(image_width / aspect_ratio);
  }
};

#endif  // CONFIG_HPP
