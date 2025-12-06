#include "../include/geometry_logic.hpp"
#include "../include/hit_record.hpp"
#include "../include/object_base.hpp"
#include "../include/objects.hpp"
#include "../include/ray.hpp"
#include "../include/vector.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <utility>

namespace render {

  // ESFERA

  // colisión con una esfera
  // calcula el valor de 't' más cercano dentro del rango [t_min, t_max] donde el rayo 'r'
  // intersecta 'sph'. Devuelve -1.0 si no hay intersección válida
  double hit_sphere(ray const & r, double t_min, double t_max, Sphere const * sph) {
    point_vector const center_vec(sph->center_x, sph->center_y, sph->center_z);
    direction_vector const oc = r.orig - center_vec;
    auto a                    = r.dir.magnitude();
    auto half_b               = dot(oc, r.dir);
    auto c                    = oc.magnitude() - sph->radius * sph->radius;
    auto discriminant         = half_b * half_b - a * c;
    if (discriminant < 0) {
      return -1.0;
    }
    auto sqrtd        = std::sqrt(discriminant);
    double closest_t  = t_max;
    bool hit_anything = false;
    auto root1        = (-half_b - sqrtd) / a;
    if (root1 > t_min and root1 < closest_t) {
      closest_t    = root1;
      hit_anything = true;
    }
    auto root2 = (-half_b + sqrtd) / a;
    if (root2 > t_min and root2 < closest_t) {
      closest_t    = root2;
      hit_anything = true;
    }
    if (!hit_anything) {
      return -1.0;
    }
    return closest_t;
  }

  // procesado de colisión para esfera
  // rellena 'rec' con los datos de intersección si 't' es válido. Retorna true cuando se ha
  // registrado correctamente la intersección
  bool process_sphere_hit(ray const & r, double t, hit_record & rec, Sphere const * sph) {
    if (t < 0) {
      return false;
    }
    point_vector const center_vec(sph->center_x, sph->center_y, sph->center_z);
    rec.t                                 = t;
    rec.intersect                         = r.at(rec.t);
    direction_vector const outward_normal = unit_vector(rec.intersect - center_vec);
    rec.set_face_normal(r, outward_normal);
    return true;
  }

  // CILINDRO

  // colisión con un cilindro lateral
  // prueba intersecciones con la superficie lateral del cilindro y devuelve el 't' más cercano en
  // (t_min, t_max) o -1.0 si no hay hit lateral
  double hit_cylinder_lateral(ray const & r, double t_min, double t_max, Cylinder const * cyl) {
    point_vector const center(cyl->center_x, cyl->center_y, cyl->center_z);
    direction_vector const axis =
        unit_vector(direction_vector(cyl->axis_x, cyl->axis_y, cyl->axis_z));
    double const radius = cyl->radius, HALF_HEIGHT = cyl->height / 2.0;
    direction_vector const oc = r.orig - center;
    double const a_dot_d = dot(axis, r.dir), a_dot_oc = dot(axis, oc);
    auto A            = r.dir.magnitude() - a_dot_d * a_dot_d;
    auto B            = 2.0 * (dot(r.dir, oc) - a_dot_d * a_dot_oc);
    auto C            = oc.magnitude() - a_dot_oc * a_dot_oc - radius * radius;
    auto discriminant = B * B - 4.0 * A * C;
    double closest_t  = -1.0;
    if (discriminant >= 0) {
      auto sqrtd                              = std::sqrt(discriminant);
      std::array<double, 2> const t_solutions = {(-B - sqrtd) / (2.0 * A),
                                                 (-B + sqrtd) / (2.0 * A)};
      for (double const t_side : t_solutions) {
        if (t_side > t_min and t_side < t_max) {
          point_vector const hit_point = r.at(t_side);
          double const hit_height      = dot(hit_point - center, axis);
          if (std::fabs(hit_height) <= HALF_HEIGHT) {
            if (closest_t < 0 or t_side < closest_t) {
              closest_t = t_side;
            }
          }
        }
      }
    }
    return closest_t;
  }

  // colisión con cilindro tapas
  // comprueba intersecciones contra los discos que cierran el cilindro (caps) y devuelve el t más
  // cercano válido o -1.0 si no hay intersección
  double hit_cylinder_caps(ray const & r, double t_min, double t_max, Cylinder const * cyl) {
    point_vector const center(cyl->center_x, cyl->center_y, cyl->center_z);
    direction_vector const axis =
        unit_vector(direction_vector(cyl->axis_x, cyl->axis_y, cyl->axis_z));
    double const radius = cyl->radius, HALF_HEIGHT = cyl->height / 2.0;
    double closest_t                                                    = -1.0;
    std::array<std::pair<point_vector, direction_vector>, 2> const caps = {
      std::make_pair(center + (HALF_HEIGHT * axis), axis),
      std::make_pair(center - (HALF_HEIGHT * axis), -axis)};
    for (auto const & cap : caps) {
      double const d_dot_n = dot(r.dir, cap.second);
      if (std::fabs(d_dot_n) < 1e-8) {
        continue;
      }
      double const t_cap = dot(cap.first - r.orig, cap.second) / d_dot_n;
      if (t_cap > t_min and t_cap < t_max) {
        point_vector const hit_point = r.at(t_cap);
        if ((hit_point - cap.first).magnitude() <= radius * radius) {
          if (closest_t < 0 or t_cap < closest_t) {
            closest_t = t_cap;
          }
        }
      }
    }
    return closest_t;
  }

  // colisión con cilindro (combinado)
  // Combina las pruebas laterales y de tapas y devuelve el t más cercano válido entre ambas. -1.0
  // indica ausencia de colisión
  double hit_cylinder(ray const & r, double t_min, double t_max, Cylinder const * cyl) {
    double const lateral_t = hit_cylinder_lateral(r, t_min, t_max, cyl);
    double const caps_t    = hit_cylinder_caps(r, t_min, t_max, cyl);
    if (lateral_t < 0 and caps_t < 0) {
      return -1.0;
    }
    if (lateral_t < 0) {
      return caps_t;
    }
    if (caps_t < 0) {
      return lateral_t;
    }
    return std::min(lateral_t, caps_t);
  }

  // procesado de colisión para cilindro
  // determina si el hit corresponde a una tapa o a la superficie lateral, calcula la normal
  // adecuada y rellena 'rec' con los datos de intersección
  bool process_cylinder_hit(ray const & r, double t, hit_record & rec, Cylinder const * cyl) {
    if (t < 0) {
      return false;
    }
    point_vector const center(cyl->center_x, cyl->center_y, cyl->center_z);
    direction_vector const axis =
        unit_vector(direction_vector(cyl->axis_x, cyl->axis_y, cyl->axis_z));
    double const radius           = cyl->radius;
    double const HALF_HEIGHT      = 1.0;
    rec.t                         = t;
    rec.intersect                 = r.at(rec.t);
    direction_vector const hit_oc = rec.intersect - center;
    double const hit_height       = dot(hit_oc, axis);
    direction_vector outward_normal;
    std::array<std::pair<point_vector, direction_vector>, 2> const caps = {
      std::make_pair(center + (HALF_HEIGHT * axis), axis),
      std::make_pair(center - (HALF_HEIGHT * axis), -axis)};
    bool is_cap_hit = false;
    for (auto const & cap : caps) {
      point_vector const P_cap          = cap.first;
      direction_vector const N_cap      = cap.second;
      direction_vector const vec_to_cap = rec.intersect - P_cap;
      if (std::fabs(dot(vec_to_cap, N_cap)) < 1e-6 and vec_to_cap.magnitude() <= radius * radius) {
        outward_normal = N_cap;
        is_cap_hit     = true;
        break;
      }
    }
    if (!is_cap_hit) {
      direction_vector const p_minus_c           = rec.intersect - center;
      direction_vector const normal_unnormalized = p_minus_c - (hit_height * axis);
      outward_normal                             = unit_vector(normal_unnormalized);
    }
    rec.set_face_normal(r, outward_normal);
    return true;
  }

  // elección general de colisiones
  // llama a la función específica según el tipo de objeto y procesa el hit para rellenar 'rec'.
  // Devuelve true si hubo intersección
  bool hit_object(ray const & r, std::array<double, 2> const & t_range, hit_record & rec,
                  ObjectBase const * obj) {
    if (obj == nullptr) {
      return false;
    }

    // Usar dynamic_cast con verificación y manejar el caso de fallo
    if (auto const * sphere = dynamic_cast<Sphere const *>(obj)) {
      double const t = hit_sphere(r, t_range[0], t_range[1], sphere);
      return process_sphere_hit(r, t, rec, sphere);
    }
    if (auto const * cylinder = dynamic_cast<Cylinder const *>(obj)) {
      double const t = hit_cylinder(r, t_range[0], t_range[1], cylinder);
      return process_cylinder_hit(r, t, rec, cylinder);
    }

    return false;  // Tipo desconocido o casteo fallido
  }

}  // namespace render
