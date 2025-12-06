#ifndef RENDER_RAY_HPP
#define RENDER_RAY_HPP

#include "vector.hpp"

namespace render {

  // CLASE QUE DEFINE A UN RAYO
  class ray {
  public:
    // vectores que representan origen y dirección respectivamente
    point_vector orig;
    direction_vector dir;

    // para crear instancia se deberan recibir 2 vectores que se guardan en los ya declarados
    ray() = default;

    ray(point_vector const & origin, direction_vector const & direction)
        : orig(origin), dir(direction) { }

    // FUNCIÓN PARA CALCULAR POSICIÓN A LA DISTANCIA t DEL ORIGEN, A LO LARGO DE LA DIRECCIÓN
    // P(t) = origen + t*dirección
    [[nodiscard]] point_vector at(double t) const { return orig + t * dir; }
  };

}  // namespace render

#endif
