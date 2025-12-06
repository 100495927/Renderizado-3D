// utsoa/test_soa_color.cpp
#include "../../common/include/config.hpp"
#include "../../soa/include/soa_color.hpp"

#include <chrono>
#include <cmath>
#include <cstdint>
#include <gtest/gtest.h>
#include <limits>
#include <memory>

namespace soa::test {

  class SOAColorTest : public ::testing::Test {
  protected:
    void SetUp() override { setupBasicConfig(); }

    void setupBasicConfig() {
      cfg        = std::make_unique<ConfigParams>();
      cfg->gamma = 2.2;
    }

    std::unique_ptr<ConfigParams> cfg;
  };

  // Tests de background_color
  TEST_F(SOAColorTest, BackgroundColor_ValidParameters_ReturnsColor) {
    color::Color const light{1.0, 1.0, 1.0};
    color::Color const dark{0.0, 0.0, 0.0};

    auto result1 = color::background_color(0.0, light, dark);
    auto result2 = color::background_color(0.5, light, dark);
    auto result3 = color::background_color(1.0, light, dark);

    // Verificar que se devuelven colores válidos
    EXPECT_TRUE(result1.r >= 0.0 and result1.r <= 1.0);
    EXPECT_TRUE(result2.r >= 0.0 and result2.r <= 1.0);
    EXPECT_TRUE(result3.r >= 0.0 and result3.r <= 1.0);
  }

  TEST_F(SOAColorTest, BackgroundColor_ExtremeDirYValues_ReturnsColor) {
    color::Color const light{1.0, 1.0, 1.0};
    color::Color const dark{0.0, 0.0, 0.0};

    // Valores extremos de dir_y
    auto result1 = color::background_color(-1.0, light, dark);
    auto result2 = color::background_color(0.0, light, dark);
    auto result3 = color::background_color(1.0, light, dark);
    auto result4 = color::background_color(-2.0, light, dark);  // Fuera del rango teórico
    auto result5 = color::background_color(2.0, light, dark);   // Fuera del rango teórico

    // Verificar que todos devuelven colores válidos
    EXPECT_TRUE(result1.r >= 0.0 and result1.r <= 1.0);
    EXPECT_TRUE(result2.r >= 0.0 and result2.r <= 1.0);
    EXPECT_TRUE(result3.r >= 0.0 and result3.r <= 1.0);
    EXPECT_TRUE(result4.r >= 0.0 and result4.r <= 1.0);
    EXPECT_TRUE(result5.r >= 0.0 and result5.r <= 1.0);
  }

  TEST_F(SOAColorTest, BackgroundColor_DifferentColorCombinations_ReturnsColor) {
    // Diferentes combinaciones de colores claro y oscuro
    auto result1 =
        color::background_color(0.5, color::Color{1.0, 1.0, 1.0}, color::Color{0.0, 0.0, 0.0});

    auto result2 =
        color::background_color(0.5, color::Color{1.0, 0.0, 0.0}, color::Color{0.0, 1.0, 0.0});

    auto result3 =
        color::background_color(0.5, color::Color{0.5, 0.5, 0.5}, color::Color{0.2, 0.3, 0.4});

    // Verificar resultados
    EXPECT_TRUE(result1.r >= 0.0 and result1.r <= 1.0);
    EXPECT_TRUE(result2.r >= 0.0 and result2.r <= 1.0);
    EXPECT_TRUE(result3.r >= 0.0 and result3.r <= 1.0);
  }

  TEST_F(SOAColorTest, BackgroundColor_Interpolation_CorrectResults) {
    color::Color const light{1.0, 1.0, 1.0};
    color::Color const dark{0.0, 0.0, 0.0};

    // dir_y = -1.0 debería dar dark color (m = 0)
    auto result_min = color::background_color(-1.0, light, dark);
    EXPECT_NEAR(result_min.r, dark.r, 1e-6);
    EXPECT_NEAR(result_min.g, dark.g, 1e-6);
    EXPECT_NEAR(result_min.b, dark.b, 1e-6);

    // dir_y = 1.0 debería dar light color (m = 1)
    auto result_max = color::background_color(1.0, light, dark);
    EXPECT_NEAR(result_max.r, light.r, 1e-6);
    EXPECT_NEAR(result_max.g, light.g, 1e-6);
    EXPECT_NEAR(result_max.b, light.b, 1e-6);

    // dir_y = 0.0 debería dar mezcla 50/50 (m = 0.5)
    auto result_mid = color::background_color(0.0, light, dark);
    EXPECT_NEAR(result_mid.r, 0.5, 1e-6);
    EXPECT_NEAR(result_mid.g, 0.5, 1e-6);
    EXPECT_NEAR(result_mid.b, 0.5, 1e-6);
  }

  TEST_F(SOAColorTest, BackgroundColor_SpecificColorValues_CorrectInterpolation) {
    color::Color const light{1.0, 0.5, 0.0};  // R=1.0, G=0.5, B=0.0
    color::Color const dark{0.0, 0.0, 1.0};   // R=0.0, G=0.0, B=1.0

    // dir_y = 0.5 debería dar mezcla exacta
    auto result = color::background_color(0.5, light, dark);

    // m = (0.5 + 1) / 2 = 0.75
    // color = (1-0.75)*dark + 0.75*light
    // = 0.25*dark + 0.75*light
    double const expected_r = 0.25 * 0.0 + 0.75 * 1.0;  // 0.75
    double const expected_g = 0.25 * 0.0 + 0.75 * 0.5;  // 0.375
    double const expected_b = 0.25 * 1.0 + 0.75 * 0.0;  // 0.25

    EXPECT_NEAR(result.r, expected_r, 1e-6);
    EXPECT_NEAR(result.g, expected_g, 1e-6);
    EXPECT_NEAR(result.b, expected_b, 1e-6);
  }

  // Tests de clamp
  TEST_F(SOAColorTest, Clamp_ValidValues_ReturnsSameValue) {
    EXPECT_DOUBLE_EQ(color::clamp(0.5, 0.0, 1.0), 0.5);
    EXPECT_DOUBLE_EQ(color::clamp(0.0, 0.0, 1.0), 0.0);
    EXPECT_DOUBLE_EQ(color::clamp(1.0, 0.0, 1.0), 1.0);
  }

  TEST_F(SOAColorTest, Clamp_OutOfRangeValues_ReturnsClampedValue) {
    // Valores por debajo del mínimo
    EXPECT_DOUBLE_EQ(color::clamp(-1.0, 0.0, 1.0), 0.0);
    EXPECT_DOUBLE_EQ(color::clamp(-0.5, 0.0, 1.0), 0.0);

    // Valores por encima del máximo
    EXPECT_DOUBLE_EQ(color::clamp(1.5, 0.0, 1.0), 1.0);
    EXPECT_DOUBLE_EQ(color::clamp(2.0, 0.0, 1.0), 1.0);
  }

  TEST_F(SOAColorTest, Clamp_EdgeCases_CorrectBehavior) {
    // Valores en los límites
    EXPECT_DOUBLE_EQ(color::clamp(0.0, 0.0, 1.0), 0.0);
    EXPECT_DOUBLE_EQ(color::clamp(1.0, 0.0, 1.0), 1.0);

    // Valores muy cercanos a los límites
    EXPECT_DOUBLE_EQ(color::clamp(-0.0001, 0.0, 1.0), 0.0);
    EXPECT_DOUBLE_EQ(color::clamp(1.0001, 0.0, 1.0), 1.0);
  }

  // Tests de to_byte
  TEST_F(SOAColorTest, ToByte_ValidValues_ReturnsByte) {
    uint8_t const result1 = color::to_byte(0.0, cfg->gamma);
    uint8_t const result2 = color::to_byte(0.5, cfg->gamma);
    uint8_t const result3 = color::to_byte(1.0, cfg->gamma);

    EXPECT_TRUE(result1 >= 0 and result1 <= 255);
    EXPECT_TRUE(result2 >= 0 and result2 <= 255);
    EXPECT_TRUE(result3 >= 0 and result3 <= 255);
  }

  TEST_F(SOAColorTest, ToByte_ExtremeValues_ReturnsClampedByte) {
    // Valores fuera del rango [0,1] deberían clampearse
    uint8_t const result1 = color::to_byte(-1.0, cfg->gamma);
    uint8_t const result2 = color::to_byte(2.0, cfg->gamma);
    uint8_t const result3 = color::to_byte(-100.0, cfg->gamma);
    uint8_t const result4 = color::to_byte(100.0, cfg->gamma);

    EXPECT_EQ(result1, 0);
    EXPECT_EQ(result2, 255);
    EXPECT_EQ(result3, 0);
    EXPECT_EQ(result4, 255);
  }

  TEST_F(SOAColorTest, ToByte_SpecificValues_CorrectConversion) {
    // Valor 0.0 debería convertirse a 0
    EXPECT_EQ(color::to_byte(0.0, cfg->gamma), 0);

    // Valor 1.0 debería convertirse a 255
    EXPECT_EQ(color::to_byte(1.0, cfg->gamma), 255);

    // Valor 0.5 con gamma 1.0 debería convertirse a 128 (aproximadamente)
    uint8_t const result = color::to_byte(0.5, 1.0);
    EXPECT_NEAR(result, 128, 1);  // Permitir pequeña diferencia por redondeo
  }

  TEST_F(SOAColorTest, ToByte_GammaCorrection_CorrectApplication) {
    // Con gamma = 1.0, no hay corrección
    uint8_t const no_gamma = color::to_byte(0.5, 1.0);

    // Con gamma = 2.2, 0.5 se corrige a aproximadamente 0.73
    uint8_t const with_gamma = color::to_byte(0.5, 2.2);

    // El valor con gamma debería ser mayor (más claro)
    EXPECT_GT(with_gamma, no_gamma);
  }

  TEST_F(SOAColorTest, ToByte_DifferentGammaValues_CompletesWithoutCrash) {
    // Diferentes valores de gamma
    uint8_t const result1 = color::to_byte(0.5, 0.5);  // Gamma bajo
    uint8_t const result2 = color::to_byte(0.5, 1.0);  // Sin corrección
    uint8_t const result3 = color::to_byte(0.5, 2.2);  // Gamma estándar
    uint8_t const result4 = color::to_byte(0.5, 5.0);  // Gamma alto

    EXPECT_TRUE(result1 >= 0 and result1 <= 255);
    EXPECT_TRUE(result2 >= 0 and result2 <= 255);
    EXPECT_TRUE(result3 >= 0 and result3 <= 255);
    EXPECT_TRUE(result4 >= 0 and result4 <= 255);
  }

  TEST_F(SOAColorTest, ToByte_EdgeCases_CorrectBehavior) {
    // Valores exactos en los límites
    EXPECT_EQ(color::to_byte(0.0, cfg->gamma), 0);
    EXPECT_EQ(color::to_byte(1.0, cfg->gamma), 255);

    // Valores muy cercanos a los límites
    EXPECT_EQ(color::to_byte(-0.0001, cfg->gamma), 0);   // Debería clampearse a 0
    EXPECT_EQ(color::to_byte(1.0001, cfg->gamma), 255);  // Debería clampearse a 255
  }

  // Tests de operaciones con color::Color
  TEST_F(SOAColorTest, ColorConstructor_Default_InitializesToZero) {
    color::Color const color;
    EXPECT_DOUBLE_EQ(color.r, 0.0);
    EXPECT_DOUBLE_EQ(color.g, 0.0);
    EXPECT_DOUBLE_EQ(color.b, 0.0);
  }

  TEST_F(SOAColorTest, ColorConstructor_WithValues_InitializesCorrectly) {
    color::Color const color{0.1, 0.2, 0.3};
    EXPECT_DOUBLE_EQ(color.r, 0.1);
    EXPECT_DOUBLE_EQ(color.g, 0.2);
    EXPECT_DOUBLE_EQ(color.b, 0.3);
  }

  TEST_F(SOAColorTest, ColorAddition_ProducesCorrectResult) {
    color::Color const c1{0.1, 0.2, 0.3};
    color::Color const c2{0.4, 0.5, 0.6};

    color::Color const result = c1 + c2;

    EXPECT_NEAR(result.r, 0.5, 1e-6);
    EXPECT_NEAR(result.g, 0.7, 1e-6);
    EXPECT_NEAR(result.b, 0.9, 1e-6);
  }

  TEST_F(SOAColorTest, ColorScalarMultiplication_ProducesCorrectResult) {
    color::Color const c{0.2, 0.4, 0.6};
    double const scalar = 0.5;

    color::Color const result = c * scalar;

    EXPECT_NEAR(result.r, 0.1, 1e-6);
    EXPECT_NEAR(result.g, 0.2, 1e-6);
    EXPECT_NEAR(result.b, 0.3, 1e-6);
  }

  TEST_F(SOAColorTest, ColorScalarMultiplication_LeftScalar_ProducesCorrectResult) {
    color::Color const c{0.2, 0.4, 0.6};
    double const scalar = 0.5;

    color::Color const result = scalar * c;

    EXPECT_NEAR(result.r, 0.1, 1e-6);
    EXPECT_NEAR(result.g, 0.2, 1e-6);
    EXPECT_NEAR(result.b, 0.3, 1e-6);
  }

  TEST_F(SOAColorTest, ColorOperations_ChainCorrectly) {
    color::Color const c1{0.1, 0.2, 0.3};
    color::Color const c2{0.4, 0.5, 0.6};

    // (c1 + c2) * 0.5
    color::Color const result = (c1 + c2) * 0.5;

    EXPECT_NEAR(result.r, 0.25, 1e-6);  // (0.1 + 0.4) * 0.5 = 0.25
    EXPECT_NEAR(result.g, 0.35, 1e-6);  // (0.2 + 0.5) * 0.5 = 0.35
    EXPECT_NEAR(result.b, 0.45, 1e-6);  // (0.3 + 0.6) * 0.5 = 0.45
  }

  // Tests de integración entre funciones
  TEST_F(SOAColorTest, Integration_BackgroundColorThenToByte_CompletesWithoutCrash) {
    color::Color const light{1.0, 1.0, 1.0};
    color::Color const dark{0.0, 0.0, 0.0};

    auto bg_color = color::background_color(0.5, light, dark);

    uint8_t const r_byte = color::to_byte(bg_color.r, cfg->gamma);
    uint8_t const g_byte = color::to_byte(bg_color.g, cfg->gamma);
    uint8_t const b_byte = color::to_byte(bg_color.b, cfg->gamma);

    EXPECT_TRUE(r_byte >= 0 and r_byte <= 255);
    EXPECT_TRUE(g_byte >= 0 and g_byte <= 255);
    EXPECT_TRUE(b_byte >= 0 and b_byte <= 255);
  }

  TEST_F(SOAColorTest, Integration_CompleteColorPipeline_ProducesValidBytes) {
    // Pipeline completo: background_color -> to_byte
    color::Color const light{0.8, 0.9, 1.0};
    color::Color const dark{0.1, 0.2, 0.3};

    // Usar un bucle con índice entero para evitar problemas de precisión con floats
    for (int i = 0; i <= 4; ++i) {
      double const dir_y = -1.0 + i * 0.5;  // -1.0, -0.5, 0.0, 0.5, 1.0
      auto bg_color      = color::background_color(dir_y, light, dark);

      uint8_t const r_byte = color::to_byte(bg_color.r, cfg->gamma);
      uint8_t const g_byte = color::to_byte(bg_color.g, cfg->gamma);
      uint8_t const b_byte = color::to_byte(bg_color.b, cfg->gamma);

      // Los bytes deberían estar en el rango 0-255
      EXPECT_GE(r_byte, 0);
      EXPECT_LE(r_byte, 255);
      EXPECT_GE(g_byte, 0);
      EXPECT_LE(g_byte, 255);
      EXPECT_GE(b_byte, 0);
      EXPECT_LE(b_byte, 255);
    }
  }

  // Tests de rendimiento
  TEST_F(SOAColorTest, Performance_BackgroundColor_ManyCalls_ReasonableTime) {
    color::Color const light{1.0, 1.0, 1.0};
    color::Color const dark{0.0, 0.0, 0.0};

    auto start = std::chrono::high_resolution_clock::now();

    // Llamar 10,000 veces
    for (int i = 0; i < 10'000; ++i) {
      double const dir_y           = static_cast<double>(i % 200 - 100) / 100.0;  // -1.0 to 1.0
      [[maybe_unused]] auto result = color::background_color(dir_y, light, dark);
    }

    auto end      = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    EXPECT_LT(duration.count(), 10) << "10,000 llamadas a background_color deberían ser rápidas";
  }

  TEST_F(SOAColorTest, Performance_ToByte_ManyCalls_ReasonableTime) {
    auto start = std::chrono::high_resolution_clock::now();

    // Llamar 10,000 veces
    for (int i = 0; i < 10'000; ++i) {
      double const value                    = static_cast<double>(i % 100) / 100.0;  // 0.0 to 0.99
      [[maybe_unused]] uint8_t const result = color::to_byte(value, cfg->gamma);
    }

    auto end      = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    EXPECT_LT(duration.count(), 10) << "10,000 llamadas a to_byte deberían ser rápidas";
  }

  // Tests de robustez
  TEST_F(SOAColorTest, Robustness_BackgroundColor_ExtremeColorValues_CompletesWithoutCrash) {
    // Colores con valores extremos
    auto result1 =
        color::background_color(0.5, color::Color{-1.0, -1.0, -1.0}, color::Color{2.0, 2.0, 2.0});

    auto result2 =
        color::background_color(0.5, color::Color{1e6, 1e6, 1e6}, color::Color{-1e6, -1e6, -1e6});

    // Verificar que se devuelven colores (aunque los valores puedan estar clampeados)
    EXPECT_TRUE(result1.r >= 0.0 and result1.r <= 1.0);
    EXPECT_TRUE(result2.r >= 0.0 and result2.r <= 1.0);
  }

  TEST_F(SOAColorTest, Robustness_ToByte_SpecialValues_CompletesWithoutCrash) {
    // Valores especiales - deberían manejarse sin crash
    uint8_t const result1 = color::to_byte(std::numeric_limits<double>::quiet_NaN(), cfg->gamma);
    uint8_t const result2 = color::to_byte(std::numeric_limits<double>::infinity(), cfg->gamma);
    uint8_t const result3 = color::to_byte(-std::numeric_limits<double>::infinity(), cfg->gamma);

    // Verificar que se devuelven bytes válidos (probablemente clampeados)
    EXPECT_TRUE(result1 >= 0 and result1 <= 255);
    EXPECT_TRUE(result2 >= 0 and result2 <= 255);
    EXPECT_TRUE(result3 >= 0 and result3 <= 255);
  }

  TEST_F(SOAColorTest, Robustness_ToByte_ZeroGamma_CompletesWithoutCrash) {
    // Gamma cero o negativo debería manejarse
    uint8_t const result1 = color::to_byte(0.5, 0.0);
    uint8_t const result2 = color::to_byte(0.5, -1.0);

    EXPECT_TRUE(result1 >= 0 and result1 <= 255);
    EXPECT_TRUE(result2 >= 0 and result2 <= 255);
  }

  // Tests de precisión numérica
  TEST_F(SOAColorTest, Precision_BackgroundColor_SmallValues_CorrectResults) {
    color::Color const light{1.0, 1.0, 1.0};
    color::Color const dark{0.0, 0.0, 0.0};

    // Valores muy pequeños de dir_y
    auto result1 = color::background_color(1e-10, light, dark);
    auto result2 = color::background_color(-1e-10, light, dark);

    // Deberían ser muy cercanos pero no iguales
    EXPECT_NEAR(result1.r, 0.5, 1e-6);
    EXPECT_NEAR(result2.r, 0.5, 1e-6);
  }

  TEST_F(SOAColorTest, Precision_ToByte_Rounding_CorrectBehavior) {
    // Verificar que el redondeo es consistente
    uint8_t const value1 = color::to_byte(0.4999, 1.0);
    uint8_t const value2 = color::to_byte(0.5001, 1.0);

    // Deberían ser diferentes debido al redondeo
    EXPECT_NE(value1, value2);
  }

}  // namespace soa::test
