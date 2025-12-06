#include "../include/aos_ray.hpp"
#include "../../common/include/vector.hpp"
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

      if (std::abs(dist_along_axis) > cyl.height / 2.0) {
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

    // Bases del cilindro
    std::optional<HitRecord> hit_cylinder_caps(Ray const & r, CylinderParams const & cyl,
                                               CylinderHelperParams const & helper,
                                               IntersectionParams const & params) {
      // Base superior
      CapParams const top_cap{.center = cyl.center + helper.axis_unit * (cyl.height / 2.0),
                              .normal = helper.axis_unit,
                              .radius = cyl.radius};

      // Base inferior
      CapParams const bottom_cap{.center = cyl.center - helper.axis_unit * (cyl.height / 2.0),
                                 .normal = -helper.axis_unit,
                                 .radius = cyl.radius};

      auto top_hit    = hit_single_cap(r, top_cap, params);
      auto bottom_hit = hit_single_cap(r, bottom_cap, params);

      // Retornar la intersección más cercana
      if (top_hit and bottom_hit) {
        return (top_hit->t < bottom_hit->t) ? top_hit : bottom_hit;
      }
      if (top_hit) {
        return (top_hit->t < helper.closest_t) ? top_hit : std::nullopt;
      }
      if (bottom_hit) {
        return (bottom_hit->t < helper.closest_t) ? bottom_hit : std::nullopt;
      }

      return std::nullopt;
    }

  }  // namespace

  // Función principal de intersección RAYO-CILINDRO
  std::optional<HitRecord> hit_cylinder(Ray const & r, CylinderParams const & cyl,
                                        IntersectionParams const & params) {
    render::vector const axis_unit = cyl.axis / cyl.axis.magnitude();

    // Superficie curva
    auto surface_hit       = hit_cylinder_surface(r, cyl, axis_unit, params);
    double const closest_t = surface_hit ? surface_hit->t : params.t_max + 1.0;

    // Bases
    CylinderHelperParams const helper{axis_unit, closest_t};
    auto caps_hit = hit_cylinder_caps(r, cyl, helper, params);

    if (caps_hit) {
      return caps_hit;
    }
    return surface_hit;
  }

}  // namespace ray
