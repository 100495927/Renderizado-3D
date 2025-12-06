#include "../../common/include/materials.hpp"
#include "../../common/include/objects.hpp"
#include "../../common/include/scene_parser.hpp"
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <string>
#include <vector>

// Fixture para tests de scene parser
class SceneParserTest : public ::testing::Test {
protected:
  void SetUp() override {
    // Crear archivos de test temporales
    test_dir = std::filesystem::temp_directory_path() / "render_scene_test";
    std::filesystem::create_directories(test_dir);
  }

  void TearDown() override {
    // Limpiar archivos de test
    std::filesystem::remove_all(test_dir);
  }

  std::filesystem::path test_dir;

  void createTestFile(std::string const & filename, std::string const & content) {
    std::ofstream file(test_dir / filename);
    file << content;
    file.close();
  }
};

// Tests básicos de parse_scene
TEST_F(SceneParserTest, ParseScene_ValidFile_ParsesCorrectly) {
  std::string const scene_content = R"(
matte: wall 0.8 0.8 0.8
metal: gold 1.0 0.8 0.2 0.1
refractive: glass 1.5
sphere: 0 0 0 1 wall
sphere: 2 0 0 0.5 gold
cylinder: 0 0 2 0.5 0 1 0 glass
)";

  createTestFile("valid_scene.txt", scene_content);

  std::vector<MatteMaterial> matte_materials;
  std::vector<MetalMaterial> metal_materials;
  std::vector<RefractiveMaterial> refractive_materials;
  std::vector<Sphere> spheres;
  std::vector<Cylinder> cylinders;

  SceneOutput out{matte_materials, metal_materials, refractive_materials, spheres, cylinders};
  bool const result = parse_scene((test_dir / "valid_scene.txt").string(), out);

  EXPECT_TRUE(result);
  EXPECT_EQ(matte_materials.size(), 1);
  EXPECT_EQ(metal_materials.size(), 1);
  EXPECT_EQ(refractive_materials.size(), 1);
  EXPECT_EQ(spheres.size(), 2);
  EXPECT_EQ(cylinders.size(), 1);

  // Verificar materiales
  EXPECT_EQ(matte_materials[0].name, "wall");
  EXPECT_DOUBLE_EQ(matte_materials[0].reflectance_r, 0.8);
  EXPECT_DOUBLE_EQ(matte_materials[0].reflectance_g, 0.8);
  EXPECT_DOUBLE_EQ(matte_materials[0].reflectance_b, 0.8);

  EXPECT_EQ(metal_materials[0].name, "gold");
  EXPECT_DOUBLE_EQ(metal_materials[0].reflectance_r, 1.0);
  EXPECT_DOUBLE_EQ(metal_materials[0].reflectance_g, 0.8);
  EXPECT_DOUBLE_EQ(metal_materials[0].reflectance_b, 0.2);
  EXPECT_DOUBLE_EQ(metal_materials[0].diffusion, 0.1);

  EXPECT_EQ(refractive_materials[0].name, "glass");
  EXPECT_DOUBLE_EQ(refractive_materials[0].refractive_index, 1.5);

  // Verificar objetos
  EXPECT_DOUBLE_EQ(spheres[0].center_x, 0.0);
  EXPECT_DOUBLE_EQ(spheres[0].center_y, 0.0);
  EXPECT_DOUBLE_EQ(spheres[0].center_z, 0.0);
  EXPECT_DOUBLE_EQ(spheres[0].radius, 1.0);
  EXPECT_EQ(spheres[0].material_name, "wall");

  EXPECT_DOUBLE_EQ(cylinders[0].center_x, 0.0);
  EXPECT_DOUBLE_EQ(cylinders[0].center_y, 0.0);
  EXPECT_DOUBLE_EQ(cylinders[0].center_z, 2.0);
  EXPECT_DOUBLE_EQ(cylinders[0].radius, 0.5);
  EXPECT_DOUBLE_EQ(cylinders[0].axis_x, 0.0);
  EXPECT_DOUBLE_EQ(cylinders[0].axis_y, 1.0);
  EXPECT_DOUBLE_EQ(cylinders[0].axis_z, 0.0);
  EXPECT_EQ(cylinders[0].material_name, "glass");
}

TEST_F(SceneParserTest, ParseScene_NonExistentFile_ReturnsFalse) {
  std::vector<MatteMaterial> matte_materials;
  std::vector<MetalMaterial> metal_materials;
  std::vector<RefractiveMaterial> refractive_materials;
  std::vector<Sphere> spheres;
  std::vector<Cylinder> cylinders;

  SceneOutput out{matte_materials, metal_materials, refractive_materials, spheres, cylinders};
  bool const result = parse_scene("nonexistent_scene.txt", out);

  EXPECT_FALSE(result);
}

TEST_F(SceneParserTest, ParseScene_EmptyFile_ReturnsTrue) {
  createTestFile("empty_scene.txt", "");

  std::vector<MatteMaterial> matte_materials;
  std::vector<MetalMaterial> metal_materials;
  std::vector<RefractiveMaterial> refractive_materials;
  std::vector<Sphere> spheres;
  std::vector<Cylinder> cylinders;

  SceneOutput out{matte_materials, metal_materials, refractive_materials, spheres, cylinders};
  bool const result = parse_scene((test_dir / "empty_scene.txt").string(), out);

  EXPECT_TRUE(result);  // Archivo vacío es válido
}

// Tests de validación de materiales
TEST_F(SceneParserTest, ParseScene_DuplicateMaterialName_ReturnsFalse) {
  std::string const scene_content = R"(
matte: wall 0.8 0.8 0.8
matte: wall 0.5 0.5 0.5
)";

  createTestFile("duplicate_material.txt", scene_content);

  std::vector<MatteMaterial> matte_materials;
  std::vector<MetalMaterial> metal_materials;
  std::vector<RefractiveMaterial> refractive_materials;
  std::vector<Sphere> spheres;
  std::vector<Cylinder> cylinders;

  SceneOutput out{matte_materials, metal_materials, refractive_materials, spheres, cylinders};
  bool const result = parse_scene((test_dir / "duplicate_material.txt").string(), out);

  EXPECT_FALSE(result);
}

TEST_F(SceneParserTest, ParseScene_ObjectWithUndefinedMaterial_ReturnsFalse) {
  std::string const scene_content = R"(
sphere: 0 0 0 1 undefined_material
)";

  createTestFile("undefined_material.txt", scene_content);

  std::vector<MatteMaterial> matte_materials;
  std::vector<MetalMaterial> metal_materials;
  std::vector<RefractiveMaterial> refractive_materials;
  std::vector<Sphere> spheres;
  std::vector<Cylinder> cylinders;

  SceneOutput out{matte_materials, metal_materials, refractive_materials, spheres, cylinders};
  bool const result = parse_scene((test_dir / "undefined_material.txt").string(), out);

  EXPECT_FALSE(result);
}

// Tests de casos error en parámetros
TEST_F(SceneParserTest, ParseScene_InvalidMatteParameters_ReturnsFalse) {
  std::string const scene_content = "matte: wall 2.0 0.8 0.8\n";  // R fuera de rango

  createTestFile("invalid_matte.txt", scene_content);

  std::vector<MatteMaterial> matte_materials;
  std::vector<MetalMaterial> metal_materials;
  std::vector<RefractiveMaterial> refractive_materials;
  std::vector<Sphere> spheres;
  std::vector<Cylinder> cylinders;

  SceneOutput out{matte_materials, metal_materials, refractive_materials, spheres, cylinders};
  bool const result = parse_scene((test_dir / "invalid_matte.txt").string(), out);

  EXPECT_FALSE(result);
}

TEST_F(SceneParserTest, ParseScene_InvalidMetalParameters_ReturnsFalse) {
  std::string const scene_content = "metal: gold 1.0 0.8 0.2 -0.1\n";  // Diffusion negativo

  createTestFile("invalid_metal.txt", scene_content);

  std::vector<MatteMaterial> matte_materials;
  std::vector<MetalMaterial> metal_materials;
  std::vector<RefractiveMaterial> refractive_materials;
  std::vector<Sphere> spheres;
  std::vector<Cylinder> cylinders;

  SceneOutput out{matte_materials, metal_materials, refractive_materials, spheres, cylinders};
  bool const result = parse_scene((test_dir / "invalid_metal.txt").string(), out);

  EXPECT_FALSE(result);
}

TEST_F(SceneParserTest, ParseScene_InvalidRefractiveParameters_ReturnsFalse) {
  std::string const scene_content = "refractive: glass 0.0\n";  // Índice inválido

  createTestFile("invalid_refractive.txt", scene_content);

  std::vector<MatteMaterial> matte_materials;
  std::vector<MetalMaterial> metal_materials;
  std::vector<RefractiveMaterial> refractive_materials;
  std::vector<Sphere> spheres;
  std::vector<Cylinder> cylinders;

  SceneOutput out{matte_materials, metal_materials, refractive_materials, spheres, cylinders};
  bool const result = parse_scene((test_dir / "invalid_refractive.txt").string(), out);

  EXPECT_FALSE(result);
}

TEST_F(SceneParserTest, ParseScene_InvalidSphereParameters_ReturnsFalse) {
  std::string const scene_content = "sphere: 0 0 0 -1 wall\n";  // Radio negativo

  createTestFile("invalid_sphere.txt", scene_content);

  std::vector<MatteMaterial> matte_materials;
  std::vector<MetalMaterial> metal_materials;
  std::vector<RefractiveMaterial> refractive_materials;
  std::vector<Sphere> spheres;
  std::vector<Cylinder> cylinders;

  SceneOutput out{matte_materials, metal_materials, refractive_materials, spheres, cylinders};
  bool const result = parse_scene((test_dir / "invalid_sphere.txt").string(), out);

  EXPECT_FALSE(result);
}

TEST_F(SceneParserTest, ParseScene_InvalidCylinderParameters_ReturnsFalse) {
  std::string const scene_content = "cylinder: 0 0 0 0 0 1 0 wall\n";  // Radio cero

  createTestFile("invalid_cylinder.txt", scene_content);

  std::vector<MatteMaterial> matte_materials;
  std::vector<MetalMaterial> metal_materials;
  std::vector<RefractiveMaterial> refractive_materials;
  std::vector<Sphere> spheres;
  std::vector<Cylinder> cylinders;

  SceneOutput out{matte_materials, metal_materials, refractive_materials, spheres, cylinders};
  bool const result = parse_scene((test_dir / "invalid_cylinder.txt").string(), out);

  EXPECT_FALSE(result);
}

// Test de orden correcto (materiales antes que objetos)
TEST_F(SceneParserTest, ParseScene_CorrectOrder_MaterialsBeforeObjects) {
  std::string const scene_content = R"(
matte: wall 0.8 0.8 0.8
sphere: 0 0 0 1 wall
metal: gold 1.0 0.8 0.2 0.1
sphere: 2 0 0 0.5 gold
)";

  createTestFile("correct_order.txt", scene_content);

  std::vector<MatteMaterial> matte_materials;
  std::vector<MetalMaterial> metal_materials;
  std::vector<RefractiveMaterial> refractive_materials;
  std::vector<Sphere> spheres;
  std::vector<Cylinder> cylinders;

  SceneOutput out{matte_materials, metal_materials, refractive_materials, spheres, cylinders};
  bool const result = parse_scene((test_dir / "correct_order.txt").string(), out);

  EXPECT_TRUE(result);
  EXPECT_EQ(matte_materials.size(), 1);
  EXPECT_EQ(metal_materials.size(), 1);
  EXPECT_EQ(spheres.size(), 2);
}

// Test de unknown entity
TEST_F(SceneParserTest, ParseScene_UnknownEntity_ReturnsFalse) {
  std::string const scene_content = "unknown: param1 param2\n";

  createTestFile("unknown_entity.txt", scene_content);

  std::vector<MatteMaterial> matte_materials;
  std::vector<MetalMaterial> metal_materials;
  std::vector<RefractiveMaterial> refractive_materials;
  std::vector<Sphere> spheres;
  std::vector<Cylinder> cylinders;

  SceneOutput out{matte_materials, metal_materials, refractive_materials, spheres, cylinders};
  bool const result = parse_scene((test_dir / "unknown_entity.txt").string(), out);

  EXPECT_FALSE(result);
}

// Test de integración completa
TEST_F(SceneParserTest, Integration_ComplexScene_ParsesCorrectly) {
  std::string const scene_content = R"(
# Materials
matte: floor 0.9 0.9 0.9
matte: wall1 0.8 0.7 0.6
metal: sphere_metal 0.9 0.9 0.9 0.05
refractive: glass_ball 1.33

# Objects
sphere: 0 1 0 1 sphere_metal
sphere: 2 1 1 0.5 glass_ball
cylinder: -2 0 0 0.3 0 1 0 wall1
sphere: -1 0.5 2 0.7 sphere_metal
)";

  createTestFile("complex_scene.txt", scene_content);

  std::vector<MatteMaterial> matte_materials;
  std::vector<MetalMaterial> metal_materials;
  std::vector<RefractiveMaterial> refractive_materials;
  std::vector<Sphere> spheres;
  std::vector<Cylinder> cylinders;

  SceneOutput out{matte_materials, metal_materials, refractive_materials, spheres, cylinders};
  bool const result = parse_scene((test_dir / "complex_scene.txt").string(), out);

  EXPECT_TRUE(result);
  EXPECT_EQ(matte_materials.size(), 2);
  EXPECT_EQ(metal_materials.size(), 1);
  EXPECT_EQ(refractive_materials.size(), 1);
  EXPECT_EQ(spheres.size(), 3);
  EXPECT_EQ(cylinders.size(), 1);
}
