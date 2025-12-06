#ifndef AOS_RENDER_AOS_HPP
#define AOS_RENDER_AOS_HPP

#include "../../common/include/config.hpp"
#include "../../common/include/scene_parser.hpp"
#include "aos_camera.hpp"
#include "aos_image.hpp"

namespace aos {

  // Renderiza la escena en la imagen usando la cámara en formato AOS.
  // Contrato mínimo:
  // - Usa cfg para dimensiones, SPP y colores de fondo.
  // - Usa camera.generate_primary_rays para obtener rayos primarios (w*h*spp).
  // - Si hay intersección con esferas/cilindros, pinta por la normal; si no, color de fondo.
  // - Escribe el color en AOSImage con corrección gamma.
  void render_scene(ConfigParams const & cfg, SceneOutput const & scene, CameraAOS & camera,
                    AOSImage & image);

}  // namespace aos

#endif  // AOS_RENDER_AOS_HPP
