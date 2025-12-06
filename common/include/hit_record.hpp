#ifndef RENDER_HIT_RECORD_HPP
#define RENDER_HIT_RECORD_HPP

#include "ray.hpp"
#include "vector.hpp"

namespace render {

  // estructura material base a definir después (para definir punteros o referencias a ella)
  struct MaterialBase;

  // ESTRUCTURA QUE DEFINE UNA INTERSECCIÓN ENTRE RAYO Y OBJETO
  struct hit_record {
    // DATOS A REGISTRAR

    // punto de intersección
    point_vector intersect;
    // vector normal a la superficie de un objeto
    normal_vector normal;
    // distancia donde se produce la intersección del origen del rayo
    double t{};
    // indica si el rayo golpea la cara frontal
    bool front_face{};
    // puntero al material del objeto golpeado
    MaterialBase const * mat_pointer{};

    // función para ajustar la normal (necesaria para refracción)
    void set_face_normal(ray const & r, normal_vector const & outward_normal) {
      // si el dot product es negativo el rayo va contra la normal
      front_face = dot(r.dir, outward_normal) < 0;
      // la normal siempre apunta hacia afuera del rayo incidente
      normal = front_face ? outward_normal : -outward_normal;
    }
  };

}  // namespace render

#endif
