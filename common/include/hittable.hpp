#ifndef RENDER_HITTABLE_HPP
#define RENDER_HITTABLE_HPP

#include "hit_record.hpp"
#include "object_base.hpp"
#include "ray.hpp"
#include <vector>

namespace render {

  // CLASE QUE DEFINE LA LÓGICA ABSTRACTA DE OBJETOS EN UNA IMAGEN (necesaria para ver todas las
  // posibles colisiones)
  class hittable {
  public:
    hittable()                             = default;
    hittable(hittable const &)             = default;
    hittable & operator=(hittable const &) = default;
    hittable(hittable &&)                  = default;
    hittable & operator=(hittable &&)      = default;

    // llamamos a la clase hit, declarada en hittable.cpp
    virtual bool hit(ray const & r, double t_min, double t_max, hit_record & rec) const = 0;
    virtual ~hittable()                                                                 = default;
  };

  // CLASE QUE DEFINE LA LISTA DE PUNTEROS A OBJETOS (no distinguimos el tipo)
  class hittable_list : public hittable {
  public:
    // almacena punteros a la base geométrica.
    std::vector<ObjectBase const *> objects;

    // lista de punteros
    hittable_list() = default;

    void clear() { objects.clear(); }

    void add(ObjectBase const * object) { objects.push_back(object); }

    bool hit(ray const & r, double t_min, double t_max, hit_record & rec) const override;
  };

}  // namespace render

#endif
