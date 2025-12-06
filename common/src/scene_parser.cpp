/**/
/**/
#include "../include/scene_parser.hpp"
#include "../include/materials.hpp"
#include "../include/objects.hpp"
#include "../include/parser_utilities.hpp"
#include <exception>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_set>
#include <vector>

namespace {

  // Estructura para pasar los vectores como un solo parámetro
  struct SceneData {
    std::vector<MatteMaterial> * matte_materials;
    std::vector<MetalMaterial> * metal_materials;
    std::vector<RefractiveMaterial> * refractive_materials;
    std::vector<Sphere> * spheres;
    std::vector<Cylinder> * cylinders;
    std::unordered_set<std::string> * material_names;
  };

  // Helper para matte - SIN código duplicado
  bool parse_matte(std::vector<std::string> const & tokens, SceneData & data) {
    if (tokens.size() < 5) {
      std::cerr << "Error: Invalid matte material parameters" << '\n';
      return false;
    }

    std::string const & name = tokens[1];

    double const r = std::stod(tokens[2]);
    double const g = std::stod(tokens[3]);
    double const b = std::stod(tokens[4]);

    if (r < 0 or r > 1 or g < 0 or g > 1 or b < 0 or b > 1) {
      std::cerr << "Error: Invalid matte material parameters" << '\n';
      return false;
    }

    if (data.material_names->contains(name)) {
      std::cerr << "Error: Material with name " << name << " already exists" << '\n';
      return false;
    }

    MatteMaterial m{};
    m.name          = name;
    m.reflectance_r = r;
    m.reflectance_g = g;
    m.reflectance_b = b;

    // SOLO una línea - sin duplicado
    data.matte_materials->push_back(m);
    data.material_names->insert(name);
    return true;
  }

  // Helper para metal - SIN código duplicado
  bool parse_metal(std::vector<std::string> const & tokens, SceneData & data) {
    if (tokens.size() < 6) {
      std::cerr << "Error: Invalid matte material parameters" << '\n';
      return false;
    }

    std::string const & name = tokens[1];

    double const r         = std::stod(tokens[2]);
    double const g         = std::stod(tokens[3]);
    double const b         = std::stod(tokens[4]);
    double const diffusion = std::stod(tokens[5]);

    if (r < 0 or r > 1 or g < 0 or g > 1 or b < 0 or b > 1 or diffusion <= 0) {
      std::cerr << "Error: Invalid metal material parameters" << '\n';
      return false;
    }

    if (data.material_names->contains(name)) {
      std::cerr << "Error: Material with name " << name << " already exists" << '\n';
      return false;
    }

    MetalMaterial mm{};
    mm.name          = name;
    mm.reflectance_r = r;
    mm.reflectance_g = g;
    mm.reflectance_b = b;
    mm.diffusion     = diffusion;

    // SOLO una línea - sin duplicado
    data.metal_materials->push_back(mm);
    data.material_names->insert(name);
    return true;
  }

  // Helper para refractive - SIN código duplicado
  bool parse_refractive(std::vector<std::string> const & tokens, SceneData & data) {
    if (tokens.size() < 3) {
      std::cerr << "Error: Invalid refractive material parameters" << '\n';
      return false;
    }

    std::string const & name = tokens[1];

    double const refraction = std::stod(tokens[2]);
    if (refraction <= 0) {
      std::cerr << "Error: Invalid refractive material parameters" << '\n';
      return false;
    }

    if (data.material_names->contains(name)) {
      std::cerr << "Error: Material with name " << name << " already exists" << '\n';
      return false;
    }

    RefractiveMaterial rm{};
    rm.name             = name;
    rm.refractive_index = refraction;

    // SOLO una línea - sin duplicado
    data.refractive_materials->push_back(rm);
    data.material_names->insert(name);
    return true;
  }

  // Helper para sphere - SIN código duplicado
  bool parse_sphere(std::vector<std::string> const & tokens, SceneData & data) {
    if (tokens.size() < 6) {
      std::cerr << "Error: Invalid sphere parameters" << '\n';
      return false;
    }

    double const x               = std::stod(tokens[1]);
    double const y               = std::stod(tokens[2]);
    double const z               = std::stod(tokens[3]);
    double const radius          = std::stod(tokens[4]);
    std::string const & material = tokens[5];

    if (radius <= 0) {
      std::cerr << "Error: Invalid sphere parameters" << '\n';
      return false;
    }

    if (!data.material_names->contains(material)) {
      std::cerr << "Error: Material not found " << material << '\n';
      return false;
    }

    Sphere s{};
    s.center_x      = x;
    s.center_y      = y;
    s.center_z      = z;
    s.radius        = radius;
    s.material_name = material;

    // SOLO una línea - sin duplicado
    data.spheres->push_back(s);
    return true;
  }

  // Helper para cylinder - SIN código duplicado
  bool parse_cylinder(std::vector<std::string> const & tokens, SceneData & data) {
    if (tokens.size() < 9) {
      std::cerr << "Error: Invalid cylinder parameters" << '\n';
      return false;
    }

    double const x               = std::stod(tokens[1]);
    double const y               = std::stod(tokens[2]);
    double const z               = std::stod(tokens[3]);
    double const radius          = std::stod(tokens[4]);
    double const ax              = std::stod(tokens[5]);
    double const ay              = std::stod(tokens[6]);
    double const az              = std::stod(tokens[7]);
    std::string const & material = tokens[8];

    if (radius <= 0) {
      std::cerr << "Error: Invalid cylinder parameters" << '\n';
      return false;
    }

    if (!data.material_names->contains(material)) {
      std::cerr << "Error: Material not found " << material << '\n';
      return false;
    }

    Cylinder c{};
    c.center_x      = x;
    c.center_y      = y;
    c.center_z      = z;
    c.radius        = radius;
    c.axis_x        = ax;
    c.axis_y        = ay;
    c.axis_z        = az;
    c.material_name = material;

    // SOLO una línea - sin duplicado
    data.cylinders->push_back(c);
    return true;
  }

  // Extraer el dispatcher a una función separada

  bool dispatch_entity(std::string const & type, std::vector<std::string> const & tokens,
                       SceneData & data) {
    if (type == "matte") {
      return parse_matte(tokens, data);
    }
    if (type == "metal") {
      return parse_metal(tokens, data);
    }
    if (type == "refractive") {
      return parse_refractive(tokens, data);
    }
    if (type == "sphere") {
      return parse_sphere(tokens, data);
    }
    if (type == "cylinder") {
      return parse_cylinder(tokens, data);
    }

    std::cerr << "Unknown scene entity " << type << '\n';
    return false;
  }

  // Función principal más pequeña
  bool parse_scene_impl(std::string const & filename, SceneData & data) {
    std::ifstream file(filename);
    if (!file.is_open()) {
      std::cerr << "Cannot open scene file: " << filename << '\n';
      return false;
    }

    std::string line;
    while (std::getline(file, line)) {
      if (is_empty_line(line)) {
        continue;
      }

      auto tokens = split_line(line);
      if (tokens.empty()) {
        continue;
      }

      std::string const & type_with_colon = tokens[0];
      if (type_with_colon.empty() or type_with_colon.back() != ':') {
        std::cerr << "Unknown scene entity " << type_with_colon << '\n';
        return false;
      }

      std::string const type = type_with_colon.substr(0, type_with_colon.size() - 1);

      try {
        if (!dispatch_entity(type, tokens, data)) {
          return false;
        }
      } catch (std::exception const & e) {
        std::cerr << "Exception parsing " << type << ": " << e.what() << "\nLine: " << line << '\n';
        return false;
      }
    }
    return true;
  }

}  // namespace

bool parse_scene(std::string const & filename, SceneOutput & out) {
  std::unordered_set<std::string> material_names;

  SceneData data{
    .matte_materials      = &out.matte_materials.get(),
    .metal_materials      = &out.metal_materials.get(),
    .refractive_materials = &out.refractive_materials.get(),
    .spheres              = &out.spheres.get(),
    .cylinders            = &out.cylinders.get(),
    .material_names       = &material_names,
  };

  return parse_scene_impl(filename, data);
}
