#include "../../common/include/hit_record.hpp"
#include "../../common/include/ray.hpp"
#include "../../common/include/vector.hpp"
#include <gtest/gtest.h>

// Fixture para tests de HitRecord
class HitRecordTest : public ::testing::Test {
protected:
  void SetUp() override {
    // Configuración común para los tests
    default_intersect = render::point_vector(0.0, 0.0, 0.0);
    default_normal    = render::normal_vector(0.0, 1.0, 0.0);
  }

  render::point_vector default_intersect;
  render::normal_vector default_normal;
};

// Tests de construcción básica
TEST_F(HitRecordTest, DefaultConstructor_InitializesWithDefaultValues) {
  render::hit_record const record;

  EXPECT_NEAR(record.intersect.get_x(), 0.0, 1e-6);
  EXPECT_NEAR(record.intersect.get_y(), 0.0, 1e-6);
  EXPECT_NEAR(record.intersect.get_z(), 0.0, 1e-6);
  EXPECT_NEAR(record.normal.get_x(), 0.0, 1e-6);
  EXPECT_NEAR(record.normal.get_y(), 0.0, 1e-6);
  EXPECT_NEAR(record.normal.get_z(), 0.0, 1e-6);
  EXPECT_EQ(record.mat_pointer, nullptr);
  EXPECT_DOUBLE_EQ(record.t, 0.0);
  EXPECT_FALSE(record.front_face);
}

// Tests de set_face_normal - Casos principales
TEST_F(HitRecordTest, SetFaceNormal_RayAgainstNormal_SetsFrontFaceTrue) {
  render::hit_record record;

  // Rayo viniendo en dirección opuesta a la normal (desde el exterior)
  render::ray const r(render::point_vector(0.0, 0.0, -1.0),
                      render::direction_vector(0.0, 0.0, 1.0));
  render::normal_vector const outward_normal(0.0, 0.0, 1.0);  // Normal apuntando hacia Z positivo

  record.set_face_normal(r, outward_normal);

  EXPECT_TRUE(record.front_face);
  EXPECT_NEAR(record.normal.get_x(), outward_normal.get_x(), 1e-6);
  EXPECT_NEAR(record.normal.get_y(), outward_normal.get_y(), 1e-6);
  EXPECT_NEAR(record.normal.get_z(), outward_normal.get_z(), 1e-6);
}

TEST_F(HitRecordTest, SetFaceNormal_RayWithNormal_SetsFrontFaceFalse) {
  render::hit_record record;

  // Rayo viniendo en la misma dirección que la normal (desde el interior)
  render::ray const r(render::point_vector(0.0, 0.0, 1.0),
                      render::direction_vector(0.0, 0.0, -1.0));
  render::normal_vector const outward_normal(0.0, 0.0, 1.0);  // Normal apuntando hacia Z positivo

  record.set_face_normal(r, outward_normal);

  EXPECT_FALSE(record.front_face);

  EXPECT_NEAR(record.normal.get_x(), 0.0, 1e-6);
  EXPECT_NEAR(record.normal.get_y(), 0.0, 1e-6);
  EXPECT_NEAR(record.normal.get_z(), -1.0, 1e-6);
}

TEST_F(HitRecordTest, SetFaceNormal_PerpendicularRay_SetsFrontFaceFalse) {
  render::hit_record record;

  // Rayo perpendicular a la normal (dot product = 0)
  render::ray const r(render::point_vector(0.0, 0.0, 0.0), render::direction_vector(1.0, 0.0, 0.0));
  render::normal_vector const outward_normal(0.0, 0.0, 1.0);

  record.set_face_normal(r, outward_normal);

  EXPECT_FALSE(record.front_face);  // dot = 0, por lo tanto no es front_face
  EXPECT_NEAR(record.normal.get_x(), 0.0, 1e-6);
  EXPECT_NEAR(record.normal.get_y(), 0.0, 1e-6);
  EXPECT_NEAR(record.normal.get_z(), -1.0, 1e-6);
}

// Tests de casos límite y edge cases
TEST_F(HitRecordTest, SetFaceNormal_WithZeroOutwardNormal_HandlesCorrectly) {
  render::hit_record record;

  render::ray const r(render::point_vector(0.0, 0.0, -1.0),
                      render::direction_vector(0.0, 0.0, 1.0));
  render::normal_vector const zero_normal(0.0, 0.0, 0.0);

  record.set_face_normal(r, zero_normal);

  EXPECT_FALSE(record.front_face);  // dot product con vector cero es 0
  // Normal debe mantenerse como cero
  EXPECT_NEAR(record.normal.get_x(), zero_normal.get_x(), 1e-6);
  EXPECT_NEAR(record.normal.get_y(), zero_normal.get_y(), 1e-6);
  EXPECT_NEAR(record.normal.get_z(), zero_normal.get_z(), 1e-6);
}

TEST_F(HitRecordTest, SetFaceNormal_WithOppositeDirections_VariousAngles) {
  render::hit_record record;

  // Test con varios ángulos entre el rayo y la normal
  render::normal_vector const outward_normal(0.0, 0.0, 1.0);

  // Ángulo agudo (más de 90 grados) - debe ser front_face = false
  render::ray const r1(render::point_vector(0.0, 0.0, 1.0),
                       render::direction_vector(0.0, 0.1, -0.9));
  record.set_face_normal(r1, outward_normal);
  EXPECT_FALSE(record.front_face);

  // Ángulo obtuso (menos de 90 grados) - debe ser front_face = true
  render::ray const r2(render::point_vector(0.0, 0.0, -1.0),
                       render::direction_vector(0.0, 0.1, 0.9));
  record.set_face_normal(r2, outward_normal);
  EXPECT_TRUE(record.front_face);
}

TEST_F(HitRecordTest, SetFaceNormal_NormalizedVectors_MaintainsNormalization) {
  render::hit_record record;

  // Usar vectores normalizados
  render::ray const r(render::point_vector(0.0, 0.0, -1.0),
                      render::direction_vector(0.0, 0.0, 1.0));   // Ya normalizado
  render::normal_vector const outward_normal(0.0, 0.707, 0.707);  // Normalizado aproximado

  record.set_face_normal(r, outward_normal);

  // Verificar que la normal resultante tiene magnitud similar usando la interfaz pública
  double const original_magnitude = outward_normal.magnitude();
  double const result_magnitude   = record.normal.magnitude();

  EXPECT_NEAR(result_magnitude, original_magnitude, 0.001);
}

// Tests de modificación directa de campos
TEST_F(HitRecordTest, DirectFieldAssignment_WorksCorrectly) {
  render::hit_record record;

  // Asignación directa de campos
  record.intersect  = render::point_vector(1.0, 2.0, 3.0);
  record.normal     = render::normal_vector(0.0, 1.0, 0.0);
  record.t          = 5.5;
  record.front_face = true;
  // No asignamos mat_pointer ya que MaterialBase es forward declaration

  // Verificar valores asignados
  EXPECT_NEAR(record.intersect.get_x(), 1.0, 1e-6);
  EXPECT_NEAR(record.intersect.get_y(), 2.0, 1e-6);
  EXPECT_NEAR(record.intersect.get_z(), 3.0, 1e-6);
  EXPECT_NEAR(record.normal.get_x(), 0.0, 1e-6);
  EXPECT_NEAR(record.normal.get_y(), 1.0, 1e-6);
  EXPECT_NEAR(record.normal.get_z(), 0.0, 1e-6);
  EXPECT_DOUBLE_EQ(record.t, 5.5);
  EXPECT_TRUE(record.front_face);
  EXPECT_EQ(record.mat_pointer, nullptr);
}

TEST_F(HitRecordTest, MultipleSetFaceNormalCalls_OverwritePreviousValues) {
  render::hit_record record;

  // Primera llamada
  render::ray const r1(render::point_vector(0.0, 0.0, -1.0),
                       render::direction_vector(0.0, 0.0, 1.0));
  render::normal_vector const normal1(0.0, 0.0, 1.0);
  record.set_face_normal(r1, normal1);

  EXPECT_TRUE(record.front_face);
  EXPECT_NEAR(record.normal.get_x(), normal1.get_x(), 1e-6);
  EXPECT_NEAR(record.normal.get_y(), normal1.get_y(), 1e-6);
  EXPECT_NEAR(record.normal.get_z(), normal1.get_z(), 1e-6);

  // Segunda llamada - debería sobrescribir
  render::ray const r2(render::point_vector(0.0, 0.0, 1.0),
                       render::direction_vector(0.0, 0.0, -1.0));
  render::normal_vector const normal2(1.0, 0.0, 0.0);
  record.set_face_normal(r2, normal2);

  EXPECT_FALSE(record.front_face);
  EXPECT_NEAR(record.normal.get_x(), -1.0, 1e-6);
  EXPECT_NEAR(record.normal.get_y(), 0.0, 1e-6);
  EXPECT_NEAR(record.normal.get_z(), 0.0, 1e-6);
}

// Tests de valores extremos
TEST_F(HitRecordTest, ExtremeTValues_AreHandledCorrectly) {
  render::hit_record record;

  record.t = -1.0;  // T negativo
  EXPECT_DOUBLE_EQ(record.t, -1.0);

  record.t = 1e10;  // T muy grande
  EXPECT_DOUBLE_EQ(record.t, 1e10);

  record.t = 0.0;  // T cero
  EXPECT_DOUBLE_EQ(record.t, 0.0);
}

// Test de integración con otros componentes
TEST_F(HitRecordTest, IntegrationWithRayAndVector_WorksCorrectly) {
  // Crear componentes reales
  render::point_vector const origin(1.0, 2.0, 3.0);
  render::direction_vector const direction(0.0, 0.0, 1.0);
  render::ray const test_ray(origin, direction);

  render::point_vector const intersect_point(1.0, 2.0, 5.0);
  render::normal_vector const surface_normal(0.0, 0.0, 1.0);

  render::hit_record record;
  record.intersect = intersect_point;
  record.t         = 2.0;  // distancia desde origin hasta intersect_point
  record.set_face_normal(test_ray, surface_normal);

  // Verificar integración
  EXPECT_NEAR(record.intersect.get_x(), intersect_point.get_x(), 1e-6);
  EXPECT_NEAR(record.intersect.get_y(), intersect_point.get_y(), 1e-6);
  EXPECT_NEAR(record.intersect.get_z(), intersect_point.get_z(), 1e-6);
  EXPECT_DOUBLE_EQ(record.t, 2.0);
  EXPECT_TRUE(record.front_face);  // Rayo en dirección opuesta a la normal
  EXPECT_NEAR(record.normal.get_x(), surface_normal.get_x(), 1e-6);
  EXPECT_NEAR(record.normal.get_y(), surface_normal.get_y(), 1e-6);
  EXPECT_NEAR(record.normal.get_z(), surface_normal.get_z(), 1e-6);
}

// Test adicional para verificar comportamiento con diferentes tipos de vectores
TEST_F(HitRecordTest, VectorTypeCompatibility_WorksCorrectly) {
  render::hit_record record;

  // Usar diferentes tipos de vectores (todos son alias de vector)
  render::point_vector const position(1.0, 2.0, 3.0);
  render::direction_vector const direction(0.0, 1.0, 0.0);
  render::normal_vector const normal(0.0, 0.0, 1.0);
  render::color_vector const color(0.5, 0.5, 0.5);

  // Asignar diferentes tipos - deberían ser compatibles
  record.intersect = position;
  record.normal    = normal;

  EXPECT_NEAR(record.intersect.get_x(), position.get_x(), 1e-6);
  EXPECT_NEAR(record.intersect.get_y(), position.get_y(), 1e-6);
  EXPECT_NEAR(record.intersect.get_z(), position.get_z(), 1e-6);

  EXPECT_NEAR(record.normal.get_x(), normal.get_x(), 1e-6);
  EXPECT_NEAR(record.normal.get_y(), normal.get_y(), 1e-6);
  EXPECT_NEAR(record.normal.get_z(), normal.get_z(), 1e-6);
}

// Test para verificar que el dot product se calcula correctamente en set_face_normal
TEST_F(HitRecordTest, SetFaceNormal_DotProductCalculation_IsCorrect) {
  render::hit_record record;

  // Caso donde dot product es claramente negativo
  render::ray const r(render::point_vector(0.0, 0.0, -2.0),
                      render::direction_vector(0.0, 0.0, 1.0));
  render::normal_vector const outward_normal(0.0, 0.0, 1.0);

  // dot(r.dir, outward_normal) = (0,0,1) · (0,0,1) = 1 > 0, por lo tanto front_face = false
  // front_face = dot(r.dir, outward_normal) < 0
  // En este caso dot = 1, que NO es < 0, por lo tanto front_face = false

  record.set_face_normal(r, outward_normal);
  EXPECT_FALSE(record.front_face);

  // Caso donde dot product es claramente positivo (direcciones opuestas)
  render::ray const r2(render::point_vector(0.0, 0.0, 2.0),
                       render::direction_vector(0.0, 0.0, -1.0));
  // dot(r2.dir, outward_normal) = (0,0,-1) · (0,0,1) = -1 < 0, por lo tanto front_face = true

  record.set_face_normal(r2, outward_normal);
  EXPECT_TRUE(record.front_face);
}
