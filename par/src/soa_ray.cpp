#include "../include/soa_ray.hpp"
#include "../../common/include/vector.hpp"
// #include <algorithm>
// #include <array>
#include <cmath>
#include <optional>

namespace ray {

  // Intersección RAYO-ESFERA
  std::optional<HitRecord> hit_sphere(Ray const & r, render::vector const & center, double radius,
                                      IntersectionParams const & params) {
    // Vector desde centro a origen (oc = origen - centro)
    render::vector const oc = r.origin - center;

    // Coeficientes de la ecuación de segundo grado: a t^2 + b t + c = 0
    double const a = render::dot(r.direction, r.direction);
    double const b = 2.0 * render::dot(oc, r.direction);
    double const c = render::dot(oc, oc) - radius * radius;

    // Discriminante
    double const discriminant = b * b - 4.0 * a * c;

    if (discriminant < 0) {
      return std::nullopt;  // No hay intersección
    }

    // Calculamos la raíz más cercana válida
    double const sqrt_discriminant = std::sqrt(discriminant);
    double root                    = (-b - sqrt_discriminant) / (2.0 * a);

    // Verificamos si la raíz más cercana está en el rango válido
    if (root < params.t_min or root > params.t_max) {
      root = (-b + sqrt_discriminant) / (2.0 * a);
      if (root < params.t_min or root > params.t_max) {
        return std::nullopt;
      }
    }

    // Intersección válida encontrada
    HitRecord rec;
    rec.t     = root;
    rec.point = r.at(root);

    // Normal: (punto-centro) / radio
    render::vector const outward_normal = (rec.point - center) / radius;
    rec.set_face_normal(r, outward_normal);

    return rec;
  }

  namespace {

    render::vector perpendicular_component(render::vector const & v,
                                           render::vector const & axis_unit) {
      return v - axis_unit * render::dot(v, axis_unit);
    }

    struct CylinderHelperParams {
      render::vector axis_unit;
      double closest_t{};
    };

    // Struct para agrupar parámetros de la base del cilindro
    struct CapParams {
      render::vector center;
      render::vector normal;
      double radius{};
    };

    // Superficie del cilindro
    std::optional<HitRecord> hit_cylinder_surface(Ray const & r, CylinderParams const & cyl,
                                                  render::vector const & axis_unit,
                                                  IntersectionParams const & params) {
      render::vector const rc       = r.origin - cyl.center;
      render::vector const rc_perp  = perpendicular_component(rc, axis_unit);
      render::vector const dir_perp = perpendicular_component(r.direction, axis_unit);

      double const a = render::dot(dir_perp, dir_perp);
      double const b = 2.0 * render::dot(rc_perp, dir_perp);
      double const c = render::dot(rc_perp, rc_perp) - cyl.radius * cyl.radius;

      double const discriminant = b * b - 4.0 * a * c;

      if (discriminant < 0 or std::abs(a) <= 1e-8) {
        return std::nullopt;
      }

      double const sqrt_disc = std::sqrt(discriminant);
      double root            = (-b - sqrt_disc) / (2.0 * a);

      if (root < params.t_min or root > params.t_max) {
        root = (-b + sqrt_disc) / (2.0 * a);
      }

      if (root < params.t_min or root > params.t_max) {
        return std::nullopt;
      }

      render::vector const point   = r.at(root);
      double const dist_along_axis = render::dot(point - cyl.center, axis_unit);

      double const axis_magnitude = cyl.axis.magnitude();
      if (std::abs(dist_along_axis) > axis_magnitude / 2.0) {
        return std::nullopt;
      }

      HitRecord rec;
      rec.t                               = root;
      rec.point                           = point;
      render::vector const outward_normal = perpendicular_component(point - cyl.center, axis_unit);
      rec.set_face_normal(r, outward_normal);
      return rec;
    }

    // Helper para probar intersección con una base del cilindro
    std::optional<HitRecord> hit_single_cap(Ray const & r, CapParams const & cap,
                                            IntersectionParams const & params) {
      double const denom = render::dot(r.direction, cap.normal);
      if (std::abs(denom) <= 1e-8) {
        return std::nullopt;
      }

      render::vector const rp = cap.center - r.origin;
      double const t          = render::dot(rp, cap.normal) / denom;

      if (t < params.t_min or t > params.t_max) {
        return std::nullopt;
      }

      render::vector const point    = r.at(t);
      double const dist_from_center = (point - cap.center).magnitude();

      if (dist_from_center <= cap.radius) {
        HitRecord rec;
        rec.t     = t;
        rec.point = point;
        rec.set_face_normal(r, cap.normal);
        return rec;
      }

      return std::nullopt;
    }

  }  // namespace

  // Función principal de intersección RAYO-CILINDRO
  std::optional<HitRecord> hit_cylinder(Ray const & r, CylinderParams const & cyl,
                                        IntersectionParams const & params) {
    double const axis_mag = cyl.axis.magnitude();
    if (axis_mag < 1e-8) {
      return std::nullopt;
    }
    render::vector const axis_unit = cyl.axis / axis_mag;

    // Probar TODAS las intersecciones (superficie + dos caps)
    std::optional<HitRecord> best_hit;
    double best_t = params.t_max;

    // 1. Superficie del cilindro
    auto surface_hit = hit_cylinder_surface(r, cyl, axis_unit, params);
    if (surface_hit and surface_hit->t < best_t) {
      best_hit = surface_hit;
      best_t   = surface_hit->t;
    }
    // 2. Caps
    double const half_height = axis_mag / 2.0;
    CapParams const top_cap{
      .center = cyl.center + axis_unit * half_height, .normal = axis_unit, .radius = cyl.radius};
    CapParams const bottom_cap{
      .center = cyl.center - axis_unit * half_height, .normal = -axis_unit, .radius = cyl.radius};
    auto top_hit = hit_single_cap(r, top_cap, params);
    if (top_hit and top_hit->t < best_t) {
      best_hit = top_hit;
      best_t   = top_hit->t;
    }
    auto bottom_hit = hit_single_cap(r, bottom_cap, params);
    if (bottom_hit and bottom_hit->t < best_t) {
      best_hit = bottom_hit;
    }

    return best_hit;
  }

}  // namespace ray
