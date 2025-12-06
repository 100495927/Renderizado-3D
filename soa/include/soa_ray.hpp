#ifndef SOA_RAY_HPP
#define SOA_RAY_HPP

// #include "soa_color.hpp"
#include "../../common/include/vector.hpp"
#include <optional>

namespace ray {

  // Estructura para representar un rayo
  struct Ray {
    render::vector origin;
    render::vector direction;

    Ray() = default;

    Ray(render::vector const & o, render::vector const & d) : origin(o), direction(d) { }

    // Calculamos un punto a lo largo del rayo: P(t) = origin + t * direction
    [[nodiscard]] render::vector at(double t) const { return origin + direction * t; }
  };

  // Intersección RAYO-OBJETO
  struct HitRecord {
    render::vector point;   // Punto de intersección
    render::vector normal;  // Vector normal en el punto
    double t{};             // Distancia desde el origen del rayo
    bool front_face{};      // true si golpea la cara frontal del objeto, false la trasera

    // Normal según la dirección del rayo
    void set_face_normal(Ray const & r, render::vector const & outward_normal) {
      front_face = render::dot(r.direction, outward_normal) < 0;
      normal     = front_face ? outward_normal : -outward_normal;
    }
  };

  struct IntersectionParams {
    double t_min;
    double t_max;
  };

  struct CylinderParams {
    render::vector center;
    double radius{};
    render::vector axis;
    double height{};
  };

  // Constante mínima para evitar auto-intersecciones
  constexpr double MIN_DISTANCE = 1e-3;

  // Intersección RAYO-ESFERA
  [[nodiscard]] std::optional<HitRecord> hit_sphere(Ray const & r, render::vector const & center,
                                                    double radius,
                                                    IntersectionParams const & params);

  // Intersección RAYO-CILINDRO
  [[nodiscard]] std::optional<HitRecord> hit_cylinder(Ray const & r,
                                                      CylinderParams const & cyl_params,
                                                      IntersectionParams const & params);

}  // namespace ray

#endif  // SOA_RAY_HPP
