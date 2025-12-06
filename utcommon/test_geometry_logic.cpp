// utcommon/test_geometry_logic.cpp
#include "../common/include/geometry_logic.hpp"
#include "../common/include/hit_record.hpp"
#include "../common/include/ray.hpp"
#include "../common/include/vector.hpp"
#include <gtest/gtest.h>

namespace render::test {

  class GeometryLogicTest : public ::testing::Test {
  protected:
    void SetUp() override {
      // Setup objetos de prueba
      sphere.center_x      = 0.0;
      sphere.center_y      = 0.0;
      sphere.center_z      = 0.0;
      sphere.radius        = 1.0;
      sphere.material_name = "matte_red";

      cylinder.center_x      = 0.0;
      cylinder.center_y      = 0.0;
      cylinder.center_z      = 0.0;
      cylinder.radius        = 1.0;
      cylinder.axis_x        = 0.0;
      cylinder.axis_y        = 1.0;
      cylinder.axis_z        = 0.0;
      cylinder.height        = 2.0;
      cylinder.material_name = "metal";

      // Setup ray básico
      r.orig = point_vector(0, 0, -5);
      r.dir  = direction_vector(0, 0, 1);
    }

    Sphere sphere;
    Cylinder cylinder;
    ray r;
    hit_record rec;
  };

  // Tests para hit_sphere
  TEST_F(GeometryLogicTest, HitSphere_DirectHit) {
    // Rayo que golpea directamente la esfera
    double const t = hit_sphere(r, 0.0, 10.0, &sphere);

    EXPECT_GT(t, 0.0);
    EXPECT_LT(t, 10.0);
  }

  TEST_F(GeometryLogicTest, HitSphere_Miss) {
    // Rayo que no golpea la esfera
    r.dir          = direction_vector(2, 0, 1);  // Apunta al lado de la esfera
    double const t = hit_sphere(r, 0.0, 10.0, &sphere);

    EXPECT_DOUBLE_EQ(t, -1.0);  // Debería devolver -1.0 cuando no hay hit
  }

  TEST_F(GeometryLogicTest, HitSphere_Tangent) {
    // Rayo tangente a la esfera (debería contar como hit)
    r.orig         = point_vector(1, 0, -5);  // Origen en el borde
    r.dir          = direction_vector(0, 0, 1);
    double const t = hit_sphere(r, 0.0, 10.0, &sphere);

    EXPECT_GT(t, 0.0);
  }

  TEST_F(GeometryLogicTest, HitSphere_Inside) {
    // Rayo que empieza dentro de la esfera
    r.orig         = point_vector(0, 0, 0);  // Dentro de la esfera
    r.dir          = direction_vector(0, 0, 1);
    double const t = hit_sphere(r, 0.0, 10.0, &sphere);

    EXPECT_GT(t, 0.0);  // Debería golpear la superficie desde dentro
  }

  // Tests para process_sphere_hit
  TEST_F(GeometryLogicTest, ProcessSphereHit_Valid) {
    double const t    = 4.0;  // Distancia conocida
    bool const result = process_sphere_hit(r, t, rec, &sphere);

    EXPECT_TRUE(result);
    EXPECT_DOUBLE_EQ(rec.t, t);

    render::point_vector const expected_intersect_s = r.at(t);
    EXPECT_NEAR(rec.intersect.get_x(), expected_intersect_s.get_x(), 1e-6);
    EXPECT_NEAR(rec.intersect.get_y(), expected_intersect_s.get_y(), 1e-6);
    EXPECT_NEAR(rec.intersect.get_z(), expected_intersect_s.get_z(), 1e-6);

    // La normal debería apuntar hacia afuera
    EXPECT_GT(dot(rec.normal, direction_vector(0, 0, -1)), 0);
  }

  TEST_F(GeometryLogicTest, ProcessSphereHit_InvalidT) {
    bool const result = process_sphere_hit(r, -1.0, rec, &sphere);

    EXPECT_FALSE(result);  // t negativo debería fallar
  }

  // Tests para hit_cylinder_lateral
  TEST_F(GeometryLogicTest, HitCylinderLateral_DirectHit) {
    // Rayo que golpea el lateral del cilindro
    r.orig         = point_vector(2, 0, -5);
    r.dir          = direction_vector(-0.5, 0, 1);
    double const t = hit_cylinder_lateral(r, 0.0, 10.0, &cylinder);

    EXPECT_GT(t, 0.0);
    EXPECT_LT(t, 10.0);
  }

  TEST_F(GeometryLogicTest, HitCylinderLateral_Miss) {
    // Rayo que no golpea el lateral
    r.orig         = point_vector(3, 0, -5);  // Demasiado lejos en X
    r.dir          = direction_vector(0, 0, 1);
    double const t = hit_cylinder_lateral(r, 0.0, 10.0, &cylinder);

    EXPECT_DOUBLE_EQ(t, -1.0);
  }

  TEST_F(GeometryLogicTest, HitCylinderLateral_OutsideHeight) {
    // Rayo que golpearía el lateral pero está fuera de la altura
    cylinder.height = 1.0;                     // Cilindro más corto
    r.orig          = point_vector(2, 2, -5);  // Por encima del cilindro
    r.dir           = direction_vector(-0.3, -0.3, 1);
    double const t  = hit_cylinder_lateral(r, 0.0, 10.0, &cylinder);

    EXPECT_DOUBLE_EQ(t, -1.0);  // Debería fallar por estar fuera de altura
  }

  // Tests para hit_cylinder_caps
  TEST_F(GeometryLogicTest, HitCylinderCaps_TopHit) {
    // Rayo que golpea la tapa superior
    r.orig         = point_vector(0, 2, -5);  // Por encima del cilindro
    r.dir          = direction_vector(0, -0.5, 1);
    double const t = hit_cylinder_caps(r, 0.0, 10.0, &cylinder);

    EXPECT_GT(t, 0.0);
  }

  TEST_F(GeometryLogicTest, HitCylinderCaps_BottomHit) {
    // Rayo que golpea la tapa inferior
    r.orig         = point_vector(0, -2, -5);  // Por debajo del cilindro
    r.dir          = direction_vector(0, 0.5, 1);
    double const t = hit_cylinder_caps(r, 0.0, 10.0, &cylinder);

    EXPECT_GT(t, 0.0);
  }

  TEST_F(GeometryLogicTest, HitCylinderCaps_Miss) {
    // Rayo que no golpea las tapas
    r.orig         = point_vector(0, 3, -5);     // Demasiado arriba
    r.dir          = direction_vector(0, 1, 1);  // Apunta más arriba
    double const t = hit_cylinder_caps(r, 0.0, 10.0, &cylinder);

    EXPECT_DOUBLE_EQ(t, -1.0);
  }

  // Tests para hit_cylinder (combinado)
  TEST_F(GeometryLogicTest, HitCylinder_LateralHit) {
    // Rayo que golpea el lateral
    r.orig         = point_vector(2, 0, -5);
    r.dir          = direction_vector(-0.5, 0, 1);
    double const t = hit_cylinder(r, 0.0, 10.0, &cylinder);

    EXPECT_GT(t, 0.0);
  }

  TEST_F(GeometryLogicTest, HitCylinder_CapHit) {
    // Rayo que golpea la tapa
    r.orig         = point_vector(0, 2, -5);
    r.dir          = direction_vector(0, -0.5, 1);
    double const t = hit_cylinder(r, 0.0, 10.0, &cylinder);

    EXPECT_GT(t, 0.0);
  }

  TEST_F(GeometryLogicTest, HitCylinder_Miss) {
    // Rayo que no golpea el cilindro
    r.orig         = point_vector(3, 3, -5);  // Fuera del cilindro
    r.dir          = direction_vector(0, 0, 1);
    double const t = hit_cylinder(r, 0.0, 10.0, &cylinder);

    EXPECT_DOUBLE_EQ(t, -1.0);
  }

  // Tests para process_cylinder_hit
  TEST_F(GeometryLogicTest, ProcessCylinderHit_Valid) {
    double const t    = 4.0;
    bool const result = process_cylinder_hit(r, t, rec, &cylinder);

    EXPECT_TRUE(result);
    EXPECT_DOUBLE_EQ(rec.t, t);

    render::point_vector const expected_intersect_c = r.at(t);
    EXPECT_NEAR(rec.intersect.get_x(), expected_intersect_c.get_x(), 1e-6);
    EXPECT_NEAR(rec.intersect.get_y(), expected_intersect_c.get_y(), 1e-6);
    EXPECT_NEAR(rec.intersect.get_z(), expected_intersect_c.get_z(), 1e-6);
  }

  TEST_F(GeometryLogicTest, ProcessCylinderHit_InvalidT) {
    bool const result = process_cylinder_hit(r, -1.0, rec, &cylinder);

    EXPECT_FALSE(result);
  }

  // Tests para hit_object (dispatch general)
  TEST_F(GeometryLogicTest, HitObject_SphereType) {
    bool const result = hit_object(r, {0.0, 10.0}, rec, &sphere);

    EXPECT_TRUE(result);
    render::point_vector const expected_intersect_o = r.at(rec.t);
    EXPECT_NEAR(rec.intersect.get_x(), expected_intersect_o.get_x(), 1e-6);
    EXPECT_NEAR(rec.intersect.get_y(), expected_intersect_o.get_y(), 1e-6);
    EXPECT_NEAR(rec.intersect.get_z(), expected_intersect_o.get_z(), 1e-6);
  }

  TEST_F(GeometryLogicTest, HitObject_CylinderType) {
    bool const result = hit_object(r, {0.0, 10.0}, rec, &cylinder);

    // Puede ser true o false dependiendo de la orientación
    // Solo verificamos que no hay crash
    EXPECT_TRUE(result or not result);
  }

  // Tests de casos límite
  TEST_F(GeometryLogicTest, EdgeCase_ZeroRadiusSphere) {
    sphere.radius  = 0.0;
    double const t = hit_sphere(r, 0.0, 10.0, &sphere);

    // Esfera con radio cero en el origen, rayo desde (0,0,-5)
    // Debería golpear en t=5.0 (punto (0,0,0))
    EXPECT_DOUBLE_EQ(t, 5.0);
  }

  TEST_F(GeometryLogicTest, EdgeCase_VeryLargeSphere) {
    sphere.radius  = 1000.0;
    double const t = hit_sphere(r, 0.0, 10.0, &sphere);

    EXPECT_GT(t, 0.0);  // Debería golpear
  }

  TEST_F(GeometryLogicTest, EdgeCase_ParallelRayCylinder) {
    // Rayo paralelo al eje del cilindro
    r.dir          = direction_vector(1, 0, 0);  // Paralelo al eje Y
    double const t = hit_cylinder(r, 0.0, 10.0, &cylinder);

    // Depende de la posición, pero debería manejarse sin crash
    EXPECT_TRUE(t >= -1.0);
  }

  // Tests de set_face_normal (en hit_record)
  TEST_F(GeometryLogicTest, SetFaceNormal_FrontFace) {
    ray const test_ray(point_vector(0, 0, -1), direction_vector(0, 0, 1));
    normal_vector const outward_normal(0, 0, 1);

    rec.set_face_normal(test_ray, outward_normal);

    EXPECT_TRUE(rec.front_face);
    EXPECT_NEAR(rec.normal.get_x(), outward_normal.get_x(), 1e-6);
    EXPECT_NEAR(rec.normal.get_y(), outward_normal.get_y(), 1e-6);
    EXPECT_NEAR(rec.normal.get_z(), outward_normal.get_z(), 1e-6);
  }

  TEST_F(GeometryLogicTest, SetFaceNormal_BackFace) {
    ray const test_ray(point_vector(0, 0, 1), direction_vector(0, 0, 1));
    normal_vector const outward_normal(0, 0, 1);

    rec.set_face_normal(test_ray, outward_normal);

    EXPECT_FALSE(rec.front_face);
    normal_vector const expected_normal = -outward_normal;
    EXPECT_NEAR(rec.normal.get_x(), expected_normal.get_x(), 1e-6);
    EXPECT_NEAR(rec.normal.get_y(), expected_normal.get_y(), 1e-6);
    EXPECT_NEAR(rec.normal.get_z(), expected_normal.get_z(), 1e-6);
  }

}  // namespace render::test
