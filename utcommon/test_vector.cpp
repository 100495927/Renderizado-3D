#include "../../common/include/vector.hpp"
#include <cmath>
#include <exception>
#include <gtest/gtest.h>
#include <sstream>
#include <stdexcept>

namespace {

  // Fixture para tests de vector
  class VectorTest : public ::testing::Test {
  protected:
    void SetUp() override {
      v1   = render::vector(1.0, 2.0, 3.0);
      v2   = render::vector(4.0, 5.0, 6.0);
      zero = render::vector(0.0, 0.0, 0.0);
    }

    render::vector v1;
    render::vector v2;
    render::vector zero;
  };

  // Tests de constructores
  TEST_F(VectorTest, DefaultConstructor) {
    render::vector const v;
    EXPECT_DOUBLE_EQ(v.get_x(), 0.0);
    EXPECT_DOUBLE_EQ(v.get_y(), 0.0);
    EXPECT_DOUBLE_EQ(v.get_z(), 0.0);
  }

  TEST_F(VectorTest, ParameterizedConstructor) {
    EXPECT_DOUBLE_EQ(v1.get_x(), 1.0);
    EXPECT_DOUBLE_EQ(v1.get_y(), 2.0);
    EXPECT_DOUBLE_EQ(v1.get_z(), 3.0);
  }

  // Tests de getters
  TEST_F(VectorTest, Getters) {
    EXPECT_DOUBLE_EQ(v1.get_x(), 1.0);
    EXPECT_DOUBLE_EQ(v1.get_y(), 2.0);
    EXPECT_DOUBLE_EQ(v1.get_z(), 3.0);
  }

  TEST_F(VectorTest, ColorGetters) {
    EXPECT_DOUBLE_EQ(v1.r(), 1.0);
    EXPECT_DOUBLE_EQ(v1.g(), 2.0);
    EXPECT_DOUBLE_EQ(v1.b(), 3.0);
  }

  // Tests de operadores unarios
  TEST_F(VectorTest, NegationOperator) {
    render::vector const neg = -v1;
    EXPECT_DOUBLE_EQ(neg.get_x(), -1.0);
    EXPECT_DOUBLE_EQ(neg.get_y(), -2.0);
    EXPECT_DOUBLE_EQ(neg.get_z(), -3.0);
  }

  // Tests de acceso por índice
  TEST_F(VectorTest, IndexOperatorRead) {
    EXPECT_DOUBLE_EQ(v1[0], 1.0);
    EXPECT_DOUBLE_EQ(v1[1], 2.0);
    EXPECT_DOUBLE_EQ(v1[2], 3.0);
  }

  TEST_F(VectorTest, IndexOperatorWrite) {
    render::vector v(1.0, 2.0, 3.0);
    v[0] = 10.0;
    v[1] = 20.0;
    v[2] = 30.0;

    EXPECT_DOUBLE_EQ(v.get_x(), 10.0);
    EXPECT_DOUBLE_EQ(v.get_y(), 20.0);
    EXPECT_DOUBLE_EQ(v.get_z(), 30.0);
  }

  TEST_F(VectorTest, IndexOperatorOutOfRange) {
    EXPECT_THROW(v1[-1], std::out_of_range);
    EXPECT_THROW(v1[3], std::out_of_range);
    EXPECT_THROW(v1[100], std::out_of_range);
  }

  // Tests de operadores compuestos
  TEST_F(VectorTest, PlusEqualsOperator) {
    v1 += v2;
    EXPECT_DOUBLE_EQ(v1.get_x(), 5.0);
    EXPECT_DOUBLE_EQ(v1.get_y(), 7.0);
    EXPECT_DOUBLE_EQ(v1.get_z(), 9.0);
  }

  TEST_F(VectorTest, MinusEqualsOperator) {
    v1 -= v2;
    EXPECT_DOUBLE_EQ(v1.get_x(), -3.0);
    EXPECT_DOUBLE_EQ(v1.get_y(), -3.0);
    EXPECT_DOUBLE_EQ(v1.get_z(), -3.0);
  }

  TEST_F(VectorTest, MultiplyEqualsOperator) {
    v1 *= 2.0;
    EXPECT_DOUBLE_EQ(v1.get_x(), 2.0);
    EXPECT_DOUBLE_EQ(v1.get_y(), 4.0);
    EXPECT_DOUBLE_EQ(v1.get_z(), 6.0);
  }

  TEST_F(VectorTest, DivideEqualsOperator) {
    v1 /= 2.0;
    EXPECT_DOUBLE_EQ(v1.get_x(), 0.5);
    EXPECT_DOUBLE_EQ(v1.get_y(), 1.0);
    EXPECT_DOUBLE_EQ(v1.get_z(), 1.5);
  }

  TEST_F(VectorTest, DivideByZero) {
    EXPECT_THROW(v1 /= 0.0, std::exception);  // Division por cero puede causar problemas
  }

  // Tests de magnitudes
  TEST_F(VectorTest, MagnitudeSquared) {
    EXPECT_DOUBLE_EQ(v1.magnitude_squared(), 14.0);  // 1² + 2² + 3² = 14
  }

  TEST_F(VectorTest, Magnitude) {
    EXPECT_DOUBLE_EQ(v1.magnitude(), std::sqrt(14.0));
    EXPECT_DOUBLE_EQ(zero.magnitude(), 0.0);
  }

  TEST_F(VectorTest, NearZero) {
    render::vector const small(1e-9, 1e-9, 1e-9);
    render::vector const not_small(1e-7, 1e-9, 1e-9);

    EXPECT_TRUE(small.near_zero());
    EXPECT_TRUE(zero.near_zero());
    EXPECT_FALSE(not_small.near_zero());
    EXPECT_FALSE(v1.near_zero());
  }

  // Tests de operadores globales
  TEST_F(VectorTest, AdditionOperator) {
    render::vector const result = v1 + v2;
    EXPECT_DOUBLE_EQ(result.get_x(), 5.0);
    EXPECT_DOUBLE_EQ(result.get_y(), 7.0);
    EXPECT_DOUBLE_EQ(result.get_z(), 9.0);
  }

  TEST_F(VectorTest, SubtractionOperator) {
    render::vector const result = v1 - v2;
    EXPECT_DOUBLE_EQ(result.get_x(), -3.0);
    EXPECT_DOUBLE_EQ(result.get_y(), -3.0);
    EXPECT_DOUBLE_EQ(result.get_z(), -3.0);
  }

  TEST_F(VectorTest, HadamardProduct) {
    render::vector const result = v1 * v2;
    EXPECT_DOUBLE_EQ(result.get_x(), 4.0);
    EXPECT_DOUBLE_EQ(result.get_y(), 10.0);
    EXPECT_DOUBLE_EQ(result.get_z(), 18.0);
  }

  TEST_F(VectorTest, ScalarMultiplication) {
    render::vector const result1 = 2.0 * v1;
    render::vector const result2 = v1 * 2.0;

    EXPECT_DOUBLE_EQ(result1.get_x(), 2.0);
    EXPECT_DOUBLE_EQ(result1.get_y(), 4.0);
    EXPECT_DOUBLE_EQ(result1.get_z(), 6.0);

    EXPECT_DOUBLE_EQ(result2.get_x(), 2.0);
    EXPECT_DOUBLE_EQ(result2.get_y(), 4.0);
    EXPECT_DOUBLE_EQ(result2.get_z(), 6.0);
  }

  TEST_F(VectorTest, ScalarDivision) {
    render::vector const result = v1 / 2.0;
    EXPECT_DOUBLE_EQ(result.get_x(), 0.5);
    EXPECT_DOUBLE_EQ(result.get_y(), 1.0);
    EXPECT_DOUBLE_EQ(result.get_z(), 1.5);
  }

  TEST_F(VectorTest, DotProduct) {
    double const dot_result = render::dot(v1, v2);
    EXPECT_DOUBLE_EQ(dot_result, 32.0);  // 1*4 + 2*5 + 3*6 = 32
  }

  TEST_F(VectorTest, CrossProduct) {
    render::vector const result = render::cross(v1, v2);

    // v1 × v2 = (2*6 - 3*5, 3*4 - 1*6, 1*5 - 2*4) = (-3, 6, -3)
    EXPECT_DOUBLE_EQ(result.get_x(), -3.0);
    EXPECT_DOUBLE_EQ(result.get_y(), 6.0);
    EXPECT_DOUBLE_EQ(result.get_z(), -3.0);

    // Verificar propiedad: v1 × v2 = - (v2 × v1)
    render::vector const reverse = render::cross(v2, v1);
    EXPECT_DOUBLE_EQ(result.get_x(), -reverse.get_x());
    EXPECT_DOUBLE_EQ(result.get_y(), -reverse.get_y());
    EXPECT_DOUBLE_EQ(result.get_z(), -reverse.get_z());
  }

  TEST_F(VectorTest, UnitVector) {
    render::vector const unit = render::unit_vector(v1);
    double const magnitude    = unit.magnitude();

    EXPECT_NEAR(magnitude, 1.0, 1e-10);
    EXPECT_DOUBLE_EQ(unit.get_x(), 1.0 / std::sqrt(14.0));
    EXPECT_DOUBLE_EQ(unit.get_y(), 2.0 / std::sqrt(14.0));
    EXPECT_DOUBLE_EQ(unit.get_z(), 3.0 / std::sqrt(14.0));
  }

  TEST_F(VectorTest, UnitVectorOfZero) {
    // Unit vector de cero debería manejar el caso especial
    render::vector const unit = render::unit_vector(zero);
    // Depende de la implementación, podría devolver cero o lanzar excepción
    // Por ahora asumimos que devuelve cero
    EXPECT_DOUBLE_EQ(unit.magnitude(), 0.0);
  }

  // Test de output stream
  TEST_F(VectorTest, OutputStream) {
    std::ostringstream oss;
    oss << v1;
    EXPECT_EQ(oss.str(), "1 2 3");
  }

  // Tests de alias
  TEST_F(VectorTest, TypeAliases) {
    render::point_vector const p(1.0, 2.0, 3.0);
    render::color_vector const c(0.5, 0.5, 0.5);
    render::direction_vector const d(0.0, 1.0, 0.0);
    render::normal_vector const n(0.0, 0.0, 1.0);

    EXPECT_DOUBLE_EQ(p.get_x(), 1.0);
    EXPECT_DOUBLE_EQ(c.r(), 0.5);
    EXPECT_DOUBLE_EQ(d.get_y(), 1.0);
    EXPECT_DOUBLE_EQ(n.get_z(), 1.0);
  }

}  // namespace
