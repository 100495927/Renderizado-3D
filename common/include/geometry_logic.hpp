#ifndef RENDER_GEOMETRY_LOGIC_HPP
#define RENDER_GEOMETRY_LOGIC_HPP

#include "hit_record.hpp"
#include "objects.hpp"
#include "ray.hpp"
#include <array>

namespace render {

  // FUNCIONES DE COLISIÓN - DECLARACIONES

  // esfera
  double hit_sphere(ray const & r, double t_min, double t_max, Sphere const * sph);
  bool process_sphere_hit(ray const & r, double t, hit_record & rec, Sphere const * sph);

  // cilindro
  double hit_cylinder_lateral(ray const & r, double t_min, double t_max, Cylinder const * cyl);
  double hit_cylinder_caps(ray const & r, double t_min, double t_max, Cylinder const * cyl);
  double hit_cylinder(ray const & r, double t_min, double t_max, Cylinder const * cyl);
  bool process_cylinder_hit(ray const & r, double t, hit_record & rec, Cylinder const * cyl);

  // dispatch general
  bool hit_object(ray const & r, std::array<double, 2> const & t_range, hit_record & rec,
                  ObjectBase const * obj);

}  // namespace render

#endif  // RENDER_GEOMETRY_LOGIC_HPP
