#ifndef RENDER_OBJECT_BASE_HPP
#define RENDER_OBJECT_BASE_HPP

namespace render {

  // ENUM QUE IDENTIFICA EL TIPO DE OBJETO (necesario para el dispatching switch)
  enum ObjectType { SPHERE_TYPE, CYLINDER_TYPE };

  // ESTRUCTURA QUE DEFINE EL TIPO DE OBJETO SEGÚN EL ENUM
  struct ObjectBase {
    ObjectType type;

    explicit ObjectBase(ObjectType t = SPHERE_TYPE) : type(t) { }

    virtual ~ObjectBase() = default;

    ObjectBase(ObjectBase const &)                 = default;
    ObjectBase & operator=(ObjectBase const &)     = default;
    ObjectBase(ObjectBase &&) noexcept             = default;
    ObjectBase & operator=(ObjectBase &&) noexcept = default;
  };

}  // namespace render

#endif
