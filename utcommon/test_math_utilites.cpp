#include "../../common/include/math_utilities.hpp"
#include "../../common/include/vector.hpp"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <gtest/gtest.h>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace {

  // Función helper en anonymous namespace para parsear output de color
  std::vector<int> parseColorOutput(std::string const & output) {
    std::vector<int> components;
    std::istringstream iss(output);
    int r = 0;
    int g = 0;
    int b = 0;
    iss >> r >> g >> b;
    components.push_back(r);
    components.push_back(g);
    components.push_back(b);
    return components;
  }

}  // anonymous namespace

// Fixture para tests de RNG
class RNGTest : public ::testing::Test {
protected:
  uint64_t deterministic_seed{};
  std::unique_ptr<render::RNG> rng;

  void SetUp() override {
    // Configuración común para tests determinísticos
    deterministic_seed = 42;
    rng                = std::make_unique<render::RNG>(deterministic_seed);
  }
};

// Tests de construcción de RNG
TEST_F(RNGTest, Constructor_WithSeed_InitializesCorrectly) {
  // No debería lanzar excepciones
  EXPECT_NO_THROW(render::RNG const rng(123));
}

// Tests de random_double() sin parámetros
TEST_F(RNGTest, RandomDouble_NoParameters_ReturnsValuesInZeroToOneRange) {
  int const num_samples = 1'000;

  for (int i = 0; i < num_samples; ++i) {
    double const value = rng->random_double();
    EXPECT_GE(value, 0.0);
    EXPECT_LT(value, 1.0);
  }
}

TEST_F(RNGTest, RandomDouble_NoParameters_DifferentSeedsProduceDifferentSequences) {
  render::RNG rng1(100);
  render::RNG rng2(200);

  // Los primeros valores deberían ser diferentes con seeds diferentes
  double const val1 = rng1.random_double();
  double const val2 = rng2.random_double();

  EXPECT_NE(val1, val2);
}

TEST_F(RNGTest, RandomDouble_NoParameters_SameSeedProducesSameSequence) {
  render::RNG rng1(555);
  render::RNG rng2(555);

  // Misma seed debería producir misma secuencia
  for (int i = 0; i < 10; ++i) {
    EXPECT_DOUBLE_EQ(rng1.random_double(), rng2.random_double());
  }
}

// Tests de random_double() con rango
TEST_F(RNGTest, RandomDouble_WithRange_ReturnsValuesInSpecifiedRange) {
  double const min      = -5.0;
  double const max      = 10.0;
  int const num_samples = 1'000;

  for (int i = 0; i < num_samples; ++i) {
    double const value = rng->random_double(min, max);
    EXPECT_GE(value, min);
    EXPECT_LT(value, max);
  }
}

TEST_F(RNGTest, RandomDouble_WithRange_SameSeedProducesSameSequence) {
  double const min = 0.0;
  double const max = 100.0;

  render::RNG rng1(777);
  render::RNG rng2(777);

  for (int i = 0; i < 10; ++i) {
    EXPECT_DOUBLE_EQ(rng1.random_double(min, max), rng2.random_double(min, max));
  }
}

TEST_F(RNGTest, RandomDouble_WithRange_EdgeCases) {
  // Rango cero
  EXPECT_DOUBLE_EQ(rng->random_double(5.0, 5.0), 5.0);

  // Rango negativo a positivo
  double value = rng->random_double(-10.0, 10.0);
  EXPECT_GE(value, -10.0);
  EXPECT_LT(value, 10.0);

  // Rango pequeño
  value = rng->random_double(0.0, 0.001);
  EXPECT_GE(value, 0.0);
  EXPECT_LT(value, 0.001);
}

// Tests de random_vec
TEST_F(RNGTest, RandomVec_DefaultRange_ReturnsVectorsInMinusOneToOne) {
  int const num_samples = 100;

  for (int i = 0; i < num_samples; ++i) {
    render::direction_vector const vec = render::random_vec(*rng);

    EXPECT_GE(vec.get_x(), -1.0);
    EXPECT_LT(vec.get_x(), 1.0);
    EXPECT_GE(vec.get_y(), -1.0);
    EXPECT_LT(vec.get_y(), 1.0);
    EXPECT_GE(vec.get_z(), -1.0);
    EXPECT_LT(vec.get_z(), 1.0);
  }
}

TEST_F(RNGTest, RandomVec_CustomRange_ReturnsVectorsInSpecifiedRange) {
  double const min      = -2.5;
  double const max      = 3.5;
  int const num_samples = 100;

  for (int i = 0; i < num_samples; ++i) {
    render::direction_vector const vec = render::random_vec(*rng, min, max);

    EXPECT_GE(vec.get_x(), min);
    EXPECT_LT(vec.get_x(), max);
    EXPECT_GE(vec.get_y(), min);
    EXPECT_LT(vec.get_y(), max);
    EXPECT_GE(vec.get_z(), min);
    EXPECT_LT(vec.get_z(), max);
  }
}

TEST_F(RNGTest, RandomVec_SameSeedProducesSameVectors) {
  render::RNG rng1(888);
  render::RNG rng2(888);

  for (int i = 0; i < 5; ++i) {
    render::direction_vector const vec1 = render::random_vec(rng1);
    render::direction_vector const vec2 = render::random_vec(rng2);

    EXPECT_NEAR(vec1.get_x(), vec2.get_x(), 1e-6);
    EXPECT_NEAR(vec1.get_y(), vec2.get_y(), 1e-6);
    EXPECT_NEAR(vec1.get_z(), vec2.get_z(), 1e-6);
  }
}

// Fixture para tests de write_color
class WriteColorTest : public ::testing::Test {
protected:
  void SetUp() override { output_stream = std::make_unique<std::ostringstream>(); }

  std::unique_ptr<std::ostringstream> output_stream;
};

// Tests básicos de write_color
TEST_F(WriteColorTest, WriteColor_BlackColor_OutputsZeros) {
  render::color_vector const black(0.0, 0.0, 0.0);
  int const samples  = 1;
  double const gamma = 2.2;

  render::write_color(*output_stream, black, samples, gamma);

  auto components = parseColorOutput(output_stream->str());
  EXPECT_EQ(components[0], 0);
  EXPECT_EQ(components[1], 0);
  EXPECT_EQ(components[2], 0);
}

TEST_F(WriteColorTest, WriteColor_WhiteColor_Outputs255s) {
  render::color_vector const white(1.0, 1.0, 1.0);
  int const samples  = 1;
  double const gamma = 2.2;

  render::write_color(*output_stream, white, samples, gamma);

  auto components = parseColorOutput(output_stream->str());
  EXPECT_EQ(components[0], 255);
  EXPECT_EQ(components[1], 255);
  EXPECT_EQ(components[2], 255);
}

TEST_F(WriteColorTest, WriteColor_MidGray_OutputsCorrectValues) {
  render::color_vector const gray(0.5, 0.5, 0.5);
  int const samples  = 1;
  double const gamma = 2.2;

  render::write_color(*output_stream, gray, samples, gamma);

  auto components = parseColorOutput(output_stream->str());
  // 0.5^(1/2.2) ≈ 0.73, 0.73 * 256 ≈ 187
  EXPECT_NEAR(components[0], 187, 1);
  EXPECT_NEAR(components[1], 187, 1);
  EXPECT_NEAR(components[2], 187, 1);
}

// Tests de muestreo múltiple
TEST_F(WriteColorTest, WriteColor_MultipleSamples_AveragesCorrectly) {
  render::color_vector const color(2.0, 2.0, 2.0);  // Valores mayores que 1
  int const samples  = 4;                           // 2.0 / 4 = 0.5
  double const gamma = 2.2;

  render::write_color(*output_stream, color, samples, gamma);

  auto components = parseColorOutput(output_stream->str());
  // Debería ser equivalente a color (0.5, 0.5, 0.5) después del promedio
  EXPECT_NEAR(components[0], 187, 1);
  EXPECT_NEAR(components[1], 187, 1);
  EXPECT_NEAR(components[2], 187, 1);
}

TEST_F(WriteColorTest, WriteColor_SingleSample_NoAveraging) {
  render::color_vector const color(0.8, 0.6, 0.4);
  int const samples  = 1;
  double const gamma = 1.0;  // Sin corrección gamma para verificar directamente

  render::write_color(*output_stream, color, samples, gamma);

  auto components = parseColorOutput(output_stream->str());
  EXPECT_NEAR(components[0], 205, 1);  // 0.8 * 256 ≈ 205
  EXPECT_NEAR(components[1], 154, 1);  // 0.6 * 256 ≈ 154
  EXPECT_NEAR(components[2], 102, 1);  // 0.4 * 256 ≈ 102
}

// Tests de corrección gamma
TEST_F(WriteColorTest, WriteColor_GammaCorrection_AppliesCorrectly) {
  render::color_vector const color(0.5, 0.5, 0.5);
  int const samples  = 1;
  double const gamma = 2.0;  // Corrección cuadrática

  render::write_color(*output_stream, color, samples, gamma);

  auto components = parseColorOutput(output_stream->str());
  // 0.5^(1/2.0) = sqrt(0.5) ≈ 0.707, 0.707 * 256 ≈ 181
  EXPECT_NEAR(components[0], 181, 1);
  EXPECT_NEAR(components[1], 181, 1);
  EXPECT_NEAR(components[2], 181, 1);
}

TEST_F(WriteColorTest, WriteColor_GammaOne_NoCorrection) {
  render::color_vector const color(0.5, 0.5, 0.5);
  int const samples  = 1;
  double const gamma = 1.0;  // Sin corrección

  render::write_color(*output_stream, color, samples, gamma);

  auto components = parseColorOutput(output_stream->str());
  // 0.5 * 256 = 128
  EXPECT_EQ(components[0], 128);
  EXPECT_EQ(components[1], 128);
  EXPECT_EQ(components[2], 128);
}

// Tests de clamping
TEST_F(WriteColorTest, WriteColor_NegativeValues_ClampedToZero) {
  render::color_vector const color(-1.0, -0.5, -0.1);
  int const samples  = 1;
  double const gamma = 2.2;

  render::write_color(*output_stream, color, samples, gamma);

  auto components = parseColorOutput(output_stream->str());
  EXPECT_EQ(components[0], 0);
  EXPECT_EQ(components[1], 0);
  EXPECT_EQ(components[2], 0);
}

TEST_F(WriteColorTest, WriteColor_ValuesAboveOne_ClampedToOne) {
  render::color_vector const color(2.0, 1.5, 1.1);
  int const samples  = 1;
  double const gamma = 2.2;

  render::write_color(*output_stream, color, samples, gamma);

  auto components = parseColorOutput(output_stream->str());
  EXPECT_EQ(components[0], 255);
  EXPECT_EQ(components[1], 255);
  EXPECT_EQ(components[2], 255);
}

TEST_F(WriteColorTest, WriteColor_MixedValues_ClampedCorrectly) {
  render::color_vector const color(-0.5, 0.5, 1.5);
  int const samples  = 1;
  double const gamma = 2.2;

  render::write_color(*output_stream, color, samples, gamma);

  auto components = parseColorOutput(output_stream->str());
  EXPECT_EQ(components[0], 0);         // Clamped from negative
  EXPECT_NEAR(components[1], 187, 1);  // Normal value
  EXPECT_EQ(components[2], 255);       // Clamped from above 1
}

// Tests de formato de salida
TEST_F(WriteColorTest, WriteColor_OutputFormat_CorrectPPMFormat) {
  render::color_vector const color(1.0, 0.5, 0.0);
  int const samples  = 1;
  double const gamma = 2.2;

  render::write_color(*output_stream, color, samples, gamma);

  std::string output = output_stream->str();
  // Debería tener el formato "R G B\n"
  EXPECT_NE(output.find(' '), std::string::npos);
  EXPECT_EQ(output.back(), '\n');

  // Verificar que tiene exactamente 3 componentes
  auto components = parseColorOutput(output);
  EXPECT_EQ(components.size(), 3);
}

// Tests de casos edge
TEST_F(WriteColorTest, WriteColor_ZeroSamples_HandlesGracefully) {
  render::color_vector const color(1.0, 1.0, 1.0);
  int const samples = 0;  // División por cero potencial

  // Esto podría causar división por cero, pero la función debería manejarlo
  // Si causa problemas, necesitaríamos modificar la función
  EXPECT_NO_THROW(render::write_color(*output_stream, color, samples, 2.2));
}

TEST_F(WriteColorTest, WriteColor_GammaZero_HandlesGracefully) {
  render::color_vector const color(0.5, 0.5, 0.5);
  int const samples  = 1;
  double const gamma = 0.0;  // División por cero potencial

  // Esto podría causar división por cero en 1.0/gamma
  EXPECT_NO_THROW(render::write_color(*output_stream, color, samples, gamma));
}

// Test de integración entre RNG y random_vec
TEST_F(RNGTest, Integration_RNGAndRandomVec_WorkTogether) {
  render::RNG test_rng(999);

  // Generar varios vectores y verificar que están en el rango correcto
  for (int i = 0; i < 10; ++i) {
    render::direction_vector const vec = render::random_vec(test_rng, -10.0, 10.0);

    EXPECT_GE(vec.get_x(), -10.0);
    EXPECT_LT(vec.get_x(), 10.0);
    EXPECT_GE(vec.get_y(), -10.0);
    EXPECT_LT(vec.get_y(), 10.0);
    EXPECT_GE(vec.get_z(), -10.0);
    EXPECT_LT(vec.get_z(), 10.0);
  }
}

// Test adicional para verificar distribución estadística básica
TEST_F(RNGTest, RandomDouble_Distribution_ReasonableSpread) {
  int const num_samples = 10'000;
  double sum            = 0.0;
  double min_val        = 1.0;
  double max_val        = 0.0;

  for (int i = 0; i < num_samples; ++i) {
    double const val = rng->random_double();
    sum += val;
    min_val = std::min(min_val, val);
    max_val = std::max(max_val, val);
  }

  double const average = sum / num_samples;
  // El promedio debería estar cerca de 0.5 para una distribución uniforme
  EXPECT_NEAR(average, 0.5, 0.01);
  // Deberíamos tener valores cerca de 0 y 1
  EXPECT_LT(min_val, 0.01);
  EXPECT_GT(max_val, 0.99);
}
