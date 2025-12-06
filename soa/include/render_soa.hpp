#ifndef SOA_RENDER_SOA_HPP
#define SOA_RENDER_SOA_HPP

#include "../../common/include/config.hpp"
#include "../../common/include/scene_parser.hpp"
#include "soa_camera.hpp"
#include "soa_image.hpp"

namespace soa {

  // Renderiza la escena en la imagen usando la cámara en formato SOA.
  // Contrato mínimo:
  // - Usa cfg para dimensiones, SPP y colores de fondo.
  // - Usa camera.generate_primary_rays para obtener rayos primarios (w*h*spp).
  // - Si hay intersección con esferas/cilindros, pinta por la normal; si no, color de fondo.
  // - Escribe el color en SOAImage con corrección gamma.
  void render_scene(ConfigParams const & cfg, SceneOutput const & scene, CameraSOA & camera,
                    SOAImage & image);

}  // namespace soa

#endif  // SOA_RENDER_SOA_HPP
