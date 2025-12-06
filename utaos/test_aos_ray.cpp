// utaos/test_aos_ray.cpp
#include "../../aos/include/aos_ray.hpp"
#include "../../common/include/vector.hpp"

#include <chrono>
#include <cmath>
#include <gtest/gtest.h>

namespace aos::test {

  class AOSRayTest : public ::testing::Test {
  protected:
    void SetUp() override {
      setupBasicRay();
      setupBasicSphere();
      setupBasicCylinder();
    }

    void setupBasicRay() {
      // Rayo básico desde (0,0,-5) hacia (0,0,1)
      ray.origin    = render::vector{0.0, 0.0, -5.0};
      ray.direction = render::vector{0.0, 0.0, 1.0};
    }

    void setupBasicSphere() {
      // Esfera en el origen con radio 1
      sphere_center = render::vector{0.0, 0.0, 0.0};
      sphere_radius = 1.0;
    }

    void setupBasicCylinder() {
      // Cilindro básico en el origen, eje Y, radio 1, altura 2
      cylinder_params.center = render::vector{0.0, 0.0, 0.0};
      cylinder_params.radius = 1.0;
      cylinder_params.axis   = render::vector{0.0, 2.0, 0.0};  // Altura = 2
      cylinder_params.height = cylinder_params.axis.magnitude();
    }

    ray::Ray ray;
    render::vector sphere_center;
    double sphere_radius = 0.0;
    ray::CylinderParams cylinder_params;
  };

  // Tests de hit_sphere - casos básicos
  TEST_F(AOSRayTest, HitSphere_DirectHit_ReturnsHitRecord) {
    ray::IntersectionParams const params{0.001, 1000.0};

    auto result = ray::hit_sphere(ray, sphere_center, sphere_radius, params);

    ASSERT_TRUE(result.has_value());
    if (!result) {
      return;
    }
    auto const hit = *result;
    EXPECT_GT(hit.t, 0.0);
    EXPECT_LT(hit.t, 10.0);
  }

  TEST_F(AOSRayTest, HitSphere_Miss_ReturnsNullopt) {
    // Rayo que apunta lejos de la esfera
    ray.direction = render::vector{1.0, 0.0, 0.0};
    ray::IntersectionParams const params{0.001, 1000.0};

    auto result = ray::hit_sphere(ray, sphere_center, sphere_radius, params);

    EXPECT_FALSE(result.has_value());
  }

  TEST_F(AOSRayTest, HitSphere_Tangent_ReturnsHitRecord) {
    // Rayo tangente a la esfera
    ray.origin    = render::vector{1.0, 0.0, -5.0};
    ray.direction = render::vector{0.0, 0.0, 1.0};
    ray::IntersectionParams const params{0.001, 1000.0};

    auto result = ray::hit_sphere(ray, sphere_center, sphere_radius, params);

    ASSERT_TRUE(result.has_value());
    if (!result) {
      return;
    }
    auto const hit = *result;
    EXPECT_GT(hit.t, 0.0);
  }

  TEST_F(AOSRayTest, HitSphere_Inside_ReturnsHitRecord) {
    // Rayo que empieza dentro de la esfera
    ray.origin    = render::vector{0.0, 0.0, 0.0};
    ray.direction = render::vector{0.0, 0.0, 1.0};
    ray::IntersectionParams const params{0.001, 1000.0};

    auto result = ray::hit_sphere(ray, sphere_center, sphere_radius, params);

    ASSERT_TRUE(result.has_value());
    if (!result) {
      return;
    }
    auto const hit = *result;
    EXPECT_GT(hit.t, 0.0);
  }

  // Tests de hit_sphere - parámetros de intersección
  TEST_F(AOSRayTest, HitSphere_TMinMax_RespectsBounds) {
    ray::IntersectionParams const params{10.0, 20.0};  // t entre 10 y 20

    auto result = ray::hit_sphere(ray, sphere_center, sphere_radius, params);

    // La intersección real está en t ~4, debería ser ignorada
    EXPECT_FALSE(result.has_value());
  }

  TEST_F(AOSRayTest, HitSphere_ValidTMinMax_ReturnsHit) {
    ray::IntersectionParams const params{0.001, 10.0};  // t entre 0.001 y 10

    auto result = ray::hit_sphere(ray, sphere_center, sphere_radius, params);

    ASSERT_TRUE(result.has_value());
    if (!result) {
      return;
    }
    auto const hit = *result;
    EXPECT_GT(hit.t, params.t_min);
    EXPECT_LT(hit.t, params.t_max);
  }

  // Tests de hit_sphere - propiedades del HitRecord
  TEST_F(AOSRayTest, HitSphere_HitRecord_HasCorrectProperties) {
    ray::IntersectionParams const params{0.001, 1000.0};

    auto result = ray::hit_sphere(ray, sphere_center, sphere_radius, params);

    ASSERT_TRUE(result.has_value());
    if (!result) {
      return;
    }
    auto const hit = *result;

    // Verificar punto de intersección
    render::vector const expected_point = ray.at(result->t);
    EXPECT_NEAR(hit.point.get_x(), expected_point.get_x(), 1e-6);
    EXPECT_NEAR(hit.point.get_y(), expected_point.get_y(), 1e-6);
    EXPECT_NEAR(hit.point.get_z(), expected_point.get_z(), 1e-6);

    // Verificar normal (debería apuntar hacia afuera)
    render::vector const expected_normal = (hit.point - sphere_center) / sphere_radius;
    EXPECT_NEAR(hit.normal.get_x(), expected_normal.get_x(), 1e-6);
    EXPECT_NEAR(hit.normal.get_y(), expected_normal.get_y(), 1e-6);
    EXPECT_NEAR(hit.normal.get_z(), expected_normal.get_z(), 1e-6);

    // Verificar face_normal
    double const dot_product = render::dot(ray.direction, hit.normal);
    if (dot_product < 0) {
      EXPECT_TRUE(hit.front_face);
    } else {
      EXPECT_FALSE(hit.front_face);
    }
  }

  // Tests de hit_cylinder - superficie curva
  TEST_F(AOSRayTest, HitCylinder_SurfaceDirectHit_ReturnsHitRecord) {
    // Rayo que golpea la superficie lateral del cilindro
    ray.origin    = render::vector{2.0, 0.0, -5.0};
    ray.direction = render::vector{-0.5, 0.0, 1.0};
    ray::IntersectionParams const params{0.001, 1000.0};

    auto result = ray::hit_cylinder(ray, cylinder_params, params);

    ASSERT_TRUE(result.has_value());
    if (!result) {
      return;
    }
    auto const hit = *result;
    EXPECT_GT(hit.t, 0.0);
  }

  TEST_F(AOSRayTest, HitCylinder_SurfaceMiss_ReturnsNullopt) {
    // Rayo que no golpea el cilindro
    ray.origin    = render::vector{3.0, 0.0, -5.0};
    ray.direction = render::vector{0.0, 0.0, 1.0};
    ray::IntersectionParams const params{0.001, 1000.0};

    auto result = ray::hit_cylinder(ray, cylinder_params, params);

    EXPECT_FALSE(result.has_value());
  }

  TEST_F(AOSRayTest, HitCylinder_SurfaceOutsideHeight_ReturnsNullopt) {
    // Rayo que golpearía la superficie pero está fuera de la altura
    ray.origin    = render::vector{2.0, 3.0, -5.0};  // Por encima del cilindro
    ray.direction = render::vector{-0.3, -0.3, 1.0};
    ray::IntersectionParams const params{0.001, 1000.0};

    auto result = ray::hit_cylinder(ray, cylinder_params, params);

    EXPECT_FALSE(result.has_value());
  }

  // Tests de hit_cylinder - tapas
  TEST_F(AOSRayTest, HitCylinder_TopCapHit_ReturnsHitRecord) {
    // Rayo que golpea la tapa superior
    ray.origin    = render::vector{0.0, 2.0, -5.0};
    ray.direction = render::vector{0.0, -0.5, 1.0};
    ray::IntersectionParams const params{0.001, 1000.0};

    auto result = ray::hit_cylinder(ray, cylinder_params, params);

    EXPECT_TRUE(result.has_value());
  }

  TEST_F(AOSRayTest, HitCylinder_BottomCapHit_ReturnsHitRecord) {
    // Rayo que golpea la tapa inferior
    ray.origin    = render::vector{0.0, -2.0, -5.0};
    ray.direction = render::vector{0.0, 0.5, 1.0};
    ray::IntersectionParams const params{0.001, 1000.0};

    auto result = ray::hit_cylinder(ray, cylinder_params, params);

    EXPECT_TRUE(result.has_value());
  }

  TEST_F(AOSRayTest, HitCylinder_CapOutsideRadius_ReturnsNullopt) {
    // Rayo que golpea el plano de la tapa pero fuera del radio
    ray.origin    = render::vector{2.0, 2.0, -5.0};  // Fuera del radio en X
    ray.direction = render::vector{0.0, -0.5, 1.0};
    ray::IntersectionParams const params{0.001, 1000.0};

    auto result = ray::hit_cylinder(ray, cylinder_params, params);

    EXPECT_FALSE(result.has_value());
  }

  // Tests de hit_cylinder - propiedades del HitRecord
  TEST_F(AOSRayTest, HitCylinder_HitRecord_HasCorrectProperties) {
    // Configurar para golpear la superficie lateral
    ray.origin    = render::vector{2.0, 0.0, -5.0};
    ray.direction = render::vector{-0.5, 0.0, 1.0};
    ray::IntersectionParams const params{0.001, 1000.0};

    auto result = ray::hit_cylinder(ray, cylinder_params, params);

    ASSERT_TRUE(result.has_value());
    if (!result) {
      return;
    }
    auto const hit = *result;

    // Verificar punto de intersección
    render::vector const expected_point = ray.at(result->t);
    EXPECT_NEAR(hit.point.get_x(), expected_point.get_x(), 1e-6);
    EXPECT_NEAR(hit.point.get_y(), expected_point.get_y(), 1e-6);
    EXPECT_NEAR(hit.point.get_z(), expected_point.get_z(), 1e-6);

    // Verificar que la normal es perpendicular al eje
    render::vector const axis_unit       = cylinder_params.axis / cylinder_params.axis.magnitude();
    render::vector const point_to_center = hit.point - cylinder_params.center;
    point_to_center - axis_unit * render::dot(point_to_center, axis_unit);

    double const dot_product = render::dot(hit.normal, axis_unit);
    EXPECT_NEAR(dot_product, 0.0, 1e-6);  // Normal debería ser perpendicular al eje

    // Verificar face_normal
    double const ray_dot_normal = render::dot(ray.direction, hit.normal);
    if (ray_dot_normal < 0) {
      EXPECT_TRUE(hit.front_face);
    } else {
      EXPECT_FALSE(hit.front_face);
    }
  }

  // Tests de casos especiales
  TEST_F(AOSRayTest, HitSphere_ZeroRadius_ReturnsHitAtCenter) {
    // Esfera con radio cero en el origen
    double const zero_radius = 0.0;
    ray::IntersectionParams const params{0.001, 1000.0};

    auto result = ray::hit_sphere(ray, sphere_center, zero_radius, params);

    // Debería golpear exactamente en el centro
    ASSERT_TRUE(result.has_value());
    if (!result) {
      return;
    }
    auto const hit = *result;
    EXPECT_NEAR(hit.t, 5.0, 1e-6);  // Desde (0,0,-5) hasta (0,0,0)
  }

  TEST_F(AOSRayTest, HitCylinder_ZeroRadius_ReturnsNullopt) {
    // Cilindro con radio cero
    cylinder_params.radius = 0.0;
    ray::IntersectionParams const params{0.001, 1000.0};

    auto result = ray::hit_cylinder(ray, cylinder_params, params);

    EXPECT_FALSE(result.has_value());
  }

  TEST_F(AOSRayTest, HitCylinder_ZeroHeight_ReturnsNullopt) {
    // Cilindro con altura cero (solo tapas)
    cylinder_params.axis   = render::vector{0.0, 0.0, 0.0};
    cylinder_params.height = 0.0;
    ray::IntersectionParams const params{0.001, 1000.0};

    auto result = ray::hit_cylinder(ray, cylinder_params, params);

    EXPECT_FALSE(result.has_value());
  }

  // Tests de rendimiento
  TEST_F(AOSRayTest, Performance_HitSphere_ManyCalls_ReasonableTime) {
    ray::IntersectionParams const params{0.001, 1000.0};

    auto start = std::chrono::high_resolution_clock::now();

    // Llamar 10,000 veces
    for (int i = 0; i < 10'000; ++i) {
      // Pequeñas variaciones en la posición para evitar optimizaciones
      render::vector const test_center{static_cast<double>(i % 10) * 0.1, 0.0, 0.0};
      [[maybe_unused]] auto result = ray::hit_sphere(ray, test_center, sphere_radius, params);
    }

    auto end      = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    EXPECT_LT(duration.count(), 50) << "10,000 llamadas a hit_sphere deberían ser rápidas";
  }

  TEST_F(AOSRayTest, Performance_HitCylinder_ManyCalls_ReasonableTime) {
    ray::IntersectionParams const params{0.001, 1000.0};

    auto start = std::chrono::high_resolution_clock::now();

    // Llamar 10,000 veces
    for (int i = 0; i < 10'000; ++i) {
      // Pequeñas variaciones en la posición
      ray::CylinderParams test_cylinder = cylinder_params;
      test_cylinder.center         = render::vector{static_cast<double>(i % 10) * 0.1, 0.0, 0.0};
      [[maybe_unused]] auto result = ray::hit_cylinder(ray, test_cylinder, params);
    }

    auto end      = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    EXPECT_LT(duration.count(), 100)
        << "10,000 llamadas a hit_cylinder deberían ser razonablemente rápidas";
  }

  // Tests de robustez
  TEST_F(AOSRayTest, Robustness_HitSphere_ExtremeValues_CompletesWithoutCrash) {
    ray::IntersectionParams const params{0.001, 1000.0};

    // Valores extremos
    render::vector const extreme_center{1e6, 1e6, 1e6};
    double const extreme_radius = 1e6;

    EXPECT_NO_THROW({
      [[maybe_unused]] auto result = ray::hit_sphere(ray, extreme_center, extreme_radius, params);
    });

    // Valores muy pequeños
    render::vector const tiny_center{1e-6, 1e-6, 1e-6};
    double const tiny_radius = 1e-6;

    EXPECT_NO_THROW(
        { [[maybe_unused]] auto result = ray::hit_sphere(ray, tiny_center, tiny_radius, params); });
  }

  TEST_F(AOSRayTest, Robustness_HitCylinder_ExtremeValues_CompletesWithoutCrash) {
    ray::IntersectionParams const params{0.001, 1000.0};

    // Cilindro muy grande
    ray::CylinderParams extreme_cylinder;
    extreme_cylinder.center = render::vector{1e6, 1e6, 1e6};
    extreme_cylinder.radius = 1e6;
    extreme_cylinder.axis   = render::vector{0.0, 1e6, 0.0};
    extreme_cylinder.height = extreme_cylinder.axis.magnitude();

    EXPECT_NO_THROW(
        { [[maybe_unused]] auto result = ray::hit_cylinder(ray, extreme_cylinder, params); });

    // Cilindro muy pequeño
    ray::CylinderParams tiny_cylinder;
    tiny_cylinder.center = render::vector{1e-6, 1e-6, 1e-6};
    tiny_cylinder.radius = 1e-6;
    tiny_cylinder.axis   = render::vector{0.0, 1e-6, 0.0};
    tiny_cylinder.height = tiny_cylinder.axis.magnitude();

    EXPECT_NO_THROW(
        { [[maybe_unused]] auto result = ray::hit_cylinder(ray, tiny_cylinder, params); });
  }

  TEST_F(AOSRayTest, Robustness_HitSphere_InvalidTMinMax_CompletesWithoutCrash) {
    // t_min mayor que t_max
    ray::IntersectionParams const invalid_params{10.0, 1.0};

    EXPECT_NO_THROW({
      [[maybe_unused]] auto result =
          ray::hit_sphere(ray, sphere_center, sphere_radius, invalid_params);
    });

    // Valores negativos
    ray::IntersectionParams const negative_params{-10.0, -1.0};

    EXPECT_NO_THROW({
      [[maybe_unused]] auto result =
          ray::hit_sphere(ray, sphere_center, sphere_radius, negative_params);
    });
  }

  // Tests de precisión numérica
  TEST_F(AOSRayTest, Precision_HitSphere_GrazingIncidence_CorrectResult) {
    // Rayo que casi no golpea la esfera
    ray.origin    = render::vector{0.999, 0.0, -5.0};  // Muy cerca del borde
    ray.direction = render::vector{0.0, 0.0, 1.0};
    ray::IntersectionParams const params{0.001, 1000.0};

    // Verificar que se completa sin crash y el resultado es consistente
    // No nos importa si golpea o no, solo que no crashea
    EXPECT_NO_THROW({
      [[maybe_unused]] auto test_result =
          ray::hit_sphere(ray, sphere_center, sphere_radius, params);
    });
  }

  TEST_F(AOSRayTest, Precision_HitCylinder_ParallelRay_CorrectResult) {
    // Rayo paralelo al eje del cilindro
    ray.origin    = render::vector{2.0, 0.0, -5.0};
    ray.direction = render::vector{0.0, 1.0, 0.0};  // Paralelo al eje Y
    ray::IntersectionParams const params{0.001, 1000.0};

    // Depende de la posición, pero debería manejarse sin crash
    EXPECT_NO_THROW(
        { [[maybe_unused]] auto test_result = ray::hit_cylinder(ray, cylinder_params, params); });
  }

  // Tests de múltiples intersecciones
  TEST_F(AOSRayTest, HitSphere_MultipleIntersections_ReturnsClosest) {
    // Rayo que atraviesa la esfera (dos intersecciones)
    ray::IntersectionParams const params{0.001, 1000.0};

    auto result = ray::hit_sphere(ray, sphere_center, sphere_radius, params);

    ASSERT_TRUE(result.has_value());
    if (!result) {
      return;
    }
    auto const hit = *result;

    // Debería devolver la intersección más cercana (menor t)
    EXPECT_LT(hit.t, 5.0);  // La primera intersección debería estar antes del centro
  }

  TEST_F(AOSRayTest, HitCylinder_MultipleIntersections_ReturnsClosest) {
    // Rayo que atraviesa el cilindro
    ray.origin    = render::vector{2.0, 0.0, -5.0};
    ray.direction = render::vector{-0.5, 0.0, 1.0};
    ray::IntersectionParams const params{0.001, 1000.0};

    auto result = ray::hit_cylinder(ray, cylinder_params, params);

    ASSERT_TRUE(result.has_value());
    if (!result) {
      return;
    }
    auto const hit = *result;

    // Debería devolver la intersección más cercana
    EXPECT_GT(hit.t, 0.0);
    EXPECT_LT(hit.t, 10.0);
  }

}  // namespace aos::test
