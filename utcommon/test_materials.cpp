#include "../../common/include/material_base.hpp"
#include "../../common/include/materials.hpp"
#include <gtest/gtest.h>
#include <string>

// Tests para MatteMaterial
TEST(MatteMaterialTest, DefaultValues) {
  MatteMaterial const mat;

  EXPECT_EQ(mat.type, render::MATTE_TYPE);
  EXPECT_TRUE(mat.name.empty());
  EXPECT_DOUBLE_EQ(mat.reflectance_r, 0.0);
  EXPECT_DOUBLE_EQ(mat.reflectance_g, 0.0);
  EXPECT_DOUBLE_EQ(mat.reflectance_b, 0.0);
}

TEST(MatteMaterialTest, Initialization) {
  MatteMaterial mat;
  mat.name          = "red_matte";
  mat.reflectance_r = 0.8;
  mat.reflectance_g = 0.2;
  mat.reflectance_b = 0.2;

  EXPECT_EQ(mat.type, render::MATTE_TYPE);
  EXPECT_EQ(mat.name, "red_matte");
  EXPECT_DOUBLE_EQ(mat.reflectance_r, 0.8);
  EXPECT_DOUBLE_EQ(mat.reflectance_g, 0.2);
  EXPECT_DOUBLE_EQ(mat.reflectance_b, 0.2);
}

TEST(MatteMaterialTest, ReflectanceRange) {
  MatteMaterial mat;

  // Test valores válidos en rango [0,1]
  mat.reflectance_r = 0.0;
  mat.reflectance_g = 0.5;
  mat.reflectance_b = 1.0;

  EXPECT_DOUBLE_EQ(mat.reflectance_r, 0.0);
  EXPECT_DOUBLE_EQ(mat.reflectance_g, 0.5);
  EXPECT_DOUBLE_EQ(mat.reflectance_b, 1.0);
}

// Tests para MetalMaterial
TEST(MetalMaterialTest, DefaultValues) {
  MetalMaterial const metal;

  EXPECT_EQ(metal.type, render::METAL_TYPE);
  EXPECT_TRUE(metal.name.empty());
  EXPECT_DOUBLE_EQ(metal.reflectance_r, 0.0);
  EXPECT_DOUBLE_EQ(metal.reflectance_g, 0.0);
  EXPECT_DOUBLE_EQ(metal.reflectance_b, 0.0);
  EXPECT_DOUBLE_EQ(metal.diffusion, 0.0);
}

TEST(MetalMaterialTest, Initialization) {
  MetalMaterial metal;
  metal.name          = "gold_metal";
  metal.reflectance_r = 1.0;
  metal.reflectance_g = 0.8;
  metal.reflectance_b = 0.2;
  metal.diffusion     = 0.3;

  EXPECT_EQ(metal.type, render::METAL_TYPE);
  EXPECT_EQ(metal.name, "gold_metal");
  EXPECT_DOUBLE_EQ(metal.reflectance_r, 1.0);
  EXPECT_DOUBLE_EQ(metal.reflectance_g, 0.8);
  EXPECT_DOUBLE_EQ(metal.reflectance_b, 0.2);
  EXPECT_DOUBLE_EQ(metal.diffusion, 0.3);
}

TEST(MetalMaterialTest, DiffusionRange) {
  MetalMaterial metal;

  // Test diferentes valores de difusión
  metal.diffusion = 0.0;  // Sin difusión
  EXPECT_DOUBLE_EQ(metal.diffusion, 0.0);

  metal.diffusion = 2.5;  // Alta difusión
  EXPECT_DOUBLE_EQ(metal.diffusion, 2.5);

  metal.diffusion = 0.1;  // Baja difusión
  EXPECT_DOUBLE_EQ(metal.diffusion, 0.1);
}

// Tests para RefractiveMaterial
TEST(RefractiveMaterialTest, DefaultValues) {
  RefractiveMaterial const refrac;

  EXPECT_EQ(refrac.type, render::REFRACTIVE_TYPE);
  EXPECT_TRUE(refrac.name.empty());
  EXPECT_DOUBLE_EQ(refrac.refractive_index, 0.0);
}

TEST(RefractiveMaterialTest, Initialization) {
  RefractiveMaterial refrac;
  refrac.name             = "glass";
  refrac.refractive_index = 1.5;

  EXPECT_EQ(refrac.type, render::REFRACTIVE_TYPE);
  EXPECT_EQ(refrac.name, "glass");
  EXPECT_DOUBLE_EQ(refrac.refractive_index, 1.5);
}

TEST(RefractiveMaterialTest, CommonRefractiveIndices) {
  RefractiveMaterial refrac;

  // Test índices de refracción comunes
  refrac.refractive_index = 1.0;  // Aire
  EXPECT_DOUBLE_EQ(refrac.refractive_index, 1.0);

  refrac.refractive_index = 1.33;  // Agua
  EXPECT_DOUBLE_EQ(refrac.refractive_index, 1.33);

  refrac.refractive_index = 1.52;  // Vidrio crown
  EXPECT_DOUBLE_EQ(refrac.refractive_index, 1.52);

  refrac.refractive_index = 2.42;  // Diamante
  EXPECT_DOUBLE_EQ(refrac.refractive_index, 2.42);
}

// Tests de comparación entre materiales
TEST(MaterialsComparisonTest, DifferentTypes) {
  MatteMaterial const matte;
  MetalMaterial const metal;
  RefractiveMaterial const refrac;

  EXPECT_EQ(matte.type, render::MATTE_TYPE);
  EXPECT_EQ(metal.type, render::METAL_TYPE);
  EXPECT_EQ(refrac.type, render::REFRACTIVE_TYPE);

  EXPECT_NE(matte.type, metal.type);
  EXPECT_NE(matte.type, refrac.type);
  EXPECT_NE(metal.type, refrac.type);
}

TEST(MaterialsComparisonTest, SameTypeDifferentNames) {
  MatteMaterial matte1;
  matte1.name          = "matte_red";
  matte1.reflectance_r = 0.8;

  MatteMaterial matte2;
  matte2.name          = "matte_blue";
  matte2.reflectance_b = 0.8;

  EXPECT_EQ(matte1.type, matte2.type);
  EXPECT_NE(matte1.name, matte2.name);
  EXPECT_NE(matte1.reflectance_r, matte2.reflectance_r);
}

// Tests de valores límite/boundary
TEST(MaterialsBoundaryTest, ExtremeValues) {
  // Matte material con valores extremos
  MatteMaterial matte;
  matte.reflectance_r = 0.0;  // Mínimo
  matte.reflectance_g = 0.5;  // Medio
  matte.reflectance_b = 1.0;  // Máximo

  EXPECT_DOUBLE_EQ(matte.reflectance_r, 0.0);
  EXPECT_DOUBLE_EQ(matte.reflectance_g, 0.5);
  EXPECT_DOUBLE_EQ(matte.reflectance_b, 1.0);

  // Metal material con difusión extrema
  MetalMaterial metal;
  metal.diffusion = 0.0;  // Mínimo
  EXPECT_DOUBLE_EQ(metal.diffusion, 0.0);

  metal.diffusion = 10.0;  // Alto (depende del rango permitido)
  EXPECT_DOUBLE_EQ(metal.diffusion, 10.0);

  // Refractive material con índices extremos
  RefractiveMaterial refrac;
  refrac.refractive_index = 0.1;  // Muy bajo (teóricamente imposible)
  EXPECT_DOUBLE_EQ(refrac.refractive_index, 0.1);

  refrac.refractive_index = 5.0;  // Muy alto
  EXPECT_DOUBLE_EQ(refrac.refractive_index, 5.0);
}

// Tests de nombres de materiales
TEST(MaterialsNameTest, ValidNames) {
  MatteMaterial matte;

  // Nombres válidos
  matte.name = "simple";
  EXPECT_EQ(matte.name, "simple");

  matte.name = "material_01";
  EXPECT_EQ(matte.name, "material_01");

  matte.name = "red_matte_512";
  EXPECT_EQ(matte.name, "red_matte_512");

  matte.name = "a";  // Nombre corto
  EXPECT_EQ(matte.name, "a");
}

// Test de herencia (si MaterialBase tiene funcionalidad)
TEST(MaterialsInheritanceTest, BaseClass) {
  MatteMaterial const matte;
  MetalMaterial const metal;
  RefractiveMaterial const refrac;

  // Verificar que todos heredan de MaterialBase
  // Esto se verifica implícitamente por la herencia en la definición

  // Podemos verificar que tienen el tipo correcto
  EXPECT_EQ(matte.type, render::MATTE_TYPE);
  EXPECT_EQ(metal.type, render::METAL_TYPE);
  EXPECT_EQ(refrac.type, render::REFRACTIVE_TYPE);
}
