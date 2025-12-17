// utcommon/test_config_parser.cpp
#include "../../common/include/config.hpp"
#include "../../common/include/config_parser.hpp"
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <string>

// Fixture para tests de config parser
class ConfigParserTest : public ::testing::Test {
protected:
  void SetUp() override {
    // Crear archivos de test temporales
    test_dir = std::filesystem::temp_directory_path() / "render_test";
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

// Tests básicos de parse_config
TEST_F(ConfigParserTest, ParseConfig_ValidFile_ParsesCorrectly) {
  std::string const config_content = R"(
aspect_ratio: 16 9
imagewidth: 1920
gamma: 2.2
cameraposition: 0 0 -10
cameratarget: 0 0 0
cameranorth: 0 1 0
fieldofview: 90
samplesperpixel: 20
maxdepth: 5
materialrngseed: 13
rayrngseed: 19
backgrounddarkcolor: 0.25 0.5 1
backgroundlightcolor: 1 1 1
)";

  createTestFile("valid_config.txt", config_content);

  ConfigParams config;
  bool const result = parse_config((test_dir / "valid_config.txt").string(), config);

  EXPECT_TRUE(result);
  EXPECT_EQ(config.aspect_width, 16);
  EXPECT_EQ(config.aspect_height, 9);
  EXPECT_EQ(config.image_width, 1'920);
  EXPECT_DOUBLE_EQ(config.gamma, 2.2);
  EXPECT_DOUBLE_EQ(config.camera_x, 0.0);
  EXPECT_DOUBLE_EQ(config.camera_y, 0.0);
  EXPECT_DOUBLE_EQ(config.camera_z, -10.0);
  EXPECT_EQ(config.samples_per_pixel, 20);
  EXPECT_EQ(config.max_depth, 5);
}

TEST_F(ConfigParserTest, ParseConfig_NonExistentFile_ReturnsFalse) {
  ConfigParams config;
  bool const result = parse_config("nonexistent_file.txt", config);

  EXPECT_FALSE(result);
}

TEST_F(ConfigParserTest, ParseConfig_EmptyFile_ReturnsTrue) {
  createTestFile("empty_config.txt", "");

  ConfigParams config;
  bool const result = parse_config((test_dir / "empty_config.txt").string(), config);

  EXPECT_TRUE(result);  // Archivo vacío es válido
}

TEST_F(ConfigParserTest, ParseConfig_FileWithCommentsAndEmptyLines_ParsesCorrectly) {
  std::string const config_content = R"(
# This is a comment
aspect_ratio: 4 3

imagewidth: 800
# Another comment

gamma: 1.8

)";

  createTestFile("commented_config.txt", config_content);

  ConfigParams config;
  bool const result = parse_config((test_dir / "commented_config.txt").string(), config);

  EXPECT_TRUE(result);
  EXPECT_EQ(config.aspect_width, 4);
  EXPECT_EQ(config.aspect_height, 3);
  EXPECT_EQ(config.image_width, 800);
  EXPECT_DOUBLE_EQ(config.gamma, 1.8);
}

// Tests de handlers individuales
TEST_F(ConfigParserTest, ParseConfig_IndividualHandlers_WorkCorrectly) {
  // Test aspect_ratio
  {
    std::string const content = "aspect_ratio: 3 2\n";
    createTestFile("test_aspect.txt", content);

    ConfigParams config;
    bool const result = parse_config((test_dir / "test_aspect.txt").string(), config);

    EXPECT_TRUE(result);
    EXPECT_EQ(config.aspect_width, 3);
    EXPECT_EQ(config.aspect_height, 2);
  }

  // Test imagewidth
  {
    std::string const content = "imagewidth: 1024\n";
    createTestFile("test_width.txt", content);

    ConfigParams config;
    bool const result = parse_config((test_dir / "test_width.txt").string(), config);

    EXPECT_TRUE(result);
    EXPECT_EQ(config.image_width, 1'024);
  }

  // Test gamma
  {
    std::string const content = "gamma: 1.5\n";
    createTestFile("test_gamma.txt", content);

    ConfigParams config;
    bool const result = parse_config((test_dir / "test_gamma.txt").string(), config);

    EXPECT_TRUE(result);
    EXPECT_DOUBLE_EQ(config.gamma, 1.5);
  }
}

// Tests de casos error
TEST_F(ConfigParserTest, ParseConfig_InvalidAspectRatio_ReturnsFalse) {
  std::string const config_content = "aspect_ratio: 0 0\n";  // Valores inválidos

  createTestFile("invalid_aspect.txt", config_content);

  ConfigParams config;
  bool const result = parse_config((test_dir / "invalid_aspect.txt").string(), config);

  EXPECT_FALSE(result);
}

TEST_F(ConfigParserTest, ParseConfig_InvalidImageWidth_ReturnsFalse) {
  std::string const config_content = "imagewidth: -100\n";  // Valor negativo

  createTestFile("invalid_width.txt", config_content);

  ConfigParams config;
  bool const result = parse_config((test_dir / "invalid_width.txt").string(), config);

  EXPECT_FALSE(result);
}

TEST_F(ConfigParserTest, ParseConfig_InvalidFieldOfView_ReturnsFalse) {
  std::string const config_content = "fieldofview: 200\n";  // Fuera de rango

  createTestFile("invalid_fov.txt", config_content);

  ConfigParams config;
  bool const result = parse_config((test_dir / "invalid_fov.txt").string(), config);

  EXPECT_FALSE(result);
}

TEST_F(ConfigParserTest, ParseConfig_UnknownKey_ReturnsFalse) {
  std::string const config_content = "unknown_key: 100\n";

  createTestFile("unknown_key.txt", config_content);

  ConfigParams config;
  bool const result = parse_config((test_dir / "unknown_key.txt").string(), config);

  EXPECT_FALSE(result);
}

TEST_F(ConfigParserTest, ParseConfig_MissingColon_ReturnsFalse) {
  std::string const config_content = "imagewidth 1920\n";  // Falta :

  createTestFile("missing_colon.txt", config_content);

  ConfigParams config;
  bool const result = parse_config((test_dir / "missing_colon.txt").string(), config);

  EXPECT_FALSE(result);
}

// Tests de valores límite
TEST_F(ConfigParserTest, ParseConfig_BoundaryValues_HandledCorrectly) {
  std::string const config_content = R"(
fieldofview: 1
samplesperpixel: 1
maxdepth: 1
)";

  createTestFile("boundary_config.txt", config_content);

  ConfigParams config;
  bool const result = parse_config((test_dir / "boundary_config.txt").string(), config);

  EXPECT_TRUE(result);
  EXPECT_DOUBLE_EQ(config.field_of_view, 1.0);
  EXPECT_EQ(config.samples_per_pixel, 1);
  EXPECT_EQ(config.max_depth, 1);
}

// Test de integración con get_image_height
TEST_F(ConfigParserTest, Integration_ParsedConfig_GetImageHeightWorks) {
  std::string const config_content = R"(
aspect_ratio: 16 9
imagewidth: 1920
)";

  createTestFile("integration_config.txt", config_content);

  ConfigParams config;
  bool const result = parse_config((test_dir / "integration_config.txt").string(), config);

  EXPECT_TRUE(result);
  EXPECT_EQ(config.get_image_height(), 1'080);
}
