// utaos/test_main.cpp
#include "../../aos/include/aos_camera.hpp"
#include "../../aos/include/aos_image.hpp"
#include "../../aos/include/render_aos.hpp"
#include "../../common/include/config.hpp"
#include "../../common/include/config_parser.hpp"
#include "../../common/include/materials.hpp"
#include "../../common/include/objects.hpp"
#include "../../common/include/scene_parser.hpp"

#include <filesystem>
#include <fstream>
#include <functional>
#include <gtest/gtest.h>
#include <system_error>
#include <vector>

namespace aos::test {

  class AOSMainTest : public ::testing::Test {
  protected:
    void SetUp() override { setupTestFiles(); }

    void TearDown() override { cleanupTestFiles(); }

    static void setupTestFiles() {
      // Crear directorio de prueba si no existe
      std::filesystem::create_directories("test_data");
      // Crear archivo de configuración de prueba
      std::ofstream config_file("test_data/test_config.cfg");
      config_file << "image_width: 100\n";
      config_file << "aspect_ratio: 16 9\n";
      config_file << "gamma: 2.2\n";
      config_file << "camera_position: 0 0 -5\n";
      config_file << "camera_target: 0 0 0\n";
      config_file << "camera_north: 0 1 0\n";
      config_file << "field_of_view: 90\n";
      config_file << "samples_per_pixel: 2\n";
      config_file << "max_depth: 3\n";
      config_file << "material_rng_seed: 42\n";
      config_file << "ray_rng_seed: 43\n";
      config_file << "background_dark_color: 0.25 0.5 1\n";
      config_file << "background_light_color: 1 1 1\n";
      config_file.close();

      // Crear archivo de escena de prueba
      std::ofstream scene_file("test_data/test_scene.txt");
      scene_file << "matte: red_material 0.8 0.2 0.2\n";
      scene_file << "metal: silver_material 0.8 0.8 0.8 0.1\n";
      scene_file << "refractive: glass_material 1.5\n";
      scene_file << "sphere: 0 0 0 1.0 red_material\n";
      scene_file << "cylinder: 2 0 0 0.5 0 2 0 silver_material\n";
      scene_file.close();

      // Crear archivo de configuración inválido
      std::ofstream invalid_config_file("test_data/invalid_config.cfg");
      invalid_config_file << "invalid_key: some_value\n";
      invalid_config_file << "image_width: not_a_number\n";
      invalid_config_file.close();

      // Crear archivo de escena inválido
      std::ofstream invalid_scene_file("test_data/invalid_scene.txt");
      invalid_scene_file << "invalid_type: invalid_data\n";
      invalid_scene_file << "sphere: 0 0 0 not_a_number invalid_material\n";
      invalid_scene_file.close();
    }

    static void cleanupTestFiles() {
      // Limpiar archivos de prueba
      std::filesystem::remove("test_data/test_config.cfg");
      std::filesystem::remove("test_data/test_scene.txt");
      std::filesystem::remove("test_data/invalid_config.cfg");
      std::filesystem::remove("test_data/invalid_scene.txt");
      std::filesystem::remove("test_data/test_output.ppm");
      std::filesystem::remove("test_output_aos.ppm");
      std::filesystem::remove("output_aos.ppm");

      // Intentar limpiar directorio (puede fallar si no está vacío)
      std::error_code ec;
      std::filesystem::remove("test_data", ec);
    }
  };

  // Tests de configuración básica
  TEST_F(AOSMainTest, ConfigParsing_ValidFile_Succeeds) {
    ConfigParams cfg;
    bool const result = parse_config("test_data/test_config.cfg", cfg);

    EXPECT_TRUE(result);
    EXPECT_EQ(cfg.image_width, 100);
    EXPECT_EQ(cfg.samples_per_pixel, 2);
    EXPECT_EQ(cfg.max_depth, 3);
  }

  TEST_F(AOSMainTest, ConfigParsing_InvalidFile_Fails) {
    ConfigParams cfg;
    bool const result = parse_config("test_data/invalid_config.cfg", cfg);

    EXPECT_FALSE(result);
  }

  TEST_F(AOSMainTest, ConfigParsing_NonExistentFile_Fails) {
    ConfigParams cfg;
    bool const result = parse_config("non_existent.cfg", cfg);

    EXPECT_FALSE(result);
  }

  // Tests de parsing de escena
  TEST_F(AOSMainTest, SceneParsing_ValidFile_Succeeds) {
    std::vector<MatteMaterial> matte_materials;
    std::vector<MetalMaterial> metal_materials;
    std::vector<RefractiveMaterial> refractive_materials;
    std::vector<Sphere> spheres;
    std::vector<Cylinder> cylinders;

    SceneOutput scene_out{std::ref(matte_materials), std::ref(metal_materials),
                          std::ref(refractive_materials), std::ref(spheres), std::ref(cylinders)};

    bool const result = parse_scene("test_data/test_scene.txt", scene_out);

    EXPECT_TRUE(result);
    EXPECT_FALSE(scene_out.matte_materials.get().empty());
    EXPECT_FALSE(scene_out.spheres.get().empty());
    EXPECT_FALSE(scene_out.cylinders.get().empty());
  }

  TEST_F(AOSMainTest, SceneParsing_InvalidFile_Fails) {
    std::vector<MatteMaterial> matte_materials;
    std::vector<MetalMaterial> metal_materials;
    std::vector<RefractiveMaterial> refractive_materials;
    std::vector<Sphere> spheres;
    std::vector<Cylinder> cylinders;

    SceneOutput scene_out{std::ref(matte_materials), std::ref(metal_materials),
                          std::ref(refractive_materials), std::ref(spheres), std::ref(cylinders)};

    bool const result = parse_scene("test_data/invalid_scene.txt", scene_out);

    EXPECT_FALSE(result);
  }

  TEST_F(AOSMainTest, SceneParsing_NonExistentFile_Fails) {
    std::vector<MatteMaterial> matte_materials;
    std::vector<MetalMaterial> metal_materials;
    std::vector<RefractiveMaterial> refractive_materials;
    std::vector<Sphere> spheres;
    std::vector<Cylinder> cylinders;

    SceneOutput scene_out{std::ref(matte_materials), std::ref(metal_materials),
                          std::ref(refractive_materials), std::ref(spheres), std::ref(cylinders)};

    bool const result = parse_scene("non_existent.txt", scene_out);

    EXPECT_FALSE(result);
  }

  // Tests de integración de componentes AOS
  TEST_F(AOSMainTest, AOSIntegration_CompletePipeline_Succeeds) {
    // Cargar configuración
    ConfigParams cfg;
    bool const config_loaded = parse_config("test_data/test_config.cfg", cfg);
    ASSERT_TRUE(config_loaded);

    // Cargar escena
    std::vector<MatteMaterial> matte_materials;
    std::vector<MetalMaterial> metal_materials;
    std::vector<RefractiveMaterial> refractive_materials;
    std::vector<Sphere> spheres;
    std::vector<Cylinder> cylinders;

    SceneOutput scene_out{std::ref(matte_materials), std::ref(metal_materials),
                          std::ref(refractive_materials), std::ref(spheres), std::ref(cylinders)};

    bool const scene_loaded = parse_scene("test_data/test_scene.txt", scene_out);
    ASSERT_TRUE(scene_loaded);

    // Crear componentes AOS
    aos::CameraAOS camera(cfg);
    aos::AOSImage image(cfg.image_width, cfg.get_image_height());

    // Ejecutar renderizado
    EXPECT_NO_THROW(aos::render_scene(cfg, scene_out, camera, image));

    // Guardar imagen
    EXPECT_NO_THROW(image.write_ppm("test_data/integration_output.ppm"));

    // Verificar que se creó el archivo
    EXPECT_TRUE(std::filesystem::exists("test_data/integration_output.ppm"));

    // Limpiar
    std::filesystem::remove("test_data/integration_output.ppm");
  }

  TEST_F(AOSMainTest, AOSIntegration_MinimalScene_Succeeds) {
    // Configuración mínima
    ConfigParams cfg;
    cfg.image_width       = 50;
    cfg.aspect_width      = 16;
    cfg.aspect_height     = 9;
    cfg.samples_per_pixel = 1;
    cfg.max_depth         = 2;
    cfg.gamma             = 2.2;

    // Escena mínima programática
    std::vector<MatteMaterial> matte_materials;
    MatteMaterial matte;
    matte.name          = "simple";
    matte.reflectance_r = 0.5;
    matte.reflectance_g = 0.5;
    matte.reflectance_b = 0.5;
    matte_materials.push_back(matte);

    std::vector<MetalMaterial> metal_materials;
    std::vector<RefractiveMaterial> refractive_materials;

    std::vector<Sphere> spheres;
    Sphere sphere;
    sphere.center_x      = 0.0;
    sphere.center_y      = 0.0;
    sphere.center_z      = 0.0;
    sphere.radius        = 1.0;
    sphere.material_name = "simple";
    spheres.push_back(sphere);

    std::vector<Cylinder> cylinders;

    SceneOutput const scene_out{std::ref(matte_materials), std::ref(metal_materials),
                                std::ref(refractive_materials), std::ref(spheres),
                                std::ref(cylinders)};

    // Ejecutar pipeline AOS
    aos::CameraAOS camera(cfg);
    aos::AOSImage image(cfg.image_width, cfg.get_image_height());

    EXPECT_NO_THROW(aos::render_scene(cfg, scene_out, camera, image));
    EXPECT_NO_THROW(image.write_ppm("test_data/minimal_output.ppm"));

    EXPECT_TRUE(std::filesystem::exists("test_data/minimal_output.ppm"));
    std::filesystem::remove("test_data/minimal_output.ppm");
  }

  // Tests de robustez
  TEST_F(AOSMainTest, Robustness_EmptyScene_RendersSuccessfully) {
    ConfigParams cfg;
    cfg.image_width       = 10;
    cfg.aspect_width      = 16;
    cfg.aspect_height     = 9;
    cfg.samples_per_pixel = 1;
    cfg.max_depth         = 1;
    cfg.gamma             = 2.2;

    // Escena vacía
    std::vector<MatteMaterial> matte_materials;
    std::vector<MetalMaterial> metal_materials;
    std::vector<RefractiveMaterial> refractive_materials;
    std::vector<Sphere> spheres;
    std::vector<Cylinder> cylinders;

    SceneOutput const scene_out{std::ref(matte_materials), std::ref(metal_materials),
                                std::ref(refractive_materials), std::ref(spheres),
                                std::ref(cylinders)};

    aos::CameraAOS camera(cfg);
    aos::AOSImage image(cfg.image_width, cfg.get_image_height());

    // No debería crashar con escena vacía
    EXPECT_NO_THROW(aos::render_scene(cfg, scene_out, camera, image));
  }

  TEST_F(AOSMainTest, Robustness_MissingMaterial_HandlesGracefully) {
    ConfigParams cfg;
    cfg.image_width       = 10;
    cfg.aspect_width      = 16;
    cfg.aspect_height     = 9;
    cfg.samples_per_pixel = 1;
    cfg.max_depth         = 1;
    cfg.gamma             = 2.2;

    // Escena con material faltante
    std::vector<MatteMaterial> matte_materials;
    std::vector<MetalMaterial> metal_materials;
    std::vector<RefractiveMaterial> refractive_materials;

    std::vector<Sphere> spheres;
    Sphere sphere;
    sphere.center_x      = 0.0;
    sphere.center_y      = 0.0;
    sphere.center_z      = 0.0;
    sphere.radius        = 1.0;
    sphere.material_name = "non_existent_material";  // Material que no existe
    spheres.push_back(sphere);

    std::vector<Cylinder> cylinders;

    SceneOutput const scene_out{std::ref(matte_materials), std::ref(metal_materials),
                                std::ref(refractive_materials), std::ref(spheres),
                                std::ref(cylinders)};

    aos::CameraAOS camera(cfg);
    aos::AOSImage image(cfg.image_width, cfg.get_image_height());

    // No debería crashar con material faltante
    EXPECT_NO_THROW(aos::render_scene(cfg, scene_out, camera, image));
  }

}  // namespace aos::test
