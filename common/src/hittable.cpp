#include "../include/hittable.hpp"
#include "../include/geometry_logic.hpp"
#include "../include/hit_record.hpp"
#include "../include/ray.hpp"
#include <array>

namespace render {

  // función que busca la colisión más cercana entre determinado rayo y un objeto
  bool hittable_list::hit(render::ray const & r, double t_min, double t_max,
                          render::hit_record & rec) const {
    // almacena el registro del golpe más cercano temporalmente
    render::hit_record temp_rec;

    // inicializamos la distancia de búsqueda máxima con el t_max original (se reduce conforme
    // encontremos objetos más cerca)
    auto closest_so_far = t_max;
    bool hit_anything   = false;

    // iterar sobre todos los objetos de la escena
    for (auto const & object : objects) {
      // llamamos a la función general hit_object en object_logic.hpp
      if (hit_object(r, std::array<double, 2>{t_min, closest_so_far}, temp_rec, object)) {
        // hay colisión
        // registrar el impacto
        hit_anything = true;

        // actualizamos closest_so_far por distancia actual
        closest_so_far = temp_rec.t;

        // actualizamos colisión más cercana
        rec = temp_rec;
      }
    }

    // devolvemos true si se encontró al menos un impacto
    return hit_anything;
  }

}  // namespace render
