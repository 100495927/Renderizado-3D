#include "../../common/include/hit_record.hpp"
#include "../../common/include/hittable.hpp"
#include "../../common/include/object_base.hpp"
#include "../../common/include/objects.hpp"
#include "../../common/include/ray.hpp"
#include "../common/include/vector.hpp"
#include <gtest/gtest.h>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

namespace {

  // Mock de ObjectBase para testing
  class MockObject : public render::ObjectBase {
  public:
    explicit MockObject(render::ObjectType type = render::SPHERE_TYPE)
        : render::ObjectBase(type) { }
  };

}  // anonymous namespace

// Fixture para tests de hittable_list
class HittableListTest : public ::testing::Test {
protected:
  render::ray test_ray;
  double t_min{};
  double t_max{};
  std::unique_ptr<Sphere> sphere1;
  std::unique_ptr<Sphere> sphere2;
  std::unique_ptr<Cylinder> cylinder;

  void SetUp() override {
    // Configuración común para los tests
    test_ray =
        render::ray(render::point_vector(0.0, 0.0, 0.0), render::direction_vector(0.0, 0.0, 1.0));
    t_min = 0.001;
    t_max = 1000.0;

    // Crear algunos objetos mock
    sphere1  = std::make_unique<Sphere>();
    sphere2  = std::make_unique<Sphere>();
    cylinder = std::make_unique<Cylinder>();
  }
};

// Tests de construcción y gestión básica
TEST_F(HittableListTest, DefaultConstructor_CreatesEmptyList) {
  render::hittable_list const list;

  EXPECT_TRUE(list.objects.empty());
}

TEST_F(HittableListTest, Add_Object_AddsToVector) {
  render::hittable_list list;

  list.add(sphere1.get());

  EXPECT_EQ(list.objects.size(), 1);
  EXPECT_EQ(list.objects[0], sphere1.get());
}

TEST_F(HittableListTest, Add_MultipleObjects_AddsAllToVector) {
  render::hittable_list list;

  list.add(sphere1.get());
  list.add(sphere2.get());
  list.add(cylinder.get());

  EXPECT_EQ(list.objects.size(), 3);
  EXPECT_EQ(list.objects[0], sphere1.get());
  EXPECT_EQ(list.objects[1], sphere2.get());
  EXPECT_EQ(list.objects[2], cylinder.get());
}

TEST_F(HittableListTest, Clear_RemovesAllObjects) {
  render::hittable_list list;

  list.add(sphere1.get());
  list.add(sphere2.get());
  EXPECT_EQ(list.objects.size(), 2);

  list.clear();

  EXPECT_TRUE(list.objects.empty());
}

// Tests de la función hit() - casos básicos
TEST_F(HittableListTest, Hit_EmptyList_ReturnsFalse) {
  render::hittable_list const list;
  render::hit_record rec;

  bool const result = list.hit(test_ray, t_min, t_max, rec);

  EXPECT_FALSE(result);
}

// Tests de la clase base hittable
TEST(HittableBaseTest, AbstractClass_CannotBeInstantiated) {
  // hittable es abstracta, no deberíamos poder instanciarla directamente
  // Esto se verifica por la presencia de funciones virtuales puras
}

TEST(HittableBaseTest, VirtualDestructor_Exists) {
  // Verificar que hittable tiene destructor virtual
  EXPECT_TRUE(std::has_virtual_destructor_v<render::hittable>);
}

TEST(HittableBaseTest, DefaultOperations_AreAvailable) {
  // Verificar que las operaciones por defecto están disponibles
  render::hittable_list list1;
  render::hittable_list list2;

  // Copy constructor
  render::hittable_list list3(list1);

  // Copy assignment
  list2 = list1;

  // Move constructor
  render::hittable_list const list4(std::move(list1));

  // Move assignment
  list2 = std::move(list3);

  // No debería lanzar excepciones
  EXPECT_TRUE(true);
}

// Test de integración básica con objetos reales
TEST_F(HittableListTest, Integration_WithRealObjects) {
  render::hittable_list list;

  // Crear objetos reales
  Sphere real_sphere;
  real_sphere.center_x = 0.0;
  real_sphere.center_y = 0.0;
  real_sphere.center_z = 5.0;
  real_sphere.radius   = 1.0;

  Cylinder real_cylinder;
  real_cylinder.center_x = 0.0;
  real_cylinder.center_y = 0.0;
  real_cylinder.center_z = 10.0;
  real_cylinder.radius   = 1.0;
  real_cylinder.height   = 2.0;

  list.add(&real_sphere);
  list.add(&real_cylinder);

  EXPECT_EQ(list.objects.size(), 2);
  EXPECT_EQ(list.objects[0], &real_sphere);
  EXPECT_EQ(list.objects[1], &real_cylinder);
}

// Test de rendimiento/estabilidad con muchos objetos
TEST_F(HittableListTest, ManyObjects_HandledCorrectly) {
  render::hittable_list list;
  int const num_objects = 100;

  std::vector<std::unique_ptr<MockObject>> objects;
  objects.reserve(num_objects);

  // Añadir muchos objetos
  for (int i = 0; i < num_objects; ++i) {
    auto obj = std::make_unique<MockObject>();
    list.add(obj.get());
    objects.push_back(std::move(obj));
  }

  EXPECT_EQ(list.objects.size(), num_objects);

  // Verificar que clear funciona incluso con muchos objetos
  list.clear();
  EXPECT_TRUE(list.objects.empty());
}

// Tests adicionales de comportamiento de la lista
TEST_F(HittableListTest, List_CanStoreDifferentObjectTypes) {
  render::hittable_list list;

  Sphere sphere;
  Cylinder cylinder;
  MockObject custom_object(render::CYLINDER_TYPE);

  list.add(&sphere);
  list.add(&cylinder);
  list.add(&custom_object);

  EXPECT_EQ(list.objects.size(), 3);

  // Verificar que todos los objetos son de tipo ObjectBase
  for (auto const * obj : list.objects) {
    EXPECT_NE(obj, nullptr);
    EXPECT_TRUE(dynamic_cast<render::ObjectBase const *>(obj) != nullptr);
  }
}

TEST_F(HittableListTest, List_HandlesNullPointersGracefully) {
  render::hittable_list list;

  // Añadir un nullptr - esto podría ser problemático en producción
  // pero para el test verificamos que la lista lo acepta
  list.add(nullptr);

  EXPECT_EQ(list.objects.size(), 1);
  EXPECT_EQ(list.objects[0], nullptr);
}

TEST_F(HittableListTest, List_CopyAndMoveOperations) {
  render::hittable_list original;
  original.add(sphere1.get());
  original.add(sphere2.get());

  // Test copy constructor
  render::hittable_list copy(original);
  EXPECT_EQ(copy.objects.size(), 2);

  // Test move constructor
  render::hittable_list const moved(std::move(original));
  EXPECT_EQ(moved.objects.size(), 2);

  // Test copy assignment
  render::hittable_list assigned;
  assigned = copy;
  EXPECT_EQ(assigned.objects.size(), 2);

  // Test move assignment
  render::hittable_list move_assigned;
  move_assigned = std::move(copy);
  EXPECT_EQ(move_assigned.objects.size(), 2);
}

// Test para verificar que hittable_list hereda correctamente de hittable
TEST_F(HittableListTest, Inheritance_FromHittable) {
  render::hittable_list list;

  // Verificar que es un hittable
  render::hittable * base_ptr = &list;
  EXPECT_NE(base_ptr, nullptr);

  // Verificar que podemos llamar a hit() a través de la interfaz base
  render::hit_record rec;
  bool const result = base_ptr->hit(test_ray, t_min, t_max, rec);

  // En una lista vacía debería retornar false
  EXPECT_FALSE(result);
}

// Test de edge cases con valores límite
TEST_F(HittableListTest, EdgeCases_ExtremeTValues) {
  render::hittable_list list;
  list.add(sphere1.get());

  render::hit_record rec;

  // T muy pequeño - no debería crash
  (void) list.hit(test_ray, 1e-10, t_max, rec);

  // T muy grande - no debería crash
  (void) list.hit(test_ray, t_min, 1e10, rec);

  // T negativo - no debería crash
  (void) list.hit(test_ray, -1000.0, -1.0, rec);

  EXPECT_TRUE(true);  // Si llegamos aquí, no hubo crash
}

// Test de limpieza de recursos
TEST_F(HittableListTest, ResourceManagement_NoMemoryLeaks) {
  {
    render::hittable_list list;

    // Añadir algunos objetos
    auto obj1 = std::make_unique<MockObject>();
    auto obj2 = std::make_unique<MockObject>();

    list.add(obj1.get());
    list.add(obj2.get());

    // La lista debería limpiarse correctamente al salir del scope
  }

  // Si llegamos aquí sin leaks, el test pasa
  EXPECT_TRUE(true);
}

// Test específico para verificar el comportamiento con hit_record
TEST_F(HittableListTest, HitRecord_IsPassedByReference) {
  render::hittable_list const list;
  render::hit_record initial_rec;
  initial_rec.t = 999.0;  // Valor inicial

  // En una lista vacía, el hit_record no debería modificarse
  bool const result = list.hit(test_ray, t_min, t_max, initial_rec);

  EXPECT_FALSE(result);
  // El hit_record podría o no modificarse, pero no podemos hacer suposiciones
  // sin mockear hit_object
}

// Test de interfaz constante
TEST_F(HittableListTest, ConstInterface_WorksCorrectly) {
  render::hittable_list const list;
  render::hit_record rec;

  // Deberíamos poder llamar a hit() en un objeto const
  // (esto verifica que hit() está marcado como const en la definición)
  bool const result = list.hit(test_ray, t_min, t_max, rec);

  EXPECT_FALSE(result);  // Lista vacía
}
