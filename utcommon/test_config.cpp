// utcommon/test_config.cpp
#include "../../common/include/config.hpp"
#include <gtest/gtest.h>

// Fixture para tests de Config
class ConfigTest : public ::testing::Test {
protected:
  void SetUp() override {
    // Configuración común
  }
};

// Tests de valores por defecto
TEST_F(ConfigTest, DefaultConstructor_SetsDefaultValues) {
  ConfigParams const config;

  EXPECT_EQ(config.aspect_width, 16);
  EXPECT_EQ(config.aspect_height, 9);
  EXPECT_EQ(config.image_width, 1'920);
  EXPECT_DOUBLE_EQ(config.gamma, 2.2);
  EXPECT_DOUBLE_EQ(config.camera_x, 0.0);
  EXPECT_DOUBLE_EQ(config.camera_y, 0.0);
  EXPECT_DOUBLE_EQ(config.camera_z, -10.0);
  EXPECT_DOUBLE_EQ(config.target_x, 0.0);
  EXPECT_DOUBLE_EQ(config.target_y, 0.0);
  EXPECT_DOUBLE_EQ(config.target_z, 0.0);
  EXPECT_DOUBLE_EQ(config.north_x, 0.0);
  EXPECT_DOUBLE_EQ(config.north_y, 1.0);
  EXPECT_DOUBLE_EQ(config.north_z, 0.0);
  EXPECT_DOUBLE_EQ(config.field_of_view, 90.0);
  EXPECT_EQ(config.samples_per_pixel, 20);
  EXPECT_EQ(config.max_depth, 5);
  EXPECT_EQ(config.material_rng_seed, 13);
  EXPECT_EQ(config.ray_rng_seed, 19);
  EXPECT_DOUBLE_EQ(config.background_dark_color_r, 0.25);
  EXPECT_DOUBLE_EQ(config.background_dark_color_g, 0.5);
  EXPECT_DOUBLE_EQ(config.background_dark_color_b, 1.0);
  EXPECT_DOUBLE_EQ(config.background_light_color_r, 1.0);
  EXPECT_DOUBLE_EQ(config.background_light_color_g, 1.0);
  EXPECT_DOUBLE_EQ(config.background_light_color_b, 1.0);
}

// Tests de get_image_height
TEST_F(ConfigTest, GetImageHeight_DefaultAspectRatio_CalculatesCorrectly) {
  ConfigParams const config;

  int const height   = config.get_image_height();
  int const expected = static_cast<int>(1920.0 / (16.0 / 9.0));

  EXPECT_EQ(height, expected);
  EXPECT_EQ(height, 1'080);  // 1920 / (16/9) = 1080
}

TEST_F(ConfigTest, GetImageHeight_CustomAspectRatio_CalculatesCorrectly) {
  ConfigParams config;
  config.aspect_width  = 4;
  config.aspect_height = 3;
  config.image_width   = 800;

  int const height   = config.get_image_height();
  int const expected = static_cast<int>(800.0 / (4.0 / 3.0));

  EXPECT_EQ(height, expected);
  EXPECT_EQ(height, 600);  // 800 / (4/3) = 600
}

TEST_F(ConfigTest, GetImageHeight_SquareAspectRatio_CalculatesCorrectly) {
  ConfigParams config;
  config.aspect_width  = 1;
  config.aspect_height = 1;
  config.image_width   = 500;

  int const height = config.get_image_height();

  EXPECT_EQ(height, 500);  // 500 / (1/1) = 500
}

TEST_F(ConfigTest, GetImageHeight_VeryWideAspectRatio_CalculatesCorrectly) {
  ConfigParams config;
  config.aspect_width  = 21;
  config.aspect_height = 9;
  config.image_width   = 2'560;

  int const height   = config.get_image_height();
  int const expected = static_cast<int>(2560.0 / (21.0 / 9.0));

  EXPECT_EQ(height, expected);
}

TEST_F(ConfigTest, GetImageHeight_VeryTallAspectRatio_CalculatesCorrectly) {
  ConfigParams config;
  config.aspect_width  = 9;
  config.aspect_height = 16;
  config.image_width   = 1'080;

  int const height   = config.get_image_height();
  int const expected = static_cast<int>(1080.0 / (9.0 / 16.0));

  EXPECT_EQ(height, expected);
}

// Tests de modificación de campos
TEST_F(ConfigTest, FieldModification_WorksCorrectly) {
  ConfigParams config;

  // Modificar algunos campos
  config.image_width       = 1'024;
  config.samples_per_pixel = 100;
  config.gamma             = 1.8;
  config.camera_x          = 5.0;
  config.camera_y          = 3.0;
  config.camera_z          = -15.0;

  EXPECT_EQ(config.image_width, 1'024);
  EXPECT_EQ(config.samples_per_pixel, 100);
  EXPECT_DOUBLE_EQ(config.gamma, 1.8);
  EXPECT_DOUBLE_EQ(config.camera_x, 5.0);
  EXPECT_DOUBLE_EQ(config.camera_y, 3.0);
  EXPECT_DOUBLE_EQ(config.camera_z, -15.0);

  // Verificar que get_image_height usa los nuevos valores
  config.aspect_width  = 16;
  config.aspect_height = 10;
  config.image_width   = 1'600;
  int const height     = config.get_image_height();
  EXPECT_EQ(height, 1'000);  // 1600 / (16/10) = 1000
}

// Tests de valores extremos
TEST_F(ConfigTest, ExtremeValues_HandledCorrectly) {
  ConfigParams config;

  // Valores muy grandes
  config.image_width       = 100'000;
  config.samples_per_pixel = 10'000;
  config.max_depth         = 1'000;

  EXPECT_EQ(config.image_width, 100'000);
  EXPECT_EQ(config.samples_per_pixel, 10'000);
  EXPECT_EQ(config.max_depth, 1'000);

  // Valores muy pequeños (pero válidos)
  config.image_width       = 1;
  config.samples_per_pixel = 1;
  config.max_depth         = 1;

  EXPECT_EQ(config.image_width, 1);
  EXPECT_EQ(config.samples_per_pixel, 1);
  EXPECT_EQ(config.max_depth, 1);
}

// Test de copia
TEST_F(ConfigTest, CopyOperations_WorkCorrectly) {
  ConfigParams original;
  original.image_width       = 800;
  original.samples_per_pixel = 50;
  original.gamma             = 2.0;

  // Copy constructor
  ConfigParams const copy(original);
  EXPECT_EQ(copy.image_width, 800);
  EXPECT_EQ(copy.samples_per_pixel, 50);
  EXPECT_DOUBLE_EQ(copy.gamma, 2.0);

  // Copy assignment
  ConfigParams assigned;
  assigned = original;
  EXPECT_EQ(assigned.image_width, 800);
  EXPECT_EQ(assigned.samples_per_pixel, 50);
  EXPECT_DOUBLE_EQ(assigned.gamma, 2.0);

  // Modificar original no afecta copias
  original.image_width = 1'024;
  EXPECT_EQ(copy.image_width, 800);
  EXPECT_EQ(assigned.image_width, 800);
}
