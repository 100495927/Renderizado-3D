// utaos/test_aos_image.cpp
#include "../../aos/include/aos_image.hpp"
#include "../../common/include/config.hpp"

#include <chrono>
#include <filesystem>
#include <gtest/gtest.h>
#include <memory>

namespace aos::test {

  class AOSImageTest : public ::testing::Test {
  protected:
    void SetUp() override { setupBasicConfig(); }

    void setupBasicConfig() {
      cfg        = std::make_unique<ConfigParams>();
      cfg->gamma = 2.2;
    }

    std::unique_ptr<ConfigParams> cfg;
  };

  // Tests de construcción básica
  TEST_F(AOSImageTest, Constructor_ValidDimensions_CreatesImage) {
    EXPECT_NO_THROW(AOSImage const image(100, 50));
  }

  TEST_F(AOSImageTest, Constructor_DifferentDimensions_CreatesImage) {
    // Dimensiones pequeñas
    EXPECT_NO_THROW(AOSImage const image1(1, 1));

    // Dimensiones grandes
    EXPECT_NO_THROW(AOSImage const image2(1'920, 1'080));

    // Dimensiones cuadradas
    EXPECT_NO_THROW(AOSImage const image3(100, 100));
  }

  TEST_F(AOSImageTest, Constructor_ZeroDimensions_CreatesImage) {
    // Dimensiones cero deberían manejarse correctamente
    EXPECT_NO_THROW(AOSImage const image1(0, 0));
    EXPECT_NO_THROW(AOSImage const image2(100, 0));
    EXPECT_NO_THROW(AOSImage const image3(0, 100));
  }

  // Tests de dimensiones
  TEST_F(AOSImageTest, WidthAndHeight_ReturnCorrectValues) {
    AOSImage const image(100, 50);

    EXPECT_EQ(image.width(), 100);
    EXPECT_EQ(image.height(), 50);
  }

  TEST_F(AOSImageTest, WidthAndHeight_DifferentSizes_ReturnCorrectValues) {
    AOSImage const image1(1, 1);
    EXPECT_EQ(image1.width(), 1);
    EXPECT_EQ(image1.height(), 1);

    AOSImage const image2(800, 600);
    EXPECT_EQ(image2.width(), 800);
    EXPECT_EQ(image2.height(), 600);

    AOSImage const image3(0, 0);
    EXPECT_EQ(image3.width(), 0);
    EXPECT_EQ(image3.height(), 0);
  }

  // Tests de setPixel
  TEST_F(AOSImageTest, SetPixel_ValidCoordinates_CompletesWithoutCrash) {
    AOSImage image(100, 50);
    RGBColor const color{0.5, 0.5, 0.5};

    // Coordenadas dentro del rango
    EXPECT_NO_THROW(image.setPixel(0, 0, color, cfg->gamma));
    EXPECT_NO_THROW(image.setPixel(25, 49, color, cfg->gamma));
    EXPECT_NO_THROW(image.setPixel(99, 0, color, cfg->gamma));
  }

  TEST_F(AOSImageTest, SetPixel_EdgeCoordinates_CompletesWithoutCrash) {
    AOSImage image(100, 50);
    RGBColor const color{0.8, 0.2, 0.6};

    // Coordenadas en los bordes
    EXPECT_NO_THROW(image.setPixel(0, 0, color, cfg->gamma));    // Esquina superior izquierda
    EXPECT_NO_THROW(image.setPixel(99, 0, color, cfg->gamma));   // Esquina superior derecha
    EXPECT_NO_THROW(image.setPixel(0, 49, color, cfg->gamma));   // Esquina inferior izquierda
    EXPECT_NO_THROW(image.setPixel(99, 49, color, cfg->gamma));  // Esquina inferior derecha
  }

  TEST_F(AOSImageTest, SetPixel_DifferentColors_CompletesWithoutCrash) {
    AOSImage image(100, 50);

    // Diferentes colores
    EXPECT_NO_THROW(image.setPixel(50, 25, RGBColor{0.0, 0.0, 0.0}, cfg->gamma));  // Negro
    EXPECT_NO_THROW(image.setPixel(50, 25, RGBColor{1.0, 1.0, 1.0}, cfg->gamma));  // Blanco
    EXPECT_NO_THROW(image.setPixel(50, 25, RGBColor{1.0, 0.0, 0.0}, cfg->gamma));  // Rojo
    EXPECT_NO_THROW(image.setPixel(50, 25, RGBColor{0.0, 1.0, 0.0}, cfg->gamma));  // Verde
    EXPECT_NO_THROW(image.setPixel(50, 25, RGBColor{0.0, 0.0, 1.0}, cfg->gamma));  // Azul
    EXPECT_NO_THROW(image.setPixel(50, 25, RGBColor{0.5, 0.5, 0.5}, cfg->gamma));  // Gris
  }

  TEST_F(AOSImageTest, SetPixel_ExtremeColorValues_CompletesWithoutCrash) {
    AOSImage image(100, 50);

    // Valores de color extremos
    EXPECT_NO_THROW(image.setPixel(50, 25, RGBColor{0.0, 0.0, 0.0}, cfg->gamma));  // Mínimo
    EXPECT_NO_THROW(image.setPixel(50, 25, RGBColor{1.0, 1.0, 1.0}, cfg->gamma));  // Máximo
    EXPECT_NO_THROW(image.setPixel(50, 25, RGBColor{-0.5, -0.5, -0.5},
                                   cfg->gamma));  // Negativos (deberían clampearse)
    EXPECT_NO_THROW(image.setPixel(50, 25, RGBColor{2.0, 2.0, 2.0},
                                   cfg->gamma));  // Mayores que 1 (deberían clampearse)
  }

  TEST_F(AOSImageTest, SetPixel_DifferentGammaValues_CompletesWithoutCrash) {
    AOSImage image(100, 50);
    RGBColor const color{0.5, 0.5, 0.5};

    // Diferentes valores de gamma
    EXPECT_NO_THROW(image.setPixel(50, 25, color, 1.0));  // Sin corrección gamma
    EXPECT_NO_THROW(image.setPixel(50, 25, color, 2.2));  // Gamma estándar
    EXPECT_NO_THROW(image.setPixel(50, 25, color, 0.5));  // Gamma bajo
    EXPECT_NO_THROW(image.setPixel(50, 25, color, 5.0));  // Gamma alto
  }

  TEST_F(AOSImageTest, SetPixel_MultipleCalls_SamePixel_CompletesWithoutCrash) {
    AOSImage image(100, 50);

    // Múltiples escrituras al mismo píxel
    EXPECT_NO_THROW(image.setPixel(50, 25, RGBColor{0.1, 0.2, 0.3}, cfg->gamma));
    EXPECT_NO_THROW(image.setPixel(50, 25, RGBColor{0.4, 0.5, 0.6}, cfg->gamma));
    EXPECT_NO_THROW(image.setPixel(50, 25, RGBColor{0.7, 0.8, 0.9}, cfg->gamma));
  }

  // Tests de write_ppm
  TEST_F(AOSImageTest, WritePPM_ValidImage_CompletesWithoutCrash) {
    AOSImage image(100, 50);

    // Llenar la imagen con algunos píxeles
    for (int row = 0; row < 50; ++row) {
      for (int col = 0; col < 100; ++col) {
        RGBColor const color{static_cast<double>(col) / 100.0, static_cast<double>(row) / 50.0,
                             0.5};
        image.setPixel(row, col, color, cfg->gamma);
      }
    }

    EXPECT_NO_THROW(image.write_ppm("test_output.ppm"));

    // Limpiar el archivo de prueba
    std::filesystem::remove("test_output.ppm");
  }

  TEST_F(AOSImageTest, WritePPM_EmptyImage_CompletesWithoutCrash) {
    AOSImage const image(0, 0);
    EXPECT_NO_THROW(image.write_ppm("empty_test.ppm"));

    // Limpiar el archivo de prueba
    std::filesystem::remove("empty_test.ppm");
  }

  TEST_F(AOSImageTest, WritePPM_SmallImage_CompletesWithoutCrash) {
    AOSImage image(1, 1);
    image.setPixel(0, 0, RGBColor{1.0, 0.0, 0.0}, cfg->gamma);  // Píxel rojo

    EXPECT_NO_THROW(image.write_ppm("small_test.ppm"));

    // Limpiar el archivo de prueba
    std::filesystem::remove("small_test.ppm");
  }

  TEST_F(AOSImageTest, WritePPM_DifferentDirectories_CompletesWithoutCrash) {
    AOSImage const image(10, 10);

    // Ruta en directorio actual
    EXPECT_NO_THROW(image.write_ppm("./current_dir_test.ppm"));

    // Ruta con subdirectorio (podría fallar si no existe, pero no debería crashar)
    EXPECT_NO_THROW(image.write_ppm("test_images/subdir_test.ppm"));

    // Limpiar archivos de prueba
    std::filesystem::remove("./current_dir_test.ppm");
    // No intentamos limpiar subdirectorios que podrían no existir
  }

  TEST_F(AOSImageTest, WritePPM_SpecialCharactersInFilename_CompletesWithoutCrash) {
    AOSImage const image(10, 10);

    // Nombres de archivo con caracteres especiales
    EXPECT_NO_THROW(image.write_ppm("test_image_123.ppm"));
    EXPECT_NO_THROW(image.write_ppm("test-image.ppm"));
    EXPECT_NO_THROW(image.write_ppm("test.image.ppm"));

    // Limpiar archivos de prueba
    std::filesystem::remove("test_image_123.ppm");
    std::filesystem::remove("test-image.ppm");
    std::filesystem::remove("test.image.ppm");
  }

  // Tests de integración setPixel + write_ppm
  TEST_F(AOSImageTest, Integration_SetPixelThenWritePPM_CompletesWithoutCrash) {
    AOSImage image(50, 25);

    // Llenar la imagen con un patrón
    for (int row = 0; row < 25; ++row) {
      for (int col = 0; col < 50; ++col) {
        RGBColor const color{static_cast<double>(col) / 50.0, static_cast<double>(row) / 25.0,
                             static_cast<double>(col + row) / 75.0};
        image.setPixel(row, col, color, cfg->gamma);
      }
    }

    EXPECT_NO_THROW(image.write_ppm("integration_test.ppm"));

    // Verificar que el archivo se creó
    EXPECT_TRUE(std::filesystem::exists("integration_test.ppm"));

    // Limpiar el archivo de prueba
    std::filesystem::remove("integration_test.ppm");
  }

  TEST_F(AOSImageTest, Integration_GradientImage_CompletesWithoutCrash) {
    AOSImage image(100, 100);

    // Crear un gradiente
    for (int row = 0; row < 100; ++row) {
      for (int col = 0; col < 100; ++col) {
        RGBColor const color{
          static_cast<double>(col) / 100.0,  // R aumenta de izquierda a derecha
          static_cast<double>(row) / 100.0,  // G aumenta de arriba a abajo
          0.5                                // B constante
        };
        image.setPixel(row, col, color, cfg->gamma);
      }
    }

    EXPECT_NO_THROW(image.write_ppm("gradient_test.ppm"));

    // Limpiar el archivo de prueba
    std::filesystem::remove("gradient_test.ppm");
  }

  // Tests de rendimiento
  TEST_F(AOSImageTest, Performance_SetPixelManyTimes_ReasonableTime) {
    AOSImage image(100, 100);
    RGBColor const color{0.5, 0.5, 0.5};

    auto start = std::chrono::high_resolution_clock::now();

    // Establecer 10,000 píxeles
    for (int row = 0; row < 100; ++row) {
      for (int col = 0; col < 100; ++col) {
        image.setPixel(row, col, color, cfg->gamma);
      }
    }

    auto end      = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    EXPECT_LT(duration.count(), 100) << "Establecer 10,000 píxeles debería ser rápido";
  }

  TEST_F(AOSImageTest, Performance_WritePPM_LargeImage_ReasonableTime) {
    AOSImage image(500, 500);

    // Llenar la imagen rápidamente
    for (int row = 0; row < 500; row += 10) {
      for (int col = 0; col < 500; col += 10) {
        image.setPixel(row, col, RGBColor{0.5, 0.5, 0.5}, cfg->gamma);
      }
    }

    auto start = std::chrono::high_resolution_clock::now();

    EXPECT_NO_THROW(image.write_ppm("performance_test.ppm"));

    auto end      = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    EXPECT_LT(duration.count(), 1'000)
        << "Escribir imagen de 500x500 debería ser razonablemente rápido";

    // Limpiar el archivo de prueba
    std::filesystem::remove("performance_test.ppm");
  }

  // Tests de robustez
  TEST_F(AOSImageTest, Robustness_SetPixel_OutOfBoundsCoordinates_CompletesWithoutCrash) {
    AOSImage image(100, 50);
    RGBColor const color{0.5, 0.5, 0.5};

    // Coordenadas fuera de rango - deberían manejarse correctamente (posiblemente con asserts o
    // excepciones) Por ahora solo verificamos que no crashea el programa
    EXPECT_NO_THROW(image.setPixel(-1, 0, color, cfg->gamma));
    EXPECT_NO_THROW(image.setPixel(0, -1, color, cfg->gamma));
    EXPECT_NO_THROW(image.setPixel(100, 0, color, cfg->gamma));    // row = height (fuera de rango)
    EXPECT_NO_THROW(image.setPixel(0, 100, color, cfg->gamma));    // col = width (fuera de rango)
    EXPECT_NO_THROW(image.setPixel(100, 100, color, cfg->gamma));  // Ambos fuera de rango
  }

  TEST_F(AOSImageTest, Robustness_WritePPM_InvalidFilename_CompletesWithoutCrash) {
    AOSImage const image(10, 10);

    // Nombres de archivo potencialmente problemáticos
    EXPECT_NO_THROW(image.write_ppm(""));                        // Nombre vacío
    EXPECT_NO_THROW(image.write_ppm("/invalid/path/test.ppm"));  // Ruta no existente
  }

  TEST_F(AOSImageTest, Robustness_WritePPM_PermissionIssues_CompletesWithoutCrash) {
    AOSImage const image(10, 10);

    // Intentar escribir en ubicaciones que podrían tener problemas de permisos
    // (esto podría fallar, pero no debería crashar el programa)
    EXPECT_NO_THROW(
        image.write_ppm("/root/test.ppm"));  // Directorio root (probablemente sin permisos)
    EXPECT_NO_THROW(image.write_ppm("/sys/test.ppm"));  // Directorio de sistema
  }

  // Tests de múltiples instancias
  TEST_F(AOSImageTest, MultipleInstances_IndependentOperation) {
    AOSImage image1(100, 50);
    AOSImage image2(50, 25);

    // Ambas deberían operar independientemente
    EXPECT_NO_THROW(image1.setPixel(0, 0, RGBColor{1.0, 0.0, 0.0}, cfg->gamma));
    EXPECT_NO_THROW(image2.setPixel(0, 0, RGBColor{0.0, 1.0, 0.0}, cfg->gamma));

    EXPECT_NO_THROW(image1.write_ppm("image1_test.ppm"));
    EXPECT_NO_THROW(image2.write_ppm("image2_test.ppm"));

    // Limpiar archivos de prueba
    std::filesystem::remove("image1_test.ppm");
    std::filesystem::remove("image2_test.ppm");
  }

}  // namespace aos::test
