#include "../../common/include/ray.hpp"
#include "../../common/include/vector.hpp"
#include <gtest/gtest.h>
#include <numbers>

// Fixture para tests de Ray
class RayTest : public ::testing::Test {
protected:
  void SetUp() override {
    // Configuración común para los tests
    default_origin    = render::point_vector(0.0, 0.0, 0.0);
    default_direction = render::direction_vector(1.0, 0.0, 0.0);
    custom_origin     = render::point_vector(1.0, 2.0, 3.0);
    custom_direction  = render::direction_vector(0.0, 1.0, 0.0);
  }

  render::point_vector default_origin;
  render::direction_vector default_direction;
  render::point_vector custom_origin;
  render::direction_vector custom_direction;
};

// Tests de construcción básica
TEST_F(RayTest, DefaultConstructor_InitializesWithDefaultVectors) {
  render::ray const r;

  EXPECT_NEAR(r.orig.get_x(), 0.0, 1e-6);
  EXPECT_NEAR(r.orig.get_y(), 0.0, 1e-6);
  EXPECT_NEAR(r.orig.get_z(), 0.0, 1e-6);

  EXPECT_NEAR(r.dir.get_x(), 0.0, 1e-6);
  EXPECT_NEAR(r.dir.get_y(), 0.0, 1e-6);
  EXPECT_NEAR(r.dir.get_z(), 0.0, 1e-6);
}

TEST_F(RayTest, ParameterizedConstructor_SetsOriginAndDirection) {
  render::ray const r(custom_origin, custom_direction);

  EXPECT_NEAR(r.orig.get_x(), custom_origin.get_x(), 1e-6);
  EXPECT_NEAR(r.orig.get_y(), custom_origin.get_y(), 1e-6);
  EXPECT_NEAR(r.orig.get_z(), custom_origin.get_z(), 1e-6);

  EXPECT_NEAR(r.dir.get_x(), custom_direction.get_x(), 1e-6);
  EXPECT_NEAR(r.dir.get_y(), custom_direction.get_y(), 1e-6);
  EXPECT_NEAR(r.dir.get_z(), custom_direction.get_z(), 1e-6);
}

// Test de la función at() con el rayo por defecto (dirección (0,0,0))
TEST_F(RayTest, DefaultConstructor_AtZeroDirection) {
  render::ray const r;

  EXPECT_NEAR(r.at(0.0).get_x(), default_origin.get_x(), 1e-6);
  EXPECT_NEAR(r.at(0.0).get_y(), default_origin.get_y(), 1e-6);
  EXPECT_NEAR(r.at(0.0).get_z(), default_origin.get_z(), 1e-6);

  EXPECT_NEAR(r.at(1.0).get_x(), default_origin.get_x(), 1e-6);
  EXPECT_NEAR(r.at(1.0).get_y(), default_origin.get_y(), 1e-6);
  EXPECT_NEAR(r.at(1.0).get_z(), default_origin.get_z(), 1e-6);

  EXPECT_NEAR(r.at(-1.0).get_x(), default_origin.get_x(), 1e-6);
  EXPECT_NEAR(r.at(-1.0).get_y(), default_origin.get_y(), 1e-6);
  EXPECT_NEAR(r.at(-1.0).get_z(), default_origin.get_z(), 1e-6);

  EXPECT_NEAR(r.at(100.0).get_x(), default_origin.get_x(), 1e-6);
  EXPECT_NEAR(r.at(100.0).get_y(), default_origin.get_y(), 1e-6);
  EXPECT_NEAR(r.at(100.0).get_z(), default_origin.get_z(), 1e-6);
}

TEST_F(RayTest, At_WithUnitVectors_DifferentAxes) {
  render::ray const rx(default_origin, render::direction_vector(1.0, 0.0, 0.0));

  EXPECT_NEAR(rx.at(2.0).get_x(), 2.0, 1e-6);
  EXPECT_NEAR(rx.at(2.0).get_y(), 0.0, 1e-6);
  EXPECT_NEAR(rx.at(2.0).get_z(), 0.0, 1e-6);

  render::ray const ry(default_origin, render::direction_vector(0.0, 1.0, 0.0));
  EXPECT_NEAR(ry.at(2.0).get_x(), 0.0, 1e-6);
  EXPECT_NEAR(ry.at(2.0).get_y(), 2.0, 1e-6);
  EXPECT_NEAR(ry.at(2.0).get_z(), 0.0, 1e-6);

  render::ray const rz(default_origin, render::direction_vector(0.0, 0.0, 1.0));
  EXPECT_NEAR(rz.at(2.0).get_x(), 0.0, 1e-6);
  EXPECT_NEAR(rz.at(2.0).get_y(), 0.0, 1e-6);
  EXPECT_NEAR(rz.at(2.0).get_z(), 2.0, 1e-6);
}

TEST_F(RayTest, At_WithNonNormalizedDirection_ScalesCorrectly) {
  render::ray const r(default_origin, render::direction_vector(2.0, 0.0, 0.0));

  render::point_vector const result = r.at(1.0);
  render::point_vector const expected(2.0, 0.0, 0.0);

  EXPECT_NEAR(result.get_x(), expected.get_x(), 1e-6);
  EXPECT_NEAR(result.get_y(), expected.get_y(), 1e-6);
  EXPECT_NEAR(result.get_z(), expected.get_z(), 1e-6);
}

// Tests de valores extremos
TEST_F(RayTest, At_VeryLargeT_ReturnsCorrectPosition) {
  render::ray const r(default_origin, default_direction);

  render::point_vector const result = r.at(1e10);
  render::point_vector const expected(1e10, 0.0, 0.0);

  EXPECT_NEAR(result.get_x(), expected.get_x(), 1e-6);
  EXPECT_NEAR(result.get_y(), expected.get_y(), 1e-6);
  EXPECT_NEAR(result.get_z(), expected.get_z(), 1e-6);
}

TEST_F(RayTest, At_VerySmallT_ReturnsCorrectPosition) {
  render::ray const r(default_origin, default_direction);

  render::point_vector const result = r.at(1e-10);
  render::point_vector const expected(1e-10, 0.0, 0.0);

  // Usar EXPECT_NEAR para comparación de punto flotante con valores muy pequeños
  EXPECT_NEAR(result.get_x(), 1e-10, 1e-20);
  EXPECT_NEAR(result.get_y(), 0.0, 1e-20);
  EXPECT_NEAR(result.get_z(), 0.0, 1e-20);
}

TEST_F(RayTest, At_VeryLargeNegativeT_ReturnsCorrectPosition) {
  render::ray const r(default_origin, default_direction);

  render::point_vector const result = r.at(-1e10);
  render::point_vector const expected(-1e10, 0.0, 0.0);

  EXPECT_NEAR(result.get_x(), expected.get_x(), 1e-6);
  EXPECT_NEAR(result.get_y(), expected.get_y(), 1e-6);
  EXPECT_NEAR(result.get_z(), expected.get_z(), 1e-6);
}

// Tests de modificación directa de campos
TEST_F(RayTest, DirectFieldModification_WorksCorrectly) {
  render::ray r;

  // Modificar campos directamente
  r.orig = custom_origin;
  r.dir  = custom_direction;

  EXPECT_NEAR(r.orig.get_x(), custom_origin.get_x(), 1e-6);
  EXPECT_NEAR(r.orig.get_y(), custom_origin.get_y(), 1e-6);
  EXPECT_NEAR(r.orig.get_z(), custom_origin.get_z(), 1e-6);

  EXPECT_NEAR(r.dir.get_x(), custom_direction.get_x(), 1e-6);
  EXPECT_NEAR(r.dir.get_y(), custom_direction.get_y(), 1e-6);
  EXPECT_NEAR(r.dir.get_z(), custom_direction.get_z(), 1e-6);

  // Verificar que at() usa los nuevos valores
  render::point_vector const result = r.at(2.0);
  render::point_vector const expected(1.0, 4.0, 3.0);  // (1,2,3) + 2*(0,1,0)

  EXPECT_NEAR(result.get_x(), expected.get_x(), 1e-6);
  EXPECT_NEAR(result.get_y(), expected.get_y(), 1e-6);
  EXPECT_NEAR(result.get_z(), expected.get_z(), 1e-6);
}

// Tests de comportamiento con diferentes configuraciones
TEST_F(RayTest, MultipleRays_IndependentBehavior) {
  render::ray const r1(render::point_vector(0.0, 0.0, 0.0),
                       render::direction_vector(1.0, 0.0, 0.0));
  render::ray const r2(render::point_vector(1.0, 0.0, 0.0),
                       render::direction_vector(0.0, 1.0, 0.0));

  // r1.at(1.0)
  EXPECT_NEAR(r1.at(1.0).get_x(), 1.0, 1e-6);
  EXPECT_NEAR(r1.at(1.0).get_y(), 0.0, 1e-6);
  EXPECT_NEAR(r1.at(1.0).get_z(), 0.0, 1e-6);

  // r2.at(1.0)
  EXPECT_NEAR(r2.at(1.0).get_x(), 1.0, 1e-6);
  EXPECT_NEAR(r2.at(1.0).get_y(), 1.0, 1e-6);
  EXPECT_NEAR(r2.at(1.0).get_z(), 0.0, 1e-6);
}

TEST_F(RayTest, Ray_WithOppositeDirection) {
  render::ray const r(default_origin, render::direction_vector(-1.0, 0.0, 0.0));

  // r.at(1.0)
  EXPECT_NEAR(r.at(1.0).get_x(), -1.0, 1e-6);
  EXPECT_NEAR(r.at(1.0).get_y(), 0.0, 1e-6);
  EXPECT_NEAR(r.at(1.0).get_z(), 0.0, 1e-6);

  // r.at(-1.0)
  EXPECT_NEAR(r.at(-1.0).get_x(), 1.0, 1e-6);
  EXPECT_NEAR(r.at(-1.0).get_y(), 0.0, 1e-6);
  EXPECT_NEAR(r.at(-1.0).get_z(), 0.0, 1e-6);
}

// Test de integración con vector operations
TEST_F(RayTest, Integration_WithVectorOperations) {
  render::point_vector const origin(1.0, 2.0, 3.0);
  render::direction_vector const direction =
      render::unit_vector(render::direction_vector(1.0, 1.0, 1.0));

  render::ray const r(origin, direction);

  // Verificar que at() funciona correctamente con vectores normalizados
  render::point_vector const result = r.at(std::numbers::sqrt3);  // Distancia para llegar a (2,3,4)

  // (1,2,3) + sqrt(3) * (1/√3, 1/√3, 1/√3) = (1,2,3) + (1,1,1) = (2,3,4)
  EXPECT_NEAR(result.get_x(), 2.0, 1e-10);
  EXPECT_NEAR(result.get_y(), 3.0, 1e-10);
  EXPECT_NEAR(result.get_z(), 4.0, 1e-10);
}

// Test para verificar que at() es const
TEST_F(RayTest, At_MethodIsConst) {
  render::ray const r(default_origin, default_direction);

  render::point_vector const result = r.at(1.0);

  EXPECT_NEAR(result.get_x(), 1.0, 1e-6);
  EXPECT_NEAR(result.get_y(), 0.0, 1e-6);
  EXPECT_NEAR(result.get_z(), 0.0, 1e-6);
}

// Test de copy y assignment (si es necesario, aunque la clase usa comportamiento por defecto)
TEST_F(RayTest, CopyConstructor_CreatesIndependentCopy) {
  render::ray original(custom_origin, custom_direction);
  render::ray const copy(original);

  EXPECT_NEAR(copy.orig.get_x(), original.orig.get_x(), 1e-6);
  EXPECT_NEAR(copy.orig.get_y(), original.orig.get_y(), 1e-6);
  EXPECT_NEAR(copy.orig.get_z(), original.orig.get_z(), 1e-6);

  EXPECT_NEAR(copy.dir.get_x(), original.dir.get_x(), 1e-6);
  EXPECT_NEAR(copy.dir.get_y(), original.dir.get_y(), 1e-6);
  EXPECT_NEAR(copy.dir.get_z(), original.dir.get_z(), 1e-6);

  // Modificar el original no debería afectar la copia
  original.orig = default_origin;

  EXPECT_NEAR(copy.orig.get_x(), custom_origin.get_x(), 1e-6);  // La copia mantiene sus valores
  EXPECT_NEAR(copy.orig.get_y(), custom_origin.get_y(), 1e-6);
  EXPECT_NEAR(copy.orig.get_z(), custom_origin.get_z(), 1e-6);
}

TEST_F(RayTest, AssignmentOperator_CreatesIndependentCopy) {
  render::ray original(custom_origin, custom_direction);
  render::ray assigned;

  assigned = original;

  EXPECT_NEAR(assigned.orig.get_x(), original.orig.get_x(), 1e-6);
  EXPECT_NEAR(assigned.orig.get_y(), original.orig.get_y(), 1e-6);
  EXPECT_NEAR(assigned.orig.get_z(), original.orig.get_z(), 1e-6);

  EXPECT_NEAR(assigned.dir.get_x(), original.dir.get_x(), 1e-6);
  EXPECT_NEAR(assigned.dir.get_y(), original.dir.get_y(), 1e-6);
  EXPECT_NEAR(assigned.dir.get_z(), original.dir.get_z(), 1e-6);

  // Modificar el original no debería afectar el asignado
  original.orig = default_origin;

  EXPECT_NEAR(assigned.orig.get_x(), custom_origin.get_x(), 1e-6);
  EXPECT_NEAR(assigned.orig.get_y(), custom_origin.get_y(), 1e-6);
  EXPECT_NEAR(assigned.orig.get_z(), custom_origin.get_z(), 1e-6);
}
