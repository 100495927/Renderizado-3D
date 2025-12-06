#include <cstddef>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include "../include/config.hpp"
#include "../include/config_parser.hpp"
#include "../include/materials.hpp"
#include "../include/objects.hpp"
#include "../include/scene_parser.hpp"

namespace {

  void test_config_parser(std::string const & filename) {
    std::cout << "\n=== PROBANDO CONFIG PARSER ===" << '\n';
    std::cout << "Archivo: " << filename << '\n';

    ConfigParams config;
    bool const success = parse_config(filename, config);

    if (success) {
      std::cout << "✅ ÉXITO - Archivo parseado correctamente" << '\n';
      std::cout << "Valores parseados:" << '\n';
      std::cout << "  - image_width: " << config.image_width << '\n';
      std::cout << "  - gamma: " << config.gamma << '\n';
      std::cout << "  - camera: (" << config.camera_x << "," << config.camera_y << ","
                << config.camera_z << ")" << '\n';
      std::cout << "  - altura calculada: " << config.get_image_height() << '\n';
      std::cout << "  - aspect_ratio: " << config.aspect_width << "x" << config.aspect_height
                << '\n';
      std::cout << "  - field_of_view: " << config.field_of_view << '\n';
      std::cout << "  - samples_per_pixel: " << config.samples_per_pixel << '\n';
    } else {
      std::cout << "❌ ERROR - Falló el parsing" << '\n';
    }
  }

  /*void test_scene_parser(std::string const & filename) {
    std::cout << "\n=== PROBANDO SCENE PARSER ===" << '\n';
    std::cout << "Archivo: " << filename << '\n';

    std::vector<MatteMaterial> mate_materials;
    std::vector<MetalMaterial> metal_materials;
    std::vector<RefractiveMaterial> refractive_materials;
    std::vector<Sphere> spheres;
    std::vector<Cylinder> cylinders;

    bool success = parse_scene(filename, mate_materials, metal_materials, refractive_materials,
                               spheres, cylinders);

    if (success) {
      std::cout << "✅ ÉXITO - Archivo parseado correctamente" << '\n';
      std::cout << "Materiales encontrados:" << '\n';
      std::cout << "  - Mates: " << mate_materials.size() << '\n';
      std::cout << "  - Metálicos: " << metal_materials.size() << '\n';
      std::cout << "  - Refractivos: " << refractive_materials.size() << '\n';
      std::cout << "Objetos encontrados:" << '\n';
      std::cout << "  - Esferas: " << spheres.size() << '\n';
      std::cout << "  - Cilindros: " << cylinders.size() << '\n';

      // Mostrar detalles
      for (auto const & mat : mate_materials) {
        std::cout << "    Mate '" << mat.name << "': RGB(" << mat.reflectance_r << ","
                  << mat.reflectance_g << "," << mat.reflectance_b << ")" << '\n';
      }
      for (auto const & sph : spheres) {
        std::cout << "    Esfera centro(" << sph.center_x << "," << sph.center_y << ","
                  << sph.center_z << ") radio=" << sph.radius << " material=" << sph.material_name
                  << '\n';
      }
    } else {
      std::cout << "❌ ERROR - Falló el parsing" << '\n';
    }
  }*/

  void test_scene_parser(std::string const & filename) {
    std::cout << "\n=== PROBANDO SCENE PARSER ===" << '\n';
    std::cout << "Archivo: " << filename << '\n';

    std::vector<MatteMaterial> mate_materials;
    std::vector<MetalMaterial> metal_materials;
    std::vector<RefractiveMaterial> refractive_materials;
    std::vector<Sphere> spheres;
    std::vector<Cylinder> cylinders;

    // Cambia esta línea: usa SceneOutput en lugar de 6 parámetros
    SceneOutput out{mate_materials, metal_materials, refractive_materials, spheres, cylinders};
    bool const success = parse_scene(filename, out);

    if (success) {
      std::cout << "✅ ÉXITO - Archivo parseado correctamente" << '\n';
      std::cout << "Materiales encontrados:" << '\n';
      std::cout << "  - Mates: " << mate_materials.size() << '\n';
      std::cout << "  - Metálicos: " << metal_materials.size() << '\n';
      std::cout << "  - Refractivos: " << refractive_materials.size() << '\n';
      std::cout << "Objetos encontrados:" << '\n';
      std::cout << "  - Esferas: " << spheres.size() << '\n';
      std::cout << "  - Cilindros: " << cylinders.size() << '\n';

      // Mostrar detalles
      for (auto const & mat : mate_materials) {
        std::cout << "    Mate '" << mat.name << "': RGB(" << mat.reflectance_r << ","
                  << mat.reflectance_g << "," << mat.reflectance_b << ")" << '\n';
      }
      for (auto const & sph : spheres) {
        std::cout << "    Esfera centro(" << sph.center_x << "," << sph.center_y << ","
                  << sph.center_z << ") radio=" << sph.radius << " material=" << sph.material_name
                  << '\n';
      }
    } else {
      std::cout << "❌ ERROR - Falló el parsing" << '\n';
    }
  }

}  // namespace

int main(int argc, char * argv[]) {
  std::vector<std::string> args;
  args.reserve(static_cast<size_t>(argc));

  args.assign(argv, argv + argc);

  if (args.size() != 3) {
    std::string prog = "program";
    if (!args.empty() and !args[0].empty()) {
      prog = std::filesystem::path(args[0]).filename().string();
    }
    std::cout << "Uso: " << prog << " <config_file> <scene_file>" << '\n';
    return 1;
  }

  std::string const & config_file = args[1];
  std::string const & scene_file  = args[2];

  test_config_parser(config_file);
  test_scene_parser(scene_file);

  return 0;
}
