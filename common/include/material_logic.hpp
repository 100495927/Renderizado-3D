#ifndef RENDER_MATERIAL_LOGIC_HPP
#define RENDER_MATERIAL_LOGIC_HPP

#include "hit_record.hpp"
#include "materials.hpp"
#include "math_utilities.hpp"
#include "ray.hpp"
#include "vector.hpp"

namespace render {

  // estructura utilizada para agrupar los punteros a los datos de salida y el contexto (RNG)
  struct ScatterIO {
    color_vector * attenuation;
    ray * scattered;
    RNG * rng;
  };

  // FUNCIONES DE DISPERSIÓN - DECLARACIONES
  bool scatter_matte(hit_record const & rec, MatteMaterial const * mat, ScatterIO & io);
  bool scatter_metal(ray const & r_in, hit_record const & rec, MetalMaterial const * mat,
                     ScatterIO & io);
  bool scatter_refractive(ray const & r_in, hit_record const & rec, RefractiveMaterial const * mat,
                          ScatterIO & io);

  // función central de dispersión que elige la lógica por tipo de material
  bool scatter(ray const & r_in, hit_record const & rec, ScatterIO & io);

}  // namespace render

#endif  // RENDER_MATERIAL_LOGIC_HPP
