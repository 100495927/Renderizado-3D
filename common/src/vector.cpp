#include "../include/vector.hpp"

#include <cmath>
#include <stdexcept>

namespace render {

  // acceso por índice
  // lectura/escritura de componentes mediante operator[] con comprobación de rango
  double vector::operator[](int i) const {
    switch (i) {
      case 0:  return get_x();
      case 1:  return get_y();
      case 2:  return get_z();
      default: throw std::out_of_range("vector index out of range");
    }
  }

  double & vector::operator[](int i) {
    switch (i) {
      case 0:  return x;
      case 1:  return y;
      case 2:  return z;
      default: throw std::out_of_range("vector index out of range");
    }
  }

  // operadores compuestos
  // definen suma/resta y escalado compuesto para el tipo vector.

  vector & vector::operator+=(vector const & v) {
    x = x + v.get_x();
    y = y + v.get_y();
    z = z + v.get_z();
    return *this;
  }

  vector & vector::operator-=(vector const & v) {
    x = x - v.get_x();
    y = y - v.get_y();
    z = z - v.get_z();
    return *this;
  }

  vector & vector::operator*=(double t) {
    x *= t;
    y *= t;
    z *= t;
    return *this;
  }

  vector & vector::operator/=(double t) {
    return *this *= (1.0 / t);
  }

}  // namespace render
