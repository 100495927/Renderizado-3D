// utaos/test_render_aos.cpp
#include "../../aos/include/aos_camera.hpp"
#include "../../aos/include/aos_image.hpp"
#include "../../aos/include/render_aos.hpp"
#include "../../common/include/config.hpp"
#include "../../common/include/materials.hpp"
#include "../../common/include/objects.hpp"
#include "../../common/include/scene_parser.hpp"

#include <chrono>
#include <functional>
#include <gtest/gtest.h>
#include <memory>
#include <vector>

namespace aos::test {

  class RenderAOSTest : public ::testing::Test {
  protected:
    void SetUp() override {
      // Configuración básica por defecto
      setupBasicConfig();
      setupBasicScene();
      setupCamera();
      setupImage();
    }

    void setupBasicConfig() {
      cfg                    = std::make_unique<ConfigParams>();
      cfg->image_width       = 100;
      cfg->aspect_width      = 16;
      cfg->aspect_height     = 9;
      cfg->samples_per_pixel = 2;
      cfg->max_depth         = 3;
      cfg->gamma             = 2.2;
      cfg->ray_rng_seed      = 42;
      cfg->material_rng_seed = 43;

      // Colores de fondo
      cfg->background_light_color_r = 1.0;
      cfg->background_light_color_g = 1.0;
      cfg->background_light_color_b = 1.0;
      cfg->background_dark_color_r  = 0.5;
      cfg->background_dark_color_g  = 0.7;
      cfg->background_dark_color_b  = 1.0;

      // Cámara - usando los nombres correctos del config.hpp
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
    }

    void setupBasicScene() {
      // Crear vectores primero y luego SceneOutput
      auto matte_materials      = std::vector<MatteMaterial>{};
      auto metal_materials      = std::vector<MetalMaterial>{};
      auto refractive_materials = std::vector<RefractiveMaterial>{};
      auto spheres              = std::vector<Sphere>{};
      auto cylinders            = std::vector<Cylinder>{};

      scene = std::make_unique<SceneOutput>(std::ref(matte_materials), std::ref(metal_materials),
                                            std::ref(refractive_materials), std::ref(spheres),
                                            std::ref(cylinders));

      // Material básico mate
      MatteMaterial matte;
      matte.name          = "test_matte";
      matte.reflectance_r = 0.8;
      matte.reflectance_g = 0.2;
      matte.reflectance_b = 0.2;
      scene->matte_materials.get().push_back(matte);

      // Esfera básica
      Sphere sphere;
      sphere.center_x      = 0.0;
      sphere.center_y      = 0.0;
      sphere.center_z      = 0.0;
      sphere.radius        = 1.0;
      sphere.material_name = "test_matte";
      scene->spheres.get().push_back(sphere);

      // Cilindro básico
      Cylinder cylinder;
      cylinder.center_x      = 2.0;
      cylinder.center_y      = 0.0;
      cylinder.center_z      = 0.0;
      cylinder.radius        = 0.5;
      cylinder.axis_x        = 0.0;
      cylinder.axis_y        = 2.0;
      cylinder.axis_z        = 0.0;
      cylinder.material_name = "test_matte";
      scene->cylinders.get().push_back(cylinder);
    }

    void setupCamera() { camera = std::make_unique<CameraAOS>(*cfg); }

    void setupImage() {
      int const height = cfg->get_image_height();
      image            = std::make_unique<AOSImage>(cfg->image_width, height);
    }

    // Helper para crear escena vacía
    static std::unique_ptr<SceneOutput> createEmptyScene() {
      auto matte_materials      = std::vector<MatteMaterial>{};
      auto metal_materials      = std::vector<MetalMaterial>{};
      auto refractive_materials = std::vector<RefractiveMaterial>{};
      auto spheres              = std::vector<Sphere>{};
      auto cylinders            = std::vector<Cylinder>{};

      return std::make_unique<SceneOutput>(std::ref(matte_materials), std::ref(metal_materials),
                                           std::ref(refractive_materials), std::ref(spheres),
                                           std::ref(cylinders));
    }

    std::unique_ptr<ConfigParams> cfg;
    std::unique_ptr<SceneOutput> scene;
    std::unique_ptr<CameraAOS> camera;
    std::unique_ptr<AOSImage> image;
  };

  // Tests básicos de funcionamiento
  TEST_F(RenderAOSTest, RenderScene_CompletesWithoutCrash) {
    EXPECT_NO_THROW({ render_scene(*cfg, *scene, *camera, *image); });
  }

  TEST_F(RenderAOSTest, RenderScene_ProducesImageWithCorrectDimensions) {
    render_scene(*cfg, *scene, *camera, *image);

    int const expected_height = cfg->get_image_height();
    EXPECT_EQ(image->width(), cfg->image_width);
    EXPECT_EQ(image->height(), expected_height);
  }

  // Tests de parámetros de configuración
  TEST_F(RenderAOSTest, RenderScene_HandlesDifferentSamplesPerPixel) {
    cfg->samples_per_pixel = 1;
    EXPECT_NO_THROW(render_scene(*cfg, *scene, *camera, *image));

    cfg->samples_per_pixel = 10;
    EXPECT_NO_THROW(render_scene(*cfg, *scene, *camera, *image));
  }

  TEST_F(RenderAOSTest, RenderScene_HandlesDifferentMaxDepth) {
    cfg->max_depth = 1;
    EXPECT_NO_THROW(render_scene(*cfg, *scene, *camera, *image));

    cfg->max_depth = 10;
    EXPECT_NO_THROW(render_scene(*cfg, *scene, *camera, *image));
  }

  TEST_F(RenderAOSTest, RenderScene_HandlesDifferentImageSizes) {
    // Imagen pequeña
    cfg->image_width = 10;
    setupImage();
    EXPECT_NO_THROW(render_scene(*cfg, *scene, *camera, *image));

    // Imagen más grande
    cfg->image_width = 200;
    setupImage();
    EXPECT_NO_THROW(render_scene(*cfg, *scene, *camera, *image));
  }

  // Tests de escenas especiales
  TEST_F(RenderAOSTest, RenderScene_EmptyScene_CompletesWithoutCrash) {
    auto empty_scene = createEmptyScene();
    EXPECT_NO_THROW(render_scene(*cfg, *empty_scene, *camera, *image));
  }

  TEST_F(RenderAOSTest, RenderScene_OnlySpheres_CompletesWithoutCrash) {
    auto sphere_scene = createEmptyScene();

    // Material para esferas
    MatteMaterial matte;
    matte.name          = "sphere_matte";
    matte.reflectance_r = 0.2;
    matte.reflectance_g = 0.8;
    matte.reflectance_b = 0.2;
    sphere_scene->matte_materials.get().push_back(matte);

    // Múltiples esferas
    Sphere sphere1;
    sphere1.center_x      = -1.0;
    sphere1.center_y      = 0.0;
    sphere1.center_z      = 0.0;
    sphere1.radius        = 0.5;
    sphere1.material_name = "sphere_matte";
    sphere_scene->spheres.get().push_back(sphere1);

    Sphere sphere2;
    sphere2.center_x      = 1.0;
    sphere2.center_y      = 0.0;
    sphere2.center_z      = 0.0;
    sphere2.radius        = 0.5;
    sphere2.material_name = "sphere_matte";
    sphere_scene->spheres.get().push_back(sphere2);

    EXPECT_NO_THROW(render_scene(*cfg, *sphere_scene, *camera, *image));
  }

  TEST_F(RenderAOSTest, RenderScene_OnlyCylinders_CompletesWithoutCrash) {
    auto cylinder_scene = createEmptyScene();

    // Material para cilindros
    MetalMaterial metal;
    metal.name          = "cylinder_metal";
    metal.reflectance_r = 0.8;
    metal.reflectance_g = 0.8;
    metal.reflectance_b = 0.2;
    metal.diffusion     = 0.1;
    cylinder_scene->metal_materials.get().push_back(metal);

    // Múltiples cilindros
    Cylinder cylinder1;
    cylinder1.center_x      = -2.0;
    cylinder1.center_y      = 0.0;
    cylinder1.center_z      = 0.0;
    cylinder1.radius        = 0.3;
    cylinder1.axis_x        = 0.0;
    cylinder1.axis_y        = 1.5;
    cylinder1.axis_z        = 0.0;
    cylinder1.material_name = "cylinder_metal";
    cylinder_scene->cylinders.get().push_back(cylinder1);

    Cylinder cylinder2;
    cylinder2.center_x      = 2.0;
    cylinder2.center_y      = 0.0;
    cylinder2.center_z      = 0.0;
    cylinder2.radius        = 0.3;
    cylinder2.axis_x        = 0.0;
    cylinder2.axis_y        = 1.5;
    cylinder2.axis_z        = 0.0;
    cylinder2.material_name = "cylinder_metal";
    cylinder_scene->cylinders.get().push_back(cylinder2);

    EXPECT_NO_THROW(render_scene(*cfg, *cylinder_scene, *camera, *image));
  }

  // Tests de materiales
  TEST_F(RenderAOSTest, RenderScene_DifferentMaterialTypes_CompletesWithoutCrash) {
    auto material_scene = createEmptyScene();

    // Material mate
    MatteMaterial matte;
    matte.name          = "matte";
    matte.reflectance_r = 0.8;
    matte.reflectance_g = 0.2;
    matte.reflectance_b = 0.2;
    material_scene->matte_materials.get().push_back(matte);

    // Material metal
    MetalMaterial metal;
    metal.name          = "metal";
    metal.reflectance_r = 0.8;
    metal.reflectance_g = 0.8;
    metal.reflectance_b = 0.8;
    metal.diffusion     = 0.2;
    material_scene->metal_materials.get().push_back(metal);

    // Material refractivo
    RefractiveMaterial refractive;
    refractive.name             = "refractive";
    refractive.refractive_index = 1.5;
    material_scene->refractive_materials.get().push_back(refractive);

    // Objetos con diferentes materiales
    Sphere sphere_matte;
    sphere_matte.center_x      = -2.0;
    sphere_matte.center_y      = 0.0;
    sphere_matte.center_z      = 0.0;
    sphere_matte.radius        = 0.7;
    sphere_matte.material_name = "matte";
    material_scene->spheres.get().push_back(sphere_matte);

    Sphere sphere_metal;
    sphere_metal.center_x      = 0.0;
    sphere_metal.center_y      = 0.0;
    sphere_metal.center_z      = 0.0;
    sphere_metal.radius        = 0.7;
    sphere_metal.material_name = "metal";
    material_scene->spheres.get().push_back(sphere_metal);

    Sphere sphere_refractive;
    sphere_refractive.center_x      = 2.0;
    sphere_refractive.center_y      = 0.0;
    sphere_refractive.center_z      = 0.0;
    sphere_refractive.radius        = 0.7;
    sphere_refractive.material_name = "refractive";
    material_scene->spheres.get().push_back(sphere_refractive);

    EXPECT_NO_THROW(render_scene(*cfg, *material_scene, *camera, *image));
  }

  // Tests de casos límite
  TEST_F(RenderAOSTest, RenderScene_MinimalImageSize_CompletesWithoutCrash) {
    cfg->image_width       = 1;
    cfg->samples_per_pixel = 1;
    cfg->max_depth         = 1;
    setupImage();

    EXPECT_NO_THROW(render_scene(*cfg, *scene, *camera, *image));
    EXPECT_EQ(image->width(), 1);
    EXPECT_EQ(image->height(), cfg->get_image_height());
  }

  TEST_F(RenderAOSTest, RenderScene_ZeroMaxDepth_CompletesWithoutCrash) {
    cfg->max_depth = 0;
    EXPECT_NO_THROW(render_scene(*cfg, *scene, *camera, *image));
  }

  TEST_F(RenderAOSTest, RenderScene_SingleSample_CompletesWithoutCrash) {
    cfg->samples_per_pixel = 1;
    EXPECT_NO_THROW(render_scene(*cfg, *scene, *camera, *image));
  }

  // Tests de robustez
  TEST_F(RenderAOSTest, RenderScene_MissingMaterial_CompletesWithoutCrash) {
    auto scene_missing_material = createEmptyScene();

    // Esfera con material que no existe
    Sphere sphere;
    sphere.center_x      = 0.0;
    sphere.center_y      = 0.0;
    sphere.center_z      = 0.0;
    sphere.radius        = 1.0;
    sphere.material_name = "non_existent_material";
    scene_missing_material->spheres.get().push_back(sphere);

    // No debería crashar
    EXPECT_NO_THROW(render_scene(*cfg, *scene_missing_material, *camera, *image));
  }

  TEST_F(RenderAOSTest, RenderScene_ObjectsBehindCamera_CompletesWithoutCrash) {
    auto scene_behind = createEmptyScene();

    MatteMaterial matte;
    matte.name          = "behind_matte";
    matte.reflectance_r = 0.5;
    matte.reflectance_g = 0.5;
    matte.reflectance_b = 0.5;
    scene_behind->matte_materials.get().push_back(matte);

    // Esfera detrás de la cámara
    Sphere sphere;
    sphere.center_x      = 0.0;
    sphere.center_y      = 0.0;
    sphere.center_z      = -10.0;  // Detrás de la cámara en z = -5
    sphere.radius        = 1.0;
    sphere.material_name = "behind_matte";
    scene_behind->spheres.get().push_back(sphere);

    EXPECT_NO_THROW(render_scene(*cfg, *scene_behind, *camera, *image));
  }

  // Test de rendimiento básico
  TEST_F(RenderAOSTest, RenderScene_Performance_CompletesInReasonableTime) {
    // Configuración pequeña para test de rendimiento
    cfg->image_width       = 50;
    cfg->samples_per_pixel = 1;
    cfg->max_depth         = 2;
    setupImage();

    auto start = std::chrono::high_resolution_clock::now();

    render_scene(*cfg, *scene, *camera, *image);

    auto end      = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // Verificar que no tarda más de 10 segundos para esta configuración pequeña
    EXPECT_LT(duration.count(), 10'000)
        << "Render debería completarse en menos de 10 segundos para configuración pequeña";
  }

  // Test de configuración de cámara
  TEST_F(RenderAOSTest, RenderScene_DifferentCameraPositions_CompletesWithoutCrash) {
    // Cámara desde diferentes ángulos
    cfg->camera_x = 5.0;
    cfg->camera_y = 2.0;
    cfg->camera_z = -3.0;
    setupCamera();

    EXPECT_NO_THROW(render_scene(*cfg, *scene, *camera, *image));

    // Otro ángulo
    cfg->camera_x = -3.0;
    cfg->camera_y = 1.0;
    cfg->camera_z = -4.0;
    setupCamera();

    EXPECT_NO_THROW(render_scene(*cfg, *scene, *camera, *image));
  }

  // Test de campo de visión
  TEST_F(RenderAOSTest, RenderScene_DifferentFieldOfView_CompletesWithoutCrash) {
    cfg->field_of_view = 45.0;  // Angosto
    setupCamera();
    EXPECT_NO_THROW(render_scene(*cfg, *scene, *camera, *image));

    cfg->field_of_view = 120.0;  // Ancho
    setupCamera();
    EXPECT_NO_THROW(render_scene(*cfg, *scene, *camera, *image));
  }

  // Test de relación de aspecto
  TEST_F(RenderAOSTest, RenderScene_DifferentAspectRatios_CompletesWithoutCrash) {
    cfg->aspect_width  = 4;
    cfg->aspect_height = 3;
    setupImage();
    EXPECT_NO_THROW(render_scene(*cfg, *scene, *camera, *image));

    cfg->aspect_width  = 16;
    cfg->aspect_height = 10;
    setupImage();
    EXPECT_NO_THROW(render_scene(*cfg, *scene, *camera, *image));
  }

}  // namespace aos::test
