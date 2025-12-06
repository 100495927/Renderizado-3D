// utcommon/test_objects.cpp
#include "../../common/include/object_base.hpp"
#include "../../common/include/objects.hpp"
#include <gtest/gtest.h>
#include <string>

// Tests para Sphere
TEST(SphereTest, DefaultValues) {
  Sphere const sphere;

  EXPECT_EQ(sphere.type, render::SPHERE_TYPE);
  EXPECT_DOUBLE_EQ(sphere.center_x, 0.0);
  EXPECT_DOUBLE_EQ(sphere.center_y, 0.0);
  EXPECT_DOUBLE_EQ(sphere.center_z, 0.0);
  EXPECT_DOUBLE_EQ(sphere.radius, 0.0);
  EXPECT_TRUE(sphere.material_name.empty());
}

TEST(SphereTest, Initialization) {
  Sphere sphere;
  sphere.center_x      = 1.0;
  sphere.center_y      = 2.0;
  sphere.center_z      = 3.0;
  sphere.radius        = 2.5;
  sphere.material_name = "glass";

  EXPECT_EQ(sphere.type, render::SPHERE_TYPE);
  EXPECT_DOUBLE_EQ(sphere.center_x, 1.0);
  EXPECT_DOUBLE_EQ(sphere.center_y, 2.0);
  EXPECT_DOUBLE_EQ(sphere.center_z, 3.0);
  EXPECT_DOUBLE_EQ(sphere.radius, 2.5);
  EXPECT_EQ(sphere.material_name, "glass");
}

TEST(SphereTest, RadiusRange) {
  Sphere sphere;

  // Test diferentes valores de radio
  sphere.radius = 0.1;  // Radio pequeño
  EXPECT_DOUBLE_EQ(sphere.radius, 0.1);

  sphere.radius = 10.0;  // Radio grande
  EXPECT_DOUBLE_EQ(sphere.radius, 10.0);

  sphere.radius = 1.5;  // Radio normal
  EXPECT_DOUBLE_EQ(sphere.radius, 1.5);
}

// Tests para Cylinder
TEST(CylinderTest, DefaultValues) {
  Cylinder const cylinder;

  EXPECT_EQ(cylinder.type, render::CYLINDER_TYPE);
  EXPECT_DOUBLE_EQ(cylinder.center_x, 0.0);
  EXPECT_DOUBLE_EQ(cylinder.center_y, 0.0);
  EXPECT_DOUBLE_EQ(cylinder.center_z, 0.0);
  EXPECT_DOUBLE_EQ(cylinder.radius, 0.0);
  EXPECT_DOUBLE_EQ(cylinder.axis_x, 0.0);
  EXPECT_DOUBLE_EQ(cylinder.axis_y, 0.0);
  EXPECT_DOUBLE_EQ(cylinder.axis_z, 0.0);
  EXPECT_DOUBLE_EQ(cylinder.height, 2.0);
  EXPECT_TRUE(cylinder.material_name.empty());
}

TEST(CylinderTest, Initialization) {
  Cylinder cylinder;
  cylinder.center_x      = 0.0;
  cylinder.center_y      = 0.0;
  cylinder.center_z      = 0.0;
  cylinder.radius        = 1.0;
  cylinder.axis_x        = 0.0;
  cylinder.axis_y        = 1.0;
  cylinder.axis_z        = 0.0;
  cylinder.height        = 3.0;
  cylinder.material_name = "metal";

  EXPECT_EQ(cylinder.type, render::CYLINDER_TYPE);
  EXPECT_DOUBLE_EQ(cylinder.center_x, 0.0);
  EXPECT_DOUBLE_EQ(cylinder.center_y, 0.0);
  EXPECT_DOUBLE_EQ(cylinder.center_z, 0.0);
  EXPECT_DOUBLE_EQ(cylinder.radius, 1.0);
  EXPECT_DOUBLE_EQ(cylinder.axis_x, 0.0);
  EXPECT_DOUBLE_EQ(cylinder.axis_y, 1.0);
  EXPECT_DOUBLE_EQ(cylinder.axis_z, 0.0);
  EXPECT_DOUBLE_EQ(cylinder.height, 3.0);
  EXPECT_EQ(cylinder.material_name, "metal");
}

TEST(CylinderTest, HeightRange) {
  Cylinder cylinder;

  // Test diferentes valores de altura
  cylinder.height = 0.5;  // Altura pequeña
  EXPECT_DOUBLE_EQ(cylinder.height, 0.5);

  cylinder.height = 10.0;  // Altura grande
  EXPECT_DOUBLE_EQ(cylinder.height, 10.0);

  cylinder.height = 2.0;  // Altura por defecto
  EXPECT_DOUBLE_EQ(cylinder.height, 2.0);
}

TEST(CylinderTest, AxisVectors) {
  Cylinder cylinder;

  // Test diferentes vectores de eje
  cylinder.axis_x = 1.0;  // Eje X
  cylinder.axis_y = 0.0;
  cylinder.axis_z = 0.0;
  EXPECT_DOUBLE_EQ(cylinder.axis_x, 1.0);

  cylinder.axis_x = 0.0;  // Eje Y
  cylinder.axis_y = 1.0;
  cylinder.axis_z = 0.0;
  EXPECT_DOUBLE_EQ(cylinder.axis_y, 1.0);

  cylinder.axis_x = 0.0;  // Eje Z
  cylinder.axis_y = 0.0;
  cylinder.axis_z = 1.0;
  EXPECT_DOUBLE_EQ(cylinder.axis_z, 1.0);

  cylinder.axis_x = 1.0;  // Eje diagonal
  cylinder.axis_y = 1.0;
  cylinder.axis_z = 1.0;
  EXPECT_DOUBLE_EQ(cylinder.axis_x, 1.0);
  EXPECT_DOUBLE_EQ(cylinder.axis_y, 1.0);
  EXPECT_DOUBLE_EQ(cylinder.axis_z, 1.0);
}

// Tests de comparación entre objetos
TEST(ObjectsComparisonTest, DifferentTypes) {
  Sphere const sphere;
  Cylinder const cylinder;

  EXPECT_EQ(sphere.type, render::SPHERE_TYPE);
  EXPECT_EQ(cylinder.type, render::CYLINDER_TYPE);
  EXPECT_NE(sphere.type, cylinder.type);
}

TEST(ObjectsComparisonTest, SameTypeDifferentProperties) {
  Sphere sphere1;
  sphere1.center_x = 1.0;
  sphere1.radius   = 2.0;

  Sphere sphere2;
  sphere2.center_x = 3.0;
  sphere2.radius   = 4.0;

  EXPECT_EQ(sphere1.type, sphere2.type);
  EXPECT_NE(sphere1.center_x, sphere2.center_x);
  EXPECT_NE(sphere1.radius, sphere2.radius);
}

// Tests de valores límite/boundary
TEST(ObjectsBoundaryTest, ExtremeValues) {
  // Sphere con valores extremos
  Sphere sphere;
  sphere.center_x = -1000.0;  // Coordenadas extremas
  sphere.center_y = 1000.0;
  sphere.center_z = 0.0;
  sphere.radius   = 0.001;  // Radio muy pequeño

  EXPECT_DOUBLE_EQ(sphere.center_x, -1000.0);
  EXPECT_DOUBLE_EQ(sphere.center_y, 1000.0);
  EXPECT_DOUBLE_EQ(sphere.center_z, 0.0);
  EXPECT_DOUBLE_EQ(sphere.radius, 0.001);

  // Cylinder con valores extremos
  Cylinder cylinder;
  cylinder.radius = 100.0;  // Radio grande
  cylinder.height = 0.01;   // Altura pequeña
  cylinder.axis_x = 0.707;  // Vector normalizado
  cylinder.axis_y = 0.707;
  cylinder.axis_z = 0.0;

  EXPECT_DOUBLE_EQ(cylinder.radius, 100.0);
  EXPECT_DOUBLE_EQ(cylinder.height, 0.01);
  EXPECT_DOUBLE_EQ(cylinder.axis_x, 0.707);
}

// Tests de nombres de materiales
TEST(ObjectsMaterialTest, ValidMaterialNames) {
  Sphere sphere;

  // Nombres válidos de materiales
  sphere.material_name = "glass";
  EXPECT_EQ(sphere.material_name, "glass");

  sphere.material_name = "red_metal_01";
  EXPECT_EQ(sphere.material_name, "red_metal_01");

  sphere.material_name = "a";  // Nombre corto
  EXPECT_EQ(sphere.material_name, "a");

  Cylinder cylinder;
  cylinder.material_name = "wood";
  EXPECT_EQ(cylinder.material_name, "wood");
}

// Test de herencia
TEST(ObjectsInheritanceTest, BaseClass) {
  Sphere const sphere;
  Cylinder const cylinder;

  // Verificar que todos heredan de ObjectBase
  // Esto se verifica implícitamente por la herencia en la definición

  // Podemos verificar que tienen el tipo correcto
  EXPECT_EQ(sphere.type, render::SPHERE_TYPE);
  EXPECT_EQ(cylinder.type, render::CYLINDER_TYPE);
}
