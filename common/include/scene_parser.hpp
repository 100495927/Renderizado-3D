#ifndef SCENE_PARSER_HPP
 #define SCENE_PARSER_HPP

 #include "materials.hpp"
 #include "objects.hpp"
 #include <functional>
 #include <string>
 #include <vector>

struct SceneOutput {
  std::reference_wrapper<std::vector<MatteMaterial>> matte_materials;
  std::reference_wrapper<std::vector<MetalMaterial>> metal_materials;
  std::reference_wrapper<std::vector<RefractiveMaterial>> refractive_materials;
  std::reference_wrapper<std::vector<Sphere>> spheres;
  std::reference_wrapper<std::vector<Cylinder>> cylinders;
};

/*bool parse_scene(const std::string &filename,
                  std::vector<MatteMaterial> &matte_materials,
                  std::vector<MetalMaterial> &metal_materials,
                  std::vector<RefractiveMaterial> &refractive_materials,
                  std::vector<Sphere> &spheres,
                  std::vector<Cylinder> &cylinders);*/

bool parse_scene(std::string const & filename, SceneOutput & out);

#endif  // SCENE_PARSER_HPP

// Falta implementar comprobación que los objetos referencian materiales ya definidos
