// SOA entry point: read config and scene, render with SOA pipeline, write PPM

#include <cstddef>
#include <filesystem>
#include <iostream>
#include <span>
#include <string>
#include <vector>

#include "../../common/include/config.hpp"
#include "../../common/include/config_parser.hpp"
#include "../../common/include/materials.hpp"
#include "../../common/include/objects.hpp"
#include "../../common/include/scene_parser.hpp"

#include "../include/render_soa.hpp"
#include "../include/soa_camera.hpp"
#include "../include/soa_image.hpp"

namespace {

  struct Args {
    std::string config_file;
    std::string scene_file;
    std::string output_file;
  };

  bool parse_args(std::span<char const *> args, Args & out) {
    if (args.size() == 4) {
      out.config_file = args[1];
      out.scene_file  = args[2];
      out.output_file = args[3];
      return true;
    }
    // Fallback: try default sample files
    out.config_file = "archivos_ejemplo/config4.cfg";
    out.scene_file  = "archivos_ejemplo/scene4.txt";
    out.output_file = "output_soa4.ppm";
    return true;
  }

  void print_usage(char const * prog) {
    auto name = std::filesystem::path(prog).filename().string();
    std::cout << "Uso: " << name << " <config_file> <scene_file> <output_ppm>\n";
    std::cout << "     Si no se proporcionan, se usan archivos por defecto en archivos_ejemplo/.\n";
  }

}  // namespace

namespace {

  inline bool parse_args_from_span(std::span<char const *> sp, Args & out) {
    return parse_args(sp, out);
  }

  int run(std::span<char const *> sp) {
    Args args{};
    if (!parse_args_from_span(sp, args)) {
      char const * prog = sp.empty() ? "program" : sp.front();
      print_usage(prog);
      return 1;
    }
    // 1) Cargar configuración
    ConfigParams cfg;
    if (!parse_config(args.config_file, cfg)) {
      std::cerr << "ERROR: fallo leyendo configuración: " << args.config_file << '\n';
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
    if (!parse_scene(args.scene_file, scene_out)) {
      std::cerr << "ERROR: fallo leyendo escena: " << args.scene_file << '\n';
      return 3;
    }

    // 3) Preparar cámara e imagen SOA
    soa::CameraSOA camera(cfg);
    int const width  = cfg.image_width;
    int const height = cfg.get_image_height();
    SOAImage image(width, height);

    // 4) Renderizar
    soa::render_scene(cfg, scene_out, camera, image);

    // 5) Guardar imagen
    image.write_ppm(args.output_file);
    std::cout << "Imagen escrita en: " << args.output_file << " (" << width << "x" << height
              << ")\n";
    return 0;
  }

}  // namespace

int main(int argc, char const * const argv[]) {
  auto sp =
      std::span<char const *>{const_cast<char const **>(argv), static_cast<std::size_t>(argc)};
  return run(sp);
}
