#include "../include/render_aos.hpp"

#include "../../common/include/config.hpp"
#include "../../common/include/materials.hpp"
#include "../../common/include/scene_parser.hpp"
#include "../../common/include/vector.hpp"
#include "../include/aos_camera.hpp"
#include "../include/aos_color.hpp"
#include "../include/aos_image.hpp"
#include "../include/aos_ray.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <optional>
#include <random>
#include <string>

#ifndef M_PI
 #define M_PI 3.14159265358979323846
#endif

namespace aos {

  namespace {

    // ==================== ESTRUCTURAS OPTIMIZADAS ====================

    struct MaterialInfo {
      enum Type { MATTE, METAL, REFRACTIVE } type{MATTE};

      struct {
        double r{0.8};
        double g{0.8};
        double b{0.8};
      } reflectance;

      double roughness{0.0};
      double refraction{1.0};
    };

    struct HitInfo {
      bool hit{false};
      render::vector point;
      render::vector normal;
      double t{0};
      MaterialInfo material;
      std::string material_name;
    };

    struct RayTracingContext {
      int depth{};
      ConfigParams const * cfg{};
      SceneOutput const * scene{};
      render::vector ray_direction;
    };

    struct RefractiveParams {
      double n1{1.0};
      double n2{1.0};
      render::vector normal;
      double cos_theta_in{0.0};
    };

    // ==================== BÚSQUEDA DE MATERIAL OPTIMIZADA ====================

    MaterialInfo create_material_info(MatteMaterial const & m) {
      return {
        MaterialInfo::MATTE, {m.reflectance_r, m.reflectance_g, m.reflectance_b},
         0.0, 0.0
      };
    }

    MaterialInfo create_material_info(MetalMaterial const & m) {
      return {
        MaterialInfo::METAL, {m.reflectance_r, m.reflectance_g, m.reflectance_b},
         m.diffusion, 0.0
      };
    }

    MaterialInfo create_material_info(RefractiveMaterial const & m) {
      return {
        MaterialInfo::REFRACTIVE, {1.0, 1.0, 1.0},
         0.0, m.refractive_index
      };
    }

    MaterialInfo find_material(std::string const & name, SceneOutput const & scene) {
      // Búsqueda optimizada
      for (auto const & m : scene.matte_materials.get()) {
        if (m.name == name) {
          return create_material_info(m);
        }
      }

      for (auto const & m : scene.metal_materials.get()) {
        if (m.name == name) {
          return create_material_info(m);
        }
      }

      for (auto const & m : scene.refractive_materials.get()) {
        if (m.name == name) {
          return create_material_info(m);
        }
      }

      return {
        MaterialInfo::MATTE, {0.5, 0.5, 0.5},
         0.0, 0.0
      };
    }

    // ==================== HELPERS OPTIMIZADOS ====================

    color::Color background_ray(ray::Ray const & r, ConfigParams const & cfg) {
      render::vector const dir_unit = r.direction / r.direction.magnitude();
      double const dir_y            = -dir_unit.get_y();

      color::Color const light{cfg.background_light_color_r, cfg.background_light_color_g,
                               cfg.background_light_color_b};

      color::Color const dark{cfg.background_dark_color_r, cfg.background_dark_color_g,
                              cfg.background_dark_color_b};

      return color::background_color(dir_y, light, dark);
    }

    // ==================== DECLARACIONES FORWARD ====================

    color::Color trace_ray(ray::Ray const & r, RayTracingContext const & ctx);

    // ==================== MANEJO DE MATERIALES OPTIMIZADO ====================

    color::Color apply_material_color(color::Color const & reflected, MaterialInfo const & mat) {
      return {reflected.r * mat.reflectance.r, reflected.g * mat.reflectance.g,
              reflected.b * mat.reflectance.b};
    }

    color::Color process_metal(ray::Ray const & r, HitInfo const & hit,
                               RayTracingContext const & ctx) {
      static constexpr double EPSILON = 1e-4;

      double const dot         = render::dot(r.direction, hit.normal);
      render::vector reflected = r.direction - hit.normal * (2.0 * dot);

      if (hit.material.roughness > 1e-8) {
        static thread_local std::mt19937 gen(
            static_cast<std::mt19937::result_type>(ctx.cfg->material_rng_seed));
        std::uniform_real_distribution<> dis(-1.0, 1.0);

        render::vector const perturbation{dis(gen) * hit.material.roughness,
                                          dis(gen) * hit.material.roughness,
                                          dis(gen) * hit.material.roughness};

        reflected = (reflected + perturbation) / (reflected + perturbation).magnitude();
      } else {
        reflected = reflected / reflected.magnitude();
      }

      ray::Ray const bounce_ray{hit.point + hit.normal * EPSILON, reflected};
      RayTracingContext const new_ctx{ctx.depth - 1, ctx.cfg, ctx.scene, reflected};

      return apply_material_color(trace_ray(bounce_ray, new_ctx), hit.material);
    }

    color::Color process_matte(HitInfo const & hit, RayTracingContext const & ctx) {
      static constexpr double EPSILON = 1e-4;

      static thread_local std::mt19937 gen(std::random_device{}());
      std::uniform_real_distribution<> dis(0.0, 1.0);

      double const r1 = dis(gen);
      double const r2 = dis(gen);

      double const theta = std::acos(std::sqrt(r1));
      double const phi   = 2.0 * M_PI * r2;

      render::vector const tangent =
          std::abs(hit.normal.get_x()) < 0.9 ? render::vector{1, 0, 0} : render::vector{0, 1, 0};

      render::vector const bitangent     = render::cross(hit.normal, tangent);
      render::vector const final_tangent = render::cross(bitangent, hit.normal);

      render::vector const reflected_dir = hit.normal * std::cos(theta) +
                                           final_tangent * std::sin(theta) * std::cos(phi) +
                                           bitangent * std::sin(theta) * std::sin(phi);

      ray::Ray const bounce_ray{hit.point + hit.normal * EPSILON, reflected_dir};
      RayTracingContext const new_ctx{ctx.depth - 1, ctx.cfg, ctx.scene, reflected_dir};

      return apply_material_color(trace_ray(bounce_ray, new_ctx), hit.material);
    }

    double schlick_fresnel(double cosine, double ref_idx) {
      double const r0 = std::pow((1.0 - ref_idx) / (1.0 + ref_idx), 2.0);
      return r0 + (1.0 - r0) * std::pow(1.0 - cosine, 5.0);
    }

    RefractiveParams determine_refraction_params(ray::Ray const & r,
                                                 render::vector const & hit_normal,
                                                 double ref_idx) {
      RefractiveParams result;
      result.cos_theta_in = render::dot(r.direction, hit_normal);

      if (result.cos_theta_in > 0.0) {
        result.n1           = ref_idx;
        result.n2           = 1.0;
        result.normal       = hit_normal * -1.0;
        result.cos_theta_in = -result.cos_theta_in;
      } else {
        result.n1           = 1.0;
        result.n2           = ref_idx;
        result.normal       = hit_normal;
        result.cos_theta_in = -result.cos_theta_in;
      }

      return result;
    }

    color::Color refract_reflect(HitInfo const & hit, render::vector const & direction,
                                 RayTracingContext const & ctx) {
      static constexpr double EPSILON = 1e-4;
      render::vector const origin     = hit.point + hit.normal * EPSILON;

      ray::Ray const bounce_ray{origin, direction};
      RayTracingContext const new_ctx{ctx.depth - 1, ctx.cfg, ctx.scene, direction};

      return trace_ray(bounce_ray, new_ctx);
    }

    color::Color process_refractive(ray::Ray const & r, HitInfo const & hit,
                                    RayTracingContext const & ctx) {
      RefractiveParams const params =
          determine_refraction_params(r, hit.normal, hit.material.refraction);

      double const sin_theta1 = std::sqrt(1.0 - params.cos_theta_in * params.cos_theta_in);
      double const sin_theta2 = (params.n1 / params.n2) * sin_theta1;

      if (sin_theta2 > 1.0) {
        double const dot               = render::dot(r.direction, hit.normal);
        render::vector const reflected = r.direction - hit.normal * (2.0 * dot);
        return refract_reflect(hit, reflected, ctx);
      }

      double const cos_theta2   = std::sqrt(1.0 - sin_theta2 * sin_theta2);
      double const reflect_prob = schlick_fresnel(params.cos_theta_in, hit.material.refraction);

      static thread_local std::mt19937 gen(
          static_cast<std::mt19937::result_type>(ctx.cfg->material_rng_seed));
      std::uniform_real_distribution<> dis(0.0, 1.0);

      if (dis(gen) < reflect_prob) {
        double const dot               = render::dot(r.direction, hit.normal);
        render::vector const reflected = r.direction - hit.normal * (2.0 * dot);
        return refract_reflect(hit, reflected, ctx);
      }

      render::vector const refracted =
          r.direction * (params.n1 / params.n2) +
          params.normal * ((params.n1 / params.n2) * params.cos_theta_in - cos_theta2);

      return refract_reflect(hit, refracted, ctx);
    }

    // ==================== INTERSECCIONES OPTIMIZADAS ====================

    HitInfo check_spheres(ray::Ray const & r, SceneOutput const & scene) {
      HitInfo hit{};
      ray::IntersectionParams params{ray::MIN_DISTANCE, 1e18};

      for (auto const & sph : scene.spheres.get()) {
        render::vector const center{sph.center_x, sph.center_y, sph.center_z};
        auto const rec = ray::hit_sphere(r, center, sph.radius, params);

        if (rec and (!hit.hit or rec->t < hit.t)) {
          hit.hit           = true;
          hit.t             = rec->t;
          hit.point         = rec->point;
          hit.normal        = rec->normal;
          hit.material_name = sph.material_name;
          params.t_max      = rec->t;
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
        cp.height = cp.axis.magnitude();

        auto const rec = ray::hit_cylinder(r, cp, params);

        if (rec and (!hit.hit or rec->t < hit.t)) {
          hit.hit           = true;
          hit.t             = rec->t;
          hit.point         = rec->point;
          hit.normal        = rec->normal;
          hit.material_name = cyl.material_name;
          params.t_max      = rec->t;
        }
      }

      return hit;
    }

    HitInfo closest_intersection(ray::Ray const & r, SceneOutput const & scene) {
      auto const sphere_hit   = check_spheres(r, scene);
      auto const cylinder_hit = check_cylinders(r, scene);

      if (sphere_hit.hit and cylinder_hit.hit) {
        return sphere_hit.t < cylinder_hit.t ? sphere_hit : cylinder_hit;
      }

      return sphere_hit.hit ? sphere_hit : cylinder_hit;
    }

    // ==================== RAY TRACING RECURSIVO OPTIMIZADO ====================

    color::Color trace_ray(ray::Ray const & r, RayTracingContext const & ctx) {
      if (ctx.depth <= 0) {
        return color::Color{0, 0, 0};
      }

      HitInfo const hit = closest_intersection(r, *ctx.scene);

      if (!hit.hit) {
        return background_ray(r, *ctx.cfg);
      }

      MaterialInfo const material = find_material(hit.material_name, *ctx.scene);

      switch (material.type) {
        case MaterialInfo::METAL:
          return process_metal(
              r, {hit.hit, hit.point, hit.normal, hit.t, material, hit.material_name}, ctx);

        case MaterialInfo::MATTE:
          return process_matte({hit.hit, hit.point, hit.normal, hit.t, material, hit.material_name},
                               ctx);

        case MaterialInfo::REFRACTIVE:
          return process_refractive(
              r, {hit.hit, hit.point, hit.normal, hit.t, material, hit.material_name}, ctx);

        default: return color::Color{1, 1, 1};
      }
    }

  }  // namespace

  // ==================== FUNCIÓN PRINCIPAL DE RENDERIZADO CORREGIDA ====================

  void render_scene(ConfigParams const & cfg, SceneOutput const & scene, CameraAOS & camera,
                    AOSImage & image) {
    int const w         = image.width();
    int const h         = image.height();
    int const spp       = std::max(1, cfg.samples_per_pixel);
    int const max_depth = std::max(1, cfg.max_depth);
    // INICIALIZACIÓN CRÍTICA: llamar begin_frame antes de generar rayos
    camera.begin_frame(static_cast<std::size_t>(w), static_cast<std::size_t>(h));
    // RNG para rayos
    std::mt19937 rng(static_cast<std::mt19937::result_type>(cfg.ray_rng_seed));

    for (int j = 0; j < h; ++j) {
      for (int i = 0; i < w; ++i) {
        color::Color accum{0.0, 0.0, 0.0};

        // Accumulate monte-carlo samples for this pixel
        for (int s = 0; s < spp; ++s) {
          // CORRECCIÓN: usar índices correctos (i, j) en lugar de (j, i)
          auto const ray_sample =
              camera.make_ray(static_cast<std::size_t>(i), static_cast<std::size_t>(j), rng);

          // Convertir a ray::Ray
          ray::Ray const r{
            render::vector{ray_sample.origin_x, ray_sample.origin_y, ray_sample.origin_z},
            render::vector{   ray_sample.dir_x,    ray_sample.dir_y,    ray_sample.dir_z}
          };

          RayTracingContext const ctx{max_depth, &cfg, &scene, r.direction};
          color::Color const c = trace_ray(r, ctx);
          accum                = accum + c;
        }

        // Aplicar promedio
        double const inv_spp = 1.0 / static_cast<double>(spp);
        RGBColor const out_color{accum.r * inv_spp, accum.g * inv_spp, accum.b * inv_spp};

        // CORRECCIÓN: usar índices correctos (j, i) para setPixel
        image.setPixel(j, i, out_color, cfg.gamma);
      }
    }
    std::cout << "AOS Render completed - Image size: " << w << "x" << h << "\n";
  }

}  // namespace aos
