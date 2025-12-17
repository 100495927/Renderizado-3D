#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <optional>
#include <random>
#include <string>
#include <thread>
#include <vector>

#include <oneapi/tbb/blocked_range2d.h>
#include <oneapi/tbb/enumerable_thread_specific.h>
#include <oneapi/tbb/parallel_for.h>
#include <oneapi/tbb/partitioner.h>
#include <oneapi/tbb/task_arena.h>

#include "../../common/include/config.hpp"
#include "../../common/include/scene_parser.hpp"
#include "../../common/include/vector.hpp"
#include "../include/render_soa.hpp"
#include "../include/soa_camera.hpp"
#include "../include/soa_color.hpp"
#include "../include/soa_image.hpp"
#include "../include/soa_ray.hpp"

#ifndef M_PI
 #define M_PI 3.14159265358979323846
#endif

namespace soa {

  namespace {

    // ==================== GENERADORES THREAD-LOCAL ====================
    // Variable global thread-local para RNG de materiales
    /// Función singleton para acceder a RNG de materiales
    tbb::enumerable_thread_specific<std::mt19937_64> & get_material_rng_per_thread() {
      static tbb::enumerable_thread_specific<std::mt19937_64> g_material_rng_per_thread{};
      return g_material_rng_per_thread;
    }

    /// Función singleton para acceder a RNG de rayos primarios
    tbb::enumerable_thread_specific<std::mt19937_64> & get_ray_rng_per_thread() {
      static tbb::enumerable_thread_specific<std::mt19937_64> g_ray_rng_per_thread{};
      return g_ray_rng_per_thread;
    }

    void initialize_material_thread_local_rngs(std::uint64_t base_seed) {
      // Determinar el número de hilos
      auto num_threads = static_cast<size_t>(tbb::this_task_arena::max_concurrency());
      if (num_threads == 0) {
        num_threads = std::thread::hardware_concurrency();
      }
      if (num_threads == 0) {
        num_threads = 1;
      }

      // Generar semillas únicas para cada hilo
      std::vector<std::uint64_t> seeds_material(num_threads);
      std::mt19937_64 const seed_gen_material{base_seed + 12'345ULL};
      std::ranges::generate(seeds_material, seed_gen_material);

      // Indicar generadores locales a cada hilo
      get_material_rng_per_thread() = tbb::enumerable_thread_specific<std::mt19937_64>{
        [seeds_material_copy = seeds_material]() mutable {
          static std::atomic<std::size_t> thread_counter{0};
          auto thread_id = thread_counter.fetch_add(1, std::memory_order_relaxed);
          return std::mt19937_64{seeds_material_copy[thread_id % seeds_material_copy.size()]};
        }};
    }

    void initialize_ray_thread_local_rngs(std::uint64_t base_seed) {
      // Determinar el número de hilos
      auto num_threads = static_cast<size_t>(tbb::this_task_arena::max_concurrency());
      if (num_threads == 0) {
        num_threads = std::thread::hardware_concurrency();
      }
      if (num_threads == 0) {
        num_threads = 1;
      }

      // Generar semillas únicas para cada hilo
      std::vector<std::uint64_t> seeds_ray(num_threads);
      std::mt19937_64 const seed_gen_ray{base_seed};
      std::ranges::generate(seeds_ray, seed_gen_ray);

      // Indicar generadores locales a cada hilo
      get_ray_rng_per_thread() =
          tbb::enumerable_thread_specific<std::mt19937_64>{[seeds_ray_copy = seeds_ray]() mutable {
            static std::atomic<std::size_t> thread_counter{0};
            auto thread_id = thread_counter.fetch_add(1, std::memory_order_relaxed);
            return std::mt19937_64{seeds_ray_copy[thread_id % seeds_ray_copy.size()]};
          }};
    }

    // ==================== ESTRUCTURAS ====================

    struct MaterialInfo {
      enum Type { MATTE, METAL, REFRACTIVE } type;

      double r;
      double g;
      double b;
      double roughness;
      double refraction;
    };

    struct HitInfo {
      bool hit{false};
      render::vector point;
      render::vector normal;
      double t{0};
      MaterialInfo material;
      std::string material_name;
    };

    // STRUCT PARA REDUCIR PARÁMETROS
    struct RayTracingContext {
      int depth{};
      ConfigParams const * cfg{};
      SceneOutput const * scene{};
      render::vector ray_direction;
      int internal_reflections{0};
    };

    // ==================== BÚSQUEDA DE MATERIAL ====================

    MaterialInfo get_material_matte(std::string const & name, SceneOutput const & scene) {
      for (auto const & m : scene.matte_materials.get()) {
        if (m.name == name) {
          MaterialInfo info{};
          info.type       = MaterialInfo::MATTE;
          info.r          = m.reflectance_r;
          info.g          = m.reflectance_g;
          info.b          = m.reflectance_b;
          info.roughness  = 0.0;
          info.refraction = 0.0;
          return info;
        }
      }
      MaterialInfo def{};
      def.type = MaterialInfo::MATTE;
      def.r    = 0.8;
      def.g    = 0.8;
      def.b    = 0.8;
      return def;
    }

    MaterialInfo get_material_metal(std::string const & name, SceneOutput const & scene) {
      for (auto const & m : scene.metal_materials.get()) {
        if (m.name == name) {
          MaterialInfo info{};
          info.type       = MaterialInfo::METAL;
          info.r          = m.reflectance_r;
          info.g          = m.reflectance_g;
          info.b          = m.reflectance_b;
          info.roughness  = m.diffusion;
          info.refraction = 0.0;
          return info;
        }
      }
      MaterialInfo def{};
      def.type      = MaterialInfo::METAL;
      def.r         = 0.8;
      def.g         = 0.8;
      def.b         = 0.8;
      def.roughness = 0.0;
      return def;
    }

    MaterialInfo get_material_refractive(std::string const & name, SceneOutput const & scene) {
      for (auto const & m : scene.refractive_materials.get()) {
        if (m.name == name) {
          MaterialInfo info{};
          info.type       = MaterialInfo::REFRACTIVE;
          info.r          = 1.0;
          info.g          = 1.0;
          info.b          = 1.0;
          info.roughness  = 0.0;
          info.refraction = m.refractive_index;
          return info;
        }
      }
      MaterialInfo def{};
      def.type       = MaterialInfo::REFRACTIVE;
      def.r          = 1.0;
      def.g          = 1.0;
      def.b          = 1.0;
      def.refraction = 1.5;
      return def;
    }

    MaterialInfo find_material(std::string const & name, SceneOutput const & scene) {
      for (auto const & m : scene.refractive_materials.get()) {
        if (m.name == name) {
          return get_material_refractive(name, scene);
        }
      }

      for (auto const & m : scene.metal_materials.get()) {
        if (m.name == name) {
          return get_material_metal(name, scene);
        }
      }

      for (auto const & m : scene.matte_materials.get()) {
        if (m.name == name) {
          return get_material_matte(name, scene);
        }
      }

      MaterialInfo def{};
      def.type = MaterialInfo::MATTE;
      def.r    = 0.5;
      def.g    = 0.5;
      def.b    = 0.5;
      return def;
    }

    // ==================== HELPERS ====================

    color::Color background_ray(ray::Ray const & r, ConfigParams const & cfg) {
      render::vector const dir_unit = r.direction / r.direction.magnitude();
      double const dir_y            = -dir_unit.get_y();
      color::Color const light{cfg.background_light_color_r, cfg.background_light_color_g,
                               cfg.background_light_color_b};
      color::Color const dark{cfg.background_dark_color_r, cfg.background_dark_color_g,
                              cfg.background_dark_color_b};
      return color::background_color(dir_y, light, dark);
    }

    // ==================== FORWARD DECLARATIONS ====================

    color::Color trace_ray(ray::Ray const & r, RayTracingContext const & ctx);

    color::Color process_metal(ray::Ray const & r, HitInfo const & hit,
                               RayTracingContext const & ctx);
    color::Color process_matte(HitInfo const & hit, RayTracingContext const & ctx);
    color::Color process_refractive(ray::Ray const & r, HitInfo const & hit,
                                    RayTracingContext const & ctx);

    HitInfo check_spheres(ray::Ray const & r, SceneOutput const & scene) {
      HitInfo hit{};
      ray::IntersectionParams params{ray::MIN_DISTANCE, 1e18};
      for (auto const & sph : scene.spheres.get()) {
        auto center = render::vector{sph.center_x, sph.center_y, sph.center_z};
        auto rec    = ray::hit_sphere(r, center, sph.radius, params);
        if (rec) {
          if (!hit.hit or rec->t < hit.t) {
            hit.hit           = true;
            hit.t             = rec->t;
            hit.point         = rec->point;
            hit.normal        = rec->normal;
            hit.material_name = sph.material_name;
            params.t_max      = rec->t;
          }
        }
      }
      return hit;
    }

    HitInfo check_cylinders(ray::Ray const & r, SceneOutput const & scene) {
      HitInfo hit{};
      ray::IntersectionParams params{ray::MIN_DISTANCE, 1e18};
      for (auto const & cyl : scene.cylinders.get()) {
        ray::CylinderParams cp{};
        cp.center = render::vector{cyl.center_x, cyl.center_y, cyl.center_z};
        cp.radius = cyl.radius;
        cp.axis   = render::vector{cyl.axis_x, cyl.axis_y, cyl.axis_z};
        cp.height = cyl.height;
        auto rec  = ray::hit_cylinder(r, cp, params);
        if (rec) {
          if (!hit.hit or rec->t < hit.t) {
            hit.hit           = true;
            hit.t             = rec->t;
            hit.point         = rec->point;
            hit.normal        = rec->normal;
            hit.material_name = cyl.material_name;
            params.t_max      = rec->t;
          }
        }
      }
      return hit;
    }

    HitInfo closest_intersection(ray::Ray const & r, SceneOutput const & scene) {
      auto sphere_hit   = check_spheres(r, scene);
      auto cylinder_hit = check_cylinders(r, scene);
      if (sphere_hit.hit and cylinder_hit.hit) {
        return sphere_hit.t < cylinder_hit.t ? sphere_hit : cylinder_hit;
      }
      return sphere_hit.hit ? sphere_hit : cylinder_hit;
    }

    color::Color trace_ray(ray::Ray const & r, RayTracingContext const & ctx) {
      if (ctx.depth <= 0) {
        return background_ray(r, *ctx.cfg);
      }

      HitInfo hit = closest_intersection(r, *ctx.scene);

      if (!hit.hit) {
        return background_ray(r, *ctx.cfg);
      }

      hit.material = find_material(hit.material_name, *ctx.scene);

      if (hit.material.type == MaterialInfo::METAL) {
        return process_metal(r, hit, ctx);
      }
      if (hit.material.type == MaterialInfo::MATTE) {
        return process_matte(hit, ctx);
      }
      if (hit.material.type == MaterialInfo::REFRACTIVE) {
        return process_refractive(r, hit, ctx);
      }

      return color::Color{1, 1, 1};
    }

    // ==================== MATERIALES CON RECURSIÓN ====================

    color::Color apply_material_color(color::Color const & reflected, MaterialInfo const & mat) {
      return color::Color{reflected.r * mat.r, reflected.g * mat.g, reflected.b * mat.b};
    }

    color::Color process_metal(ray::Ray const & r, HitInfo const & hit,
                               RayTracingContext const & ctx) {
      double const EPSILON     = 1e-4;
      double const dot         = render::dot(r.direction, hit.normal);
      render::vector reflected = r.direction - hit.normal * (2.0 * dot);

      // MEJORA: Aplicar roughness para difuminar el reflejo
      if (hit.material.roughness > 1e-8) {
        // Añadir perturbación aleatoria basada en roughness
        // Usar el generador local al hilo desde el contexto
        auto & local_rng = get_material_rng_per_thread().local();
        std::uniform_real_distribution<> dis(-1.0, 1.0);

        render::vector const perturbation{dis(local_rng) * hit.material.roughness,
                                          dis(local_rng) * hit.material.roughness,
                                          dis(local_rng) * hit.material.roughness};
        reflected = reflected + perturbation;
        reflected = reflected / reflected.magnitude();
      }

      ray::Ray const bounce_ray{hit.point + hit.normal * EPSILON, reflected};
      RayTracingContext const new_ctx{ctx.depth - 1, ctx.cfg, ctx.scene, reflected,
                                      ctx.internal_reflections};
      color::Color const reflected_color = trace_ray(bounce_ray, new_ctx);

      return apply_material_color(reflected_color, hit.material);
    }

    color::Color process_matte(HitInfo const & hit, RayTracingContext const & ctx) {
      double const EPSILON = 1e-4;

      // Usar thread_local para evitar carrera de datos
      auto & local_rng = get_material_rng_per_thread().local();
      std::uniform_real_distribution<> dis(0.0, 1.0);

      double const r1 = dis(local_rng);
      double const r2 = dis(local_rng);
      // Generar ángulos
      double const theta = std::acos(std::sqrt(r1));
      double const phi   = 2.0 * M_PI * r2;

      // Convertir a coordenadas usando la normal como base
      render::vector tangent =
          std::abs(hit.normal.get_x()) < 0.9 ? render::vector{1, 0, 0} : render::vector{0, 1, 0};
      render::vector const bitangent = render::cross(hit.normal, tangent);
      tangent                        = render::cross(bitangent, hit.normal);

      render::vector const reflected_dir = hit.normal * std::cos(theta) +
                                           tangent * std::sin(theta) * std::cos(phi) +
                                           bitangent * std::sin(theta) * std::sin(phi);

      ray::Ray const bounce_ray{hit.point + hit.normal * EPSILON, reflected_dir};
      RayTracingContext const new_ctx{ctx.depth - 1, ctx.cfg, ctx.scene, reflected_dir,
                                      ctx.internal_reflections};
      color::Color const reflected = trace_ray(bounce_ray, new_ctx);

      return apply_material_color(reflected, hit.material);
    }

    color::Color process_refractive(ray::Ray const & r, HitInfo const & hit,
                                    RayTracingContext const & ctx) {
      constexpr double EPSILON        = 1e-4;
      constexpr int MAX_INTERNAL_REFL = 4;
      if (ctx.internal_reflections >= MAX_INTERNAL_REFL) {
        return background_ray(r, *ctx.cfg);
      }
      double const n1         = 1.0;
      double const n2         = hit.material.refraction;
      double const dot_in     = render::dot(hit.normal, -r.direction);
      double const sin_theta1 = std::sqrt(std::max(0.0, 1.0 - dot_in * dot_in));
      double const sin_theta2 = (n1 / n2) * sin_theta1;
      if (sin_theta2 > 1.0) {
        double const dot               = render::dot(r.direction, hit.normal);
        render::vector const reflected = r.direction - hit.normal * (2.0 * dot);

        RayTracingContext const new_ctx{ctx.depth - 1, ctx.cfg, ctx.scene, reflected,
                                        ctx.internal_reflections + 1};

        ray::Ray const bounce_ray{hit.point + hit.normal * EPSILON, reflected};

        return trace_ray(bounce_ray, new_ctx);
      }
      double const cos_theta2 = std::sqrt(std::max(0.0, 1.0 - sin_theta2 * sin_theta2));

      render::vector const refracted =
          (r.direction * (n1 / n2)) + hit.normal * ((n1 / n2) * dot_in - cos_theta2);

      RayTracingContext const refr_ctx{ctx.depth - 1, ctx.cfg, ctx.scene, refracted, 0};

      ray::Ray const refracted_ray{hit.point + hit.normal * EPSILON, refracted};

      return trace_ray(refracted_ray, refr_ctx);
    }

    RenderOptions & get_global_render_options_internal() {
      static RenderOptions g_render_options{};
      return g_render_options;
    }

  }  // namespace

  // ============ VERSIÓN PARALELIZADA CON TBB ==============
  // Estructura para agrupar datos de configuración del render
  struct RenderParams {
    int w{};
    int h{};
    int spp{};
    int max_depth{};
    ConfigParams const * cfg{nullptr};
    SceneOutput const * scene{nullptr};
    CameraSOA * camera{nullptr};
    SOAImage * image{nullptr};
  };

  void set_render_options(RenderOptions const & opts) {
    get_global_render_options_internal() = opts;
  }

  RenderOptions const & get_render_options() {
    return get_global_render_options_internal();
  }

  namespace {  // Namespace anónimo para funciones internas

    // Función para procesar un bloque de píxeles
    void process_pixel_block(tbb::blocked_range2d<int> const & range, RenderParams const & params) {
      auto & local_ray_rng = get_ray_rng_per_thread().local();

      auto const & camera = *params.camera;
      auto const & cfg    = *params.cfg;
      auto const & scene  = *params.scene;
      auto & image        = *params.image;

      for (int j = range.rows().begin(); j < range.rows().end(); ++j) {
        for (int i = range.cols().begin(); i < range.cols().end(); ++i) {
          color::Color accum{0.0, 0.0, 0.0};

          for (int s = 0; s < params.spp; ++s) {
            auto rs = camera.make_ray(static_cast<std::size_t>(i), static_cast<std::size_t>(j),
                                      local_ray_rng);

            ray::Ray const r{
              render::vector{rs.ox, rs.oy, rs.oz},
              render::vector{rs.dx, rs.dy, rs.dz}
            };

            RayTracingContext const ctx{params.max_depth, &cfg, &scene, r.direction, 0};
            color::Color const c = trace_ray(r, ctx);
            accum                = accum + c;
          }

          double const inv_spp = 1.0 / static_cast<double>(params.spp);
          RGBColor const out_color{accum.r * inv_spp, accum.g * inv_spp, accum.b * inv_spp};

          image.setPixel(j, i, out_color, cfg.gamma);
        }
      }
    }

  }  // namespace

  // Función principal render_scene
  void render_scene(ConfigParams const & cfg, SceneOutput const & scene, CameraSOA & camera,
                    SOAImage & image) {
    auto const & opts               = get_render_options();
    std::string const & partitioner = opts.partitioner;
    int grain_rows                  = opts.grain_rows;
    int grain_cols                  = opts.grain_cols;
    // Inicializar generadores thread-local para materiales
    initialize_material_thread_local_rngs(static_cast<std::uint64_t>(cfg.material_rng_seed));
    initialize_ray_thread_local_rngs(static_cast<std::uint64_t>(cfg.ray_rng_seed));

    RenderParams params;
    params.w         = image.width();
    params.h         = image.height();
    params.spp       = std::max(1, cfg.samples_per_pixel);
    params.max_depth = std::max(1, cfg.max_depth);
    params.cfg       = &cfg;
    params.scene     = &scene;
    params.camera    = &camera;
    params.image     = &image;

    camera.begin_frame(static_cast<std::size_t>(params.w), static_cast<std::size_t>(params.h));

    grain_rows = std::max(1, std::min(grain_rows, params.h));
    grain_cols = std::max(1, std::min(grain_cols, params.w));

    std::cout << "[RENDER] Using:\n"
              << "  partitioner = " << partitioner << '\n'
              << "  grain_rows  = " << grain_rows << '\n'
              << "  grain_cols  = " << grain_cols << '\n';
    tbb::blocked_range2d<int> const range(
        0, params.h, static_cast<tbb::blocked_range2d<int>::row_range_type::size_type>(grain_rows),
        0, params.w, static_cast<tbb::blocked_range2d<int>::col_range_type::size_type>(grain_cols));

    auto body = [&](tbb::blocked_range2d<int> const & r) { process_pixel_block(r, params); };
    if (partitioner == "simple") {
      tbb::parallel_for(range, body, tbb::simple_partitioner{});
    } else if (partitioner == "static") {
      tbb::parallel_for(range, body, tbb::static_partitioner{});
    } else {
      tbb::parallel_for(range, body, tbb::auto_partitioner{});
    }
  }

}  // namespace soa
