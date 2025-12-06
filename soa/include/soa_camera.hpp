#ifndef SOA_CAMERA_HPP
#define SOA_CAMERA_HPP

#include "../../common/include/config.hpp"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

// <random> not required here; URNG type is provided by caller

namespace soa {

  class CameraSOA {
  public:
    explicit CameraSOA(ConfigParams const & cfg);
    void generate_primary_rays(std::size_t w, std::size_t h, std::size_t spp);

    // Memory-light API: precompute per-frame basis and make per-pixel rays on demand
    void begin_frame(std::size_t w, std::size_t h);

    // Ray container to keep API <= 4 parameters
    struct RaySample {
      double ox, oy, oz, dx, dy, dz;
    };

    // Lightweight [0,1) sampler built directly from the URNG without distributions
    template <class URNG> static double rand01(URNG & rng) noexcept {
      // Scale integer output of URNG into [0,1). Avoids heavy <random> distributions
      // and works with any standard-compliant URNG with min()/max().
      auto const r     = static_cast<double>(rng());
      auto const minv  = static_cast<double>(URNG::min());
      auto const maxv  = static_cast<double>(URNG::max());
      auto const range = (maxv - minv) + 1.0;  // +1 to ensure the result is < 1.0 even when r==max
      double x         = (r - minv) / range;
      if (x >= 1.0) {
        x = std::nextafter(1.0, 0.0);
      }
      x = std::max(0.0, x);
      return x;
    }

    // Create a primary ray for pixel (i,j) with random subpixel jitter from rng
    template <class URNG> RaySample make_ray(std::size_t i, std::size_t j, URNG & rng) const {
      double const rx = static_cast<double>(i) + rand01(rng);
      double const ry = static_cast<double>(j) + rand01(rng);
      double const px = p0x_ + rx * dx_x_ + ry * dy_x_;
      double const py = p0y_ + rx * dx_y_ + ry * dy_y_;
      double const pz = p0z_ + rx * dx_z_ + ry * dy_z_;
      double dx = px - OX, dy = py - OY, dz = pz - OZ;
      double const len = std::sqrt(dx * dx + dy * dy + dz * dz);
      if (len > 0.0) {
        double const inv = 1.0 / len;
        dx *= inv;
        dy *= inv;
        dz *= inv;
      }
      return RaySample{OX, OY, OZ, dx, dy, dz};
    }

    std::vector<double> origins_x, origins_y, origins_z;
    std::vector<double> dirs_x, dirs_y, dirs_z;

  private:
    double OX{}, OY{}, OZ{};
    double UX{}, UY{}, UZ{};
    double VX{}, VY{}, VZ{};
    double WX{}, WY{}, WZ{};
    double DF{}, FOV{};
    int ray_seed_{};

    // Derived per-frame values (set in begin_frame)
    double p0x_{}, p0y_{}, p0z_{};
    double dx_x_{}, dx_y_{}, dx_z_{};
    double dy_x_{}, dy_y_{}, dy_z_{};
  };

}  // namespace soa

#endif
