// SOA entry point: read config and scene, render with SOA pipeline, write PPM

#include <cstddef>
#include <iostream>
#include <span>
#include <string>
#include <vector>

#include <oneapi/tbb/global_control.h>

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
    int threads             = 1;
    std::string partitioner = "auto";
    int grain_rows          = 1;
    int grain_cols          = 1;
  };

  bool parse_args(std::span<char *> args, Args & out) {
    if (args.size() == 8) {
      out.config_file = args[1];
      out.scene_file  = args[2];
      out.output_file = args[3];
      out.threads     = std::stoi(args[4]);
      out.partitioner = args[5];
      out.grain_rows  = std::stoi(args[6]);
      out.grain_cols  = std::stoi(args[7]);
      return true;
    }
    // Fallback: try default sample files
    out.config_file = "archivos_ejemplo/config4.cfg";
    out.scene_file  = "archivos_ejemplo/scene4.txt";
    out.output_file = "output_soa4.ppm";
    return true;
  }

}  // namespace

int main(int argc, char ** argv) {
  Args args{};
  auto sp = std::span<char *>{argv, static_cast<std::size_t>(argc)};
  if (!parse_args(sp, args)) {
    return 1;
  }
  std::size_t const threads =
      args.threads > 0 ? static_cast<std::size_t>(args.threads) : std::size_t{1};
  tbb::global_control const gc(tbb::global_control::max_allowed_parallelism, threads);
  ConfigParams cfg;
  if (!parse_config(args.config_file, cfg)) {
    std::cerr << "ERROR leyendo configuración\n";
    return 2;
  }
  std::vector<MatteMaterial> matte_materials;
  std::vector<MetalMaterial> metal_materials;
  std::vector<RefractiveMaterial> refractive_materials;
  std::vector<Sphere> spheres;
  std::vector<Cylinder> cylinders;
  SceneOutput scene_out{matte_materials, metal_materials, refractive_materials, spheres, cylinders};
  if (!parse_scene(args.scene_file, scene_out)) {
    std::cerr << "ERROR leyendo escena\n";
    return 3;
  }
  soa::CameraSOA camera(cfg);
  SOAImage image(cfg.image_width, cfg.get_image_height());
  // 4) Render
  soa::RenderOptions ro;
  ro.partitioner = args.partitioner;
  ro.grain_rows  = args.grain_rows;
  ro.grain_cols  = args.grain_cols;
  std::cout << "[MAIN] RenderOptions:\n"
            << "  partitioner = " << ro.partitioner << '\n'
            << "  grain_rows  = " << ro.grain_rows << '\n'
            << "  grain_cols  = " << ro.grain_cols << '\n';
  soa::set_render_options(ro);
  soa::render_scene(cfg, scene_out, camera, image);
  // 5) Guardar imagen
  image.write_ppm(args.output_file);
  return 0;
}
