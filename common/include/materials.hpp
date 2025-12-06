#ifndef MATERIALS_HPP
#define MATERIALS_HPP

#include "material_base.hpp"
#include <string>

// Material mate - reflectancia RGB[2]
struct MatteMaterial : public render::MaterialBase {
  render::MaterialType type = render::MATTE_TYPE;
  std::string name;
  double reflectance_r{0.0}, reflectance_g{0.0}, reflectance_b{0.0};
};

// Material metálico - reflectancia + difusión [2]
struct MetalMaterial : public render::MaterialBase {
  render::MaterialType type = render::METAL_TYPE;
  std::string name;
  double reflectance_r{0.0}, reflectance_g{0.0}, reflectance_b{0.0};
  double diffusion{0.0};
};

// Material refractivo - índice de refracción[2]
struct RefractiveMaterial : public render::MaterialBase {
  render::MaterialType type = render::REFRACTIVE_TYPE;
  std::string name;
  double refractive_index{1.0};
};

#endif  // MATERIALS_HPP
