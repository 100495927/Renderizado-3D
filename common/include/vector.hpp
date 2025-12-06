#ifndef RENDER_VECTOR_HPP
#define RENDER_VECTOR_HPP

#include <cmath>
#include <iostream>

namespace render {

  // CLASE QUE DEFINE A UN VECTOR
  class vector {
  public:
    // constructores
    vector() : x{0}, y{0}, z{0} { }

    vector(double cx, double cy, double cz) : x{cx}, y{cy}, z{cz} { }

    // DEFINICION DE FUNCIONES (las dos devuelven x, y o z por valor pero diferenciamos por
    // semántica)

    // devuelve posición
    [[nodiscard]] double get_x() const { return x; }

    [[nodiscard]] double get_y() const { return y; }

    [[nodiscard]] double get_z() const { return z; }

    // devuelve color
    [[nodiscard]] double r() const { return x; }

    [[nodiscard]] double g() const { return y; }

    [[nodiscard]] double b() const { return z; }

    // OPERADORES DE VECTORES

    // cálculo del inverso
    vector operator-() const { return {-x, -y, -z}; }

    // lectura de una posición del vector
    double operator[](int i) const;
    // modificación de una posición del vector
    double & operator[](int i);

    // operadores de asignación compuesta
    // suma de vectores (componentes uno a uno)
    vector & operator+=(vector const & v);
    // resta de vectores (componentes uno a uno)
    vector & operator-=(vector const & v);
    // producto por un escalar t
    vector & operator*=(double t);
    // división por un escalar t
    vector & operator/=(double t);

    // MAGNITUDES

    [[nodiscard]] double magnitude_squared() const;  // <-- DECLARACIÓN AÑADIDA
    [[nodiscard]] double magnitude() const;

    // UTILIDAD NUMÉRICA (para estabilidad, Sección 3.5.1)
    [[nodiscard]] bool near_zero() const {
      auto const s = 1e-8;
      return (std::abs(x) < s) and (std::abs(y) < s) and (std::abs(z) < s);
    }

  private:
    double x, y, z;
  };

  // DEFINICIÓN DE MAGNITUDES

  inline double vector::magnitude_squared() const {
    return x * x + y * y + z * z;
  }

  inline double vector::magnitude() const {
    return std::sqrt(magnitude_squared());
  }

  // DECLARACIÓN de alias

  // para posición
  using point_vector = vector;
  // para color
  using color_vector = vector;
  // para dirección
  using direction_vector = vector;
  // vector normal
  using normal_vector = vector;

  // UTILIDADES GLOBALES
  // están definidas como inline para que se inserten directamente en la llamada. No cambian el
  // valor como las de asignación compuesto

  // imprime valores de un vector directamente al stream out separados por espacios
  inline std::ostream & operator<<(std::ostream & out, vector const & v) {
    return out << v.get_x() << ' ' << v.get_y() << ' ' << v.get_z();
  }

  // suma de dos vectores
  inline vector operator+(vector const & u, vector const & v) {
    return {u.get_x() + v.get_x(), u.get_y() + v.get_y(), u.get_z() + v.get_z()};
  }

  // resta de dos vectores
  inline vector operator-(vector const & u, vector const & v) {
    return {u.get_x() - v.get_x(), u.get_y() - v.get_y(), u.get_z() - v.get_z()};
  }

  // producto hadamard (elemento a elemento, usado para el color)
  inline vector operator*(vector const & u, vector const & v) {
    return {u.get_x() * v.get_x(), u.get_y() * v.get_y(), u.get_z() * v.get_z()};
  }

  // producto por un escalar t (primero t*v luego v*t)
  inline vector operator*(double t, vector const & v) {
    return {t * v.get_x(), t * v.get_y(), t * v.get_z()};
  }

  inline vector operator*(vector const & v, double t) {
    return t * v;
  }

  // división por un escalar t
  inline vector operator/(vector const & v, double t) {
    return (1 / t) * v;
  }

  // dot product (multiplica cada posición del vector y suma)
  inline double dot(vector const & u, vector const & v) {
    return u.get_x() * v.get_x() + u.get_y() * v.get_y() + u.get_z() * v.get_z();
  }

  // cross product (devuelve el vector perpendicular a los dados)
  inline vector cross(vector const & u, vector const & v) {
    return {u.get_y() * v.get_z() - u.get_z() * v.get_y(),
            u.get_z() * v.get_x() - u.get_x() * v.get_z(),
            u.get_x() * v.get_y() - u.get_y() * v.get_x()};
  }

  // normalización
  inline vector unit_vector(vector const & v) {
    return v / v.magnitude();
  }

}  // namespace render

#endif
