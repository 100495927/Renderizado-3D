// utcommon/test_material_base.cpp
#include "../../common/include/material_base.hpp"
#include <array>
#include <gtest/gtest.h>
#include <memory>
#include <utility>

TEST(MaterialBaseTest, DefaultConstructor) {
  render::MaterialBase const base;
  EXPECT_EQ(base.type, render::MATTE_TYPE);
}

TEST(MaterialBaseTest, ParameterizedConstructor) {
  // Test constructores con parámetros usando objetos temporales
  EXPECT_EQ(render::MaterialBase(render::METAL_TYPE).type, render::METAL_TYPE);
  EXPECT_EQ(render::MaterialBase(render::REFRACTIVE_TYPE).type, render::REFRACTIVE_TYPE);
}

TEST(MaterialBaseTest, CopyConstructor) {
  render::MaterialBase const original(render::METAL_TYPE);

  // Probar copy constructor sin crear variable local innecesaria
  EXPECT_EQ(render::MaterialBase(original).type, render::METAL_TYPE);
}

TEST(MaterialBaseTest, CopyAssignment) {
  render::MaterialBase const source(render::REFRACTIVE_TYPE);
  render::MaterialBase target;

  // Probar assignment y verificar inmediatamente
  target = source;
  EXPECT_EQ(target.type, render::REFRACTIVE_TYPE);
}

TEST(MaterialBaseTest, TypeComparison) {
  // Comparar tipos directamente sin variables intermedias
  EXPECT_NE(render::MaterialBase(render::MATTE_TYPE).type,
            render::MaterialBase(render::METAL_TYPE).type);
  EXPECT_NE(render::MaterialBase(render::MATTE_TYPE).type,
            render::MaterialBase(render::REFRACTIVE_TYPE).type);
}

TEST(MaterialBaseTest, VirtualDestructor) {
  struct TestMaterial : public render::MaterialBase {
    explicit TestMaterial(render::MaterialType t) : render::MaterialBase(t) { }

    // Regla de cinco
    TestMaterial(TestMaterial const &)             = default;
    TestMaterial(TestMaterial &&)                  = default;
    TestMaterial & operator=(TestMaterial const &) = default;
    TestMaterial & operator=(TestMaterial &&)      = default;
    ~TestMaterial() override                       = default;
  };

  // Verificar polimorfismo sin copias
  auto const & derived                  = std::make_unique<TestMaterial>(render::METAL_TYPE);
  render::MaterialBase const * base_ptr = derived.get();
  EXPECT_EQ(base_ptr->type, render::METAL_TYPE);
}

TEST(MaterialBaseTest, MoveConstructor) {
  render::MaterialBase original(render::REFRACTIVE_TYPE);

  // Probar move constructor sin variable local innecesaria
  EXPECT_EQ(render::MaterialBase(std::move(original)).type, render::REFRACTIVE_TYPE);
}

TEST(MaterialBaseTest, MoveAssignment) {
  // Usar objeto temporal para move assignment
  render::MaterialBase target;
  target = render::MaterialBase(render::METAL_TYPE);
  EXPECT_EQ(target.type, render::METAL_TYPE);
}

TEST(MaterialBaseTest, TypeIntegrity) {
  // Usar std::array con acceso directo por índice
  std::array<render::MaterialBase, 3> const materials = {
    render::MaterialBase(render::MATTE_TYPE), render::MaterialBase(render::METAL_TYPE),
    render::MaterialBase(render::REFRACTIVE_TYPE)};

  EXPECT_EQ(materials[0].type, render::MATTE_TYPE);
  EXPECT_EQ(materials[1].type, render::METAL_TYPE);
  EXPECT_EQ(materials[2].type, render::REFRACTIVE_TYPE);
}

TEST(MaterialBaseTest, MaterialTypeEnum) {
  // Test valores del enum directamente
  EXPECT_EQ(static_cast<int>(render::MATTE_TYPE), 0);
  EXPECT_EQ(static_cast<int>(render::METAL_TYPE), 1);
  EXPECT_EQ(static_cast<int>(render::REFRACTIVE_TYPE), 2);

  // Verificar unicidad
  EXPECT_NE(render::MATTE_TYPE, render::METAL_TYPE);
  EXPECT_NE(render::MATTE_TYPE, render::REFRACTIVE_TYPE);
  EXPECT_NE(render::METAL_TYPE, render::REFRACTIVE_TYPE);
}

TEST(MaterialBaseTest, DefaultValues) {
  // Verificar valor por defecto sin copias
  EXPECT_EQ(render::MaterialBase().type, render::MATTE_TYPE);
  EXPECT_EQ(render::MaterialBase(render::MATTE_TYPE).type, render::MaterialBase().type);
}

TEST(MaterialBaseTest, ConstObjects) {
  // Probar comportamiento con objetos const
  render::MaterialBase const const_matte(render::MATTE_TYPE);
  render::MaterialBase const const_metal(render::METAL_TYPE);

  EXPECT_EQ(const_matte.type, render::MATTE_TYPE);
  EXPECT_EQ(const_metal.type, render::METAL_TYPE);
}
