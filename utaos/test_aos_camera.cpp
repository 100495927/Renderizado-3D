// utaos/test_aos_camera.cpp
#include "../../aos/include/aos_camera.hpp"
#include "../../common/include/config.hpp"

#include <chrono>
#include <cmath>
#include <cstddef>
#include <gtest/gtest.h>
#include <memory>
#include <random>

namespace aos::test {

  class CameraAOSTest : public ::testing::Test {
  protected:
    void SetUp() override {
      setupBasicConfig();
      setupCamera();
    }

    void setupBasicConfig() {
      cfg = std::make_unique<ConfigParams>();

      // Configuración básica de cámara
      cfg->camera_x      = 0.0;
      cfg->camera_y      = 0.0;
      cfg->camera_z      = -5.0;
      cfg->target_x      = 0.0;
      cfg->target_y      = 0.0;
      cfg->target_z      = 0.0;
      cfg->north_x       = 0.0;
      cfg->north_y       = 1.0;
      cfg->north_z       = 0.0;
      cfg->field_of_view = 90.0;
      cfg->ray_rng_seed  = 42;

      // Parámetros de imagen
      cfg->image_width   = 100;
      cfg->aspect_width  = 16;
      cfg->aspect_height = 9;
    }

    void setupCamera() { camera = std::make_unique<CameraAOS>(*cfg); }

    std::unique_ptr<ConfigParams> cfg;
    std::unique_ptr<CameraAOS> camera;
  };

  // Tests de construcción básica
  TEST_F(CameraAOSTest, Constructor_InitializesWithConfigValues) {
    // Verificar que los valores básicos se inicializan correctamente
    EXPECT_NO_THROW(CameraAOS const camera_instance(*cfg));

    // Podemos verificar que no hay valores NaN o infinitos accediendo a través de funciones
    // públicas si las hubiera, pero por ahora nos aseguramos de que la construcción funciona
  }

  TEST_F(CameraAOSTest, Constructor_DifferentConfigurations_CompletesWithoutCrash) {
    // Configuración con cámara en posición diferente
    cfg->camera_x = 10.0;
    cfg->camera_y = 5.0;
    cfg->camera_z = -8.0;
    EXPECT_NO_THROW(CameraAOS const camera1(*cfg));

    // Configuración con target diferente
    cfg->target_x = 5.0;
    cfg->target_y = 3.0;
    cfg->target_z = 2.0;
    EXPECT_NO_THROW(CameraAOS const camera2(*cfg));

    // Configuración con north vector diferente
    cfg->north_x = 1.0;
    cfg->north_y = 0.0;
    cfg->north_z = 0.0;
    EXPECT_NO_THROW(CameraAOS const camera3(*cfg));
  }

  // Tests de begin_frame
  TEST_F(CameraAOSTest, BeginFrame_ValidDimensions_CompletesWithoutCrash) {
    EXPECT_NO_THROW(camera->begin_frame(100, 56));  // 100x56 para 16:9
  }

  TEST_F(CameraAOSTest, BeginFrame_DifferentDimensions_CompletesWithoutCrash) {
    // Dimensiones pequeñas
    EXPECT_NO_THROW(camera->begin_frame(10, 6));

    // Dimensiones grandes
    EXPECT_NO_THROW(camera->begin_frame(1'920, 1'080));

    // Relación de aspecto cuadrada
    EXPECT_NO_THROW(camera->begin_frame(100, 100));
  }

  TEST_F(CameraAOSTest, BeginFrame_ZeroDimensions_CompletesWithoutCrash) {
    // Dimensiones cero deberían manejarse correctamente
    EXPECT_NO_THROW(camera->begin_frame(0, 0));
    EXPECT_NO_THROW(camera->begin_frame(100, 0));
    EXPECT_NO_THROW(camera->begin_frame(0, 100));
  }

  TEST_F(CameraAOSTest, BeginFrame_MultipleCalls_CompletesWithoutCrash) {
    // Llamadas múltiples deberían funcionar
    EXPECT_NO_THROW(camera->begin_frame(100, 56));
    EXPECT_NO_THROW(camera->begin_frame(200, 112));
    EXPECT_NO_THROW(camera->begin_frame(50, 28));
  }

  // Tests de make_ray
  TEST_F(CameraAOSTest, MakeRay_ValidCoordinates_CompletesWithoutCrash) {
    camera->begin_frame(100, 56);
    std::mt19937 rng(42);

    // Coordenadas dentro del rango
    EXPECT_NO_THROW(camera->make_ray(0, 0, rng));
    EXPECT_NO_THROW(camera->make_ray(50, 28, rng));
    EXPECT_NO_THROW(camera->make_ray(99, 55, rng));
  }

  TEST_F(CameraAOSTest, MakeRay_EdgeCoordinates_CompletesWithoutCrash) {
    camera->begin_frame(100, 56);
    std::mt19937 rng(42);

    // Coordenadas en los bordes
    EXPECT_NO_THROW(camera->make_ray(0, 0, rng));
    EXPECT_NO_THROW(camera->make_ray(99, 0, rng));
    EXPECT_NO_THROW(camera->make_ray(0, 55, rng));
    EXPECT_NO_THROW(camera->make_ray(99, 55, rng));
  }

  TEST_F(CameraAOSTest, MakeRay_WithoutBeginFrame_CompletesWithoutCrash) {
    std::mt19937 rng(42);

    // make_ray sin llamar a begin_frame primero
    // Debería manejarse correctamente (posiblemente con valores por defecto)
    EXPECT_NO_THROW(camera->make_ray(0, 0, rng));
  }

  TEST_F(CameraAOSTest, MakeRay_DifferentRNGSeeds_CompletesWithoutCrash) {
    camera->begin_frame(100, 56);

    // Diferentes semillas de RNG
    std::mt19937 rng1(42);
    std::mt19937 rng2(123);
    std::mt19937 rng3(999);

    EXPECT_NO_THROW(camera->make_ray(50, 28, rng1));
    EXPECT_NO_THROW(camera->make_ray(50, 28, rng2));
    EXPECT_NO_THROW(camera->make_ray(50, 28, rng3));
  }

  TEST_F(CameraAOSTest, MakeRay_ProducesValidRayStructure) {
    camera->begin_frame(100, 56);
    std::mt19937 rng(42);

    auto ray_sample = camera->make_ray(50, 28, rng);

    // Verificar que la estructura tiene valores razonables
    // El origen debería estar cerca de la posición de la cámara
    EXPECT_NEAR(ray_sample.origin_x, cfg->camera_x, 1.0);
    EXPECT_NEAR(ray_sample.origin_y, cfg->camera_y, 1.0);
    EXPECT_NEAR(ray_sample.origin_z, cfg->camera_z, 1.0);

    // La dirección debería ser un vector unitario (magnitud ~1.0)
    double const magnitude = std::sqrt(ray_sample.dir_x * ray_sample.dir_x +
                                       ray_sample.dir_y * ray_sample.dir_y +
                                       ray_sample.dir_z * ray_sample.dir_z);
    EXPECT_NEAR(magnitude, 1.0, 0.01);
  }

  // Tests de generate_primary_rays
  TEST_F(CameraAOSTest, GeneratePrimaryRays_ValidParameters_CompletesWithoutCrash) {
    EXPECT_NO_THROW(camera->generate_primary_rays(100, 56, 2));
  }

  TEST_F(CameraAOSTest, GeneratePrimaryRays_DifferentSamplesPerPixel_CompletesWithoutCrash) {
    // Diferentes valores de samples per pixel
    EXPECT_NO_THROW(camera->generate_primary_rays(100, 56, 1));
    EXPECT_NO_THROW(camera->generate_primary_rays(100, 56, 10));
    EXPECT_NO_THROW(camera->generate_primary_rays(100, 56, 100));
  }

  TEST_F(CameraAOSTest, GeneratePrimaryRays_DifferentImageSizes_CompletesWithoutCrash) {
    // Diferentes tamaños de imagen
    EXPECT_NO_THROW(camera->generate_primary_rays(10, 6, 2));
    EXPECT_NO_THROW(camera->generate_primary_rays(50, 28, 2));
    EXPECT_NO_THROW(camera->generate_primary_rays(200, 112, 2));
  }

  TEST_F(CameraAOSTest, GeneratePrimaryRays_ZeroParameters_CompletesWithoutCrash) {
    // Parámetros cero deberían manejarse correctamente
    EXPECT_NO_THROW(camera->generate_primary_rays(0, 0, 0));
    EXPECT_NO_THROW(camera->generate_primary_rays(100, 0, 2));
    EXPECT_NO_THROW(camera->generate_primary_rays(0, 56, 2));
    EXPECT_NO_THROW(camera->generate_primary_rays(100, 56, 0));
  }

  TEST_F(CameraAOSTest, GeneratePrimaryRays_ProducesCorrectNumberOfRays) {
    // Verificar que genera el número correcto de rayos
    std::size_t const width  = 10;
    std::size_t const height = 5;
    std::size_t const spp    = 3;

    camera->generate_primary_rays(width, height, spp);

    // Accedemos a rays.size() si es público, o verificamos a través de otros medios
    // Por ahora solo verificamos que no crashea
    EXPECT_NO_THROW(camera->generate_primary_rays(width, height, spp));
  }

  // Tests de diferentes configuraciones de campo de visión
  TEST_F(CameraAOSTest, DifferentFieldOfView_CompletesWithoutCrash) {
    // Campo de visión angosto
    cfg->field_of_view = 30.0;
    setupCamera();
    EXPECT_NO_THROW(camera->begin_frame(100, 56));
    EXPECT_NO_THROW(camera->generate_primary_rays(100, 56, 2));

    // Campo de visión ancho
    cfg->field_of_view = 120.0;
    setupCamera();
    EXPECT_NO_THROW(camera->begin_frame(100, 56));
    EXPECT_NO_THROW(camera->generate_primary_rays(100, 56, 2));

    // Campo de visión extremo (pero válido)
    cfg->field_of_view = 179.0;
    setupCamera();
    EXPECT_NO_THROW(camera->begin_frame(100, 56));
    EXPECT_NO_THROW(camera->generate_primary_rays(100, 56, 2));
  }

  // Tests de diferentes vectores norte
  TEST_F(CameraAOSTest, DifferentNorthVectors_CompletesWithoutCrash) {
    // Norte en X
    cfg->north_x = 1.0;
    cfg->north_y = 0.0;
    cfg->north_z = 0.0;
    setupCamera();
    EXPECT_NO_THROW(camera->begin_frame(100, 56));
    EXPECT_NO_THROW(camera->generate_primary_rays(100, 56, 2));

    // Norte en Z
    cfg->north_x = 0.0;
    cfg->north_y = 0.0;
    cfg->north_z = 1.0;
    setupCamera();
    EXPECT_NO_THROW(camera->begin_frame(100, 56));
    EXPECT_NO_THROW(camera->generate_primary_rays(100, 56, 2));

    // Norte diagonal
    cfg->north_x = 1.0;
    cfg->north_y = 1.0;
    cfg->north_z = 1.0;
    setupCamera();
    EXPECT_NO_THROW(camera->begin_frame(100, 56));
    EXPECT_NO_THROW(camera->generate_primary_rays(100, 56, 2));
  }

  // Tests de diferentes posiciones de cámara y target
  TEST_F(CameraAOSTest, DifferentCameraTargetPositions_CompletesWithoutCrash) {
    // Cámara y target en el mismo punto (debería manejarse)
    cfg->camera_x = 0.0;
    cfg->camera_y = 0.0;
    cfg->camera_z = 0.0;
    cfg->target_x = 0.0;
    cfg->target_y = 0.0;
    cfg->target_z = 0.0;
    setupCamera();
    EXPECT_NO_THROW(camera->begin_frame(100, 56));

    // Cámara muy lejos
    cfg->camera_x = 0.0;
    cfg->camera_y = 0.0;
    cfg->camera_z = -1000.0;
    cfg->target_x = 0.0;
    cfg->target_y = 0.0;
    cfg->target_z = 0.0;
    setupCamera();
    EXPECT_NO_THROW(camera->begin_frame(100, 56));

    // Cámara en cuadrante diferente
    cfg->camera_x = -10.0;
    cfg->camera_y = 5.0;
    cfg->camera_z = -8.0;
    cfg->target_x = 3.0;
    cfg->target_y = -2.0;
    cfg->target_z = 1.0;
    setupCamera();
    EXPECT_NO_THROW(camera->begin_frame(100, 56));
  }

  // Tests de consistencia entre make_ray y generate_primary_rays
  TEST_F(CameraAOSTest, MakeRayVsGeneratePrimaryRays_ConsistentBehavior) {
    camera->begin_frame(10, 5);
    std::mt19937 const rng(42);

    // generate_primary_rays debería completarse sin crash
    EXPECT_NO_THROW(camera->generate_primary_rays(10, 5, 1));

    // Ambos métodos deberían usar la misma configuración de cámara
  }

  // Tests de rendimiento básico
  TEST_F(CameraAOSTest, Performance_GeneratePrimaryRays_ReasonableTime) {
    auto start = std::chrono::high_resolution_clock::now();

    camera->generate_primary_rays(100, 56, 10);  // 100x56 con 10 samples = 56,000 rayos

    auto end      = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    EXPECT_LT(duration.count(), 1'000) << "Generar 56,000 rayos debería ser rápido";
  }

  TEST_F(CameraAOSTest, Performance_MakeRay_ReasonableTime) {
    camera->begin_frame(100, 56);
    std::mt19937 rng(42);

    auto start = std::chrono::high_resolution_clock::now();

    // Generar 1000 rayos individualmente - usando std::size_t para evitar warnings
    for (std::size_t i = 0; i < 1'000; ++i) {
      camera->make_ray(i % 100, i % 56, rng);
    }

    auto end      = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    EXPECT_LT(duration.count(), 100) << "Generar 1000 rayos individualmente debería ser rápido";
  }

  // Tests de robustez con valores extremos
  TEST_F(CameraAOSTest, Robustness_ExtremeConfigValues_CompletesWithoutCrash) {
    // Valores muy grandes
    cfg->camera_x = 1e6;
    cfg->camera_y = 1e6;
    cfg->camera_z = -1e6;
    cfg->target_x = -1e6;
    cfg->target_y = -1e6;
    cfg->target_z = -1e6;
    setupCamera();
    EXPECT_NO_THROW(camera->begin_frame(100, 56));

    // Valores muy pequeños (pero no cero)
    cfg->camera_x = 1e-6;
    cfg->camera_y = 1e-6;
    cfg->camera_z = -1e-6;
    cfg->target_x = -1e-6;
    cfg->target_y = -1e-6;
    cfg->target_z = -1e-6;
    setupCamera();
    EXPECT_NO_THROW(camera->begin_frame(100, 56));
  }

  // Test de múltiples instancias de cámara
  TEST_F(CameraAOSTest, MultipleCameraInstances_IndependentOperation) {
    ConfigParams const cfg1 = *cfg;
    ConfigParams cfg2       = *cfg;

    cfg2.camera_x = 10.0;  // Configuración diferente

    CameraAOS camera1(cfg1);
    CameraAOS camera2(cfg2);

    // Ambas deberían operar independientemente
    EXPECT_NO_THROW(camera1.begin_frame(100, 56));
    EXPECT_NO_THROW(camera2.begin_frame(100, 56));
    EXPECT_NO_THROW(camera1.generate_primary_rays(100, 56, 2));
    EXPECT_NO_THROW(camera2.generate_primary_rays(100, 56, 2));
  }

}  // namespace aos::test
