// AOS entry point: read config and scene, render with AOS pipeline, write PPM

#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include "../../common/include/config.hpp"
#include "../../common/include/config_parser.hpp"
#include "../../common/include/materials.hpp"
#include "../../common/include/objects.hpp"
#include "../../common/include/scene_parser.hpp"

#include "../include/aos_camera.hpp"
#include "../include/aos_image.hpp"
#include "../include/render_aos.hpp"

namespace {

  struct Args {
    std::string config_file;
    std::string scene_file;
    std::string output_file;
  };

  bool parse_args(std::vector<std::string> const & argv, Args & out) {
    if (argv.size() == 4) {
      out.config_file = argv[1];
      out.scene_file  = argv[2];
      out.output_file = argv[3];
      return true;
    }
    // Fallback: try default sample files
    out.config_file = "archivos_ejemplo/config4.cfg";
    out.scene_file  = "archivos_ejemplo/scene4.txt";
    out.output_file = "output_aos_4.ppm";
    return true;
  }

  void print_usage(char const * prog) {
    auto name = std::filesystem::path(prog).filename().string();
    std::cout << "Uso: " << name << " <config_file> <scene_file> <output_ppm>\n";
    std::cout << "     Si no se proporcionan, se usan archivos por defecto en archivos_ejemplo/.\n";
  }

}  // namespace

namespace {

  inline bool parse_args_from_argv(std::vector<std::string> const & argv, Args & out) {
    return parse_args(argv, out);
  }

  int run(std::vector<std::string> const & args) {
    Args args_obj{};
    if (!parse_args_from_argv(args, args_obj)) {
      char const * prog = (args.empty()) ? "program" : args[0].c_str();
      print_usage(prog);
      return 1;
    }
    // 1) Cargar configuración
    ConfigParams cfg;
    if (!parse_config(args_obj.config_file, cfg)) {
      std::cerr << "ERROR: fallo leyendo configuración: " << args_obj.config_file << '\n';
      return 2;
    }
    // 2) Cargar escena
    std::vector<MatteMaterial> matte_materials;
    std::vector<MetalMaterial> metal_materials;
    std::vector<RefractiveMaterial> refractive_materials;
    std::vector<Sphere> spheres;
    std::vector<Cylinder> cylinders;
    SceneOutput scene_out{matte_materials, metal_materials, refractive_materials, spheres,
                          cylinders};
    if (!parse_scene(args_obj.scene_file, scene_out)) {
      std::cerr << "ERROR: fallo leyendo escena: " << args_obj.scene_file << '\n';
      return 3;
    }

    // 3) Preparar cámara e imagen AOS
    aos::CameraAOS camera(cfg);
    int const width  = cfg.image_width;
    int const height = cfg.get_image_height();
    aos::AOSImage image(width, height);  // ← CORREGIDO: añadir 'aos::'

    // 4) Renderizar
    aos::render_scene(cfg, scene_out, camera, image);

    // 5) Guardar imagen
    image.write_ppm(args_obj.output_file);
    std::cout << "Imagen escrita en: " << args_obj.output_file << " (" << width << "x" << height
              << ")\n";
    return 0;
  }

}  // namespace

int main(int argc, char const * const argv[]) {
  std::vector<std::string> const args(argv, argv + argc);
  return run(args);
}
