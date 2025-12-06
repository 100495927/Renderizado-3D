#include "../include/aos_camera.hpp"
#include "../../common/include/config.hpp"
#include <cmath>
#include <cstddef>
#include <random>
#ifndef M_PI
 #define M_PI 3.14159265358979323846
#endif

namespace {

  inline void normalize_vector(double & x, double & y, double & z) {
    double const l = std::sqrt(x * x + y * y + z * z);
    if (l > 0.0) {
      double const inv = 1.0 / l;
      x *= inv;
      y *= inv;
      z *= inv;
    }
  }

}  // anonymous namespace

namespace aos {

  CameraAOS::CameraAOS(ConfigParams const & c)
      : OX(c.camera_x), OY(c.camera_y), OZ(c.camera_z), FOV(c.field_of_view * M_PI / 180.0),
        ray_seed_(c.ray_rng_seed) {
    // W = lookfrom - lookat
    double const dx = OX - c.target_x;
    double const dy = OY - c.target_y;
    double const dz = OZ - c.target_z;
    DF              = std::sqrt(dx * dx + dy * dy + dz * dz);

    // w = unit(lookfrom - lookat)
    WX = dx;
    WY = dy;
    WZ = dz;
    normalize_vector(WX, WY, WZ);

    // Normalizar north vector
    double nx              = c.north_x;
    double ny              = c.north_y;
    double nz              = c.north_z;
    double const north_mag = std::sqrt(nx * nx + ny * ny + nz * nz);
    if (north_mag > 0.0) {
      double const north_inv = 1.0 / north_mag;
      nx *= north_inv;
      ny *= north_inv;
      nz *= north_inv;
    }

    // u = unit(cross(vup, w))
    double const cx = ny * WZ - nz * WY;
    double const cy = nz * WX - nx * WZ;
    double const cz = nx * WY - ny * WX;
    UX              = cx;
    UY              = cy;
    UZ              = cz;
    normalize_vector(UX, UY, UZ);

    // v = cross(w, u)
    VX = WY * UZ - WZ * UY;
    VY = WZ * UX - WX * UZ;
    VZ = WX * UY - WY * UX;
    normalize_vector(VX, VY, VZ);
  }

  void CameraAOS::begin_frame(std::size_t w, std::size_t h) {
    if (w == 0 or h == 0) {
      p0x_ = p0y_ = p0z_ = 0.0;
      delta_x_x_ = delta_x_y_ = delta_x_z_ = 0.0;
      delta_y_x_ = delta_y_y_ = delta_y_z_ = 0.0;
      return;
    }

    // **CÁLCULO EXACTAMENTE IGUAL A SOA**
    double const hp = 2.0 * std::tan(FOV * 0.5) * DF;
    double const wp = hp * static_cast<double>(w) / static_cast<double>(h);

    double const phx = wp * UX, phy = wp * UY, phz = wp * UZ;
    double const pvx = hp * VX, pvy = hp * VY, pvz = hp * VZ;

    // **FÓRMULA EXACTAMENTE IGUAL A SOA**
    p0x_ = OX - 0.5 * phx - 0.5 * pvx - DF * WX;
    p0y_ = OY - 0.5 * phy - 0.5 * pvy - DF * WY;
    p0z_ = OZ - 0.5 * phz - 0.5 * pvz - DF * WZ;

    delta_x_x_ = phx / static_cast<double>(w);
    delta_x_y_ = phy / static_cast<double>(w);
    delta_x_z_ = phz / static_cast<double>(w);

    delta_y_x_ = pvx / static_cast<double>(h);
    delta_y_y_ = pvy / static_cast<double>(h);
    delta_y_z_ = pvz / static_cast<double>(h);
  }

  CameraAOS::RaySample CameraAOS::make_ray(std::size_t i, std::size_t j, std::mt19937 & rng) const {
    std::uniform_real_distribution<double> dis(0.0, 1.0);

    double const rx = static_cast<double>(i) + dis(rng);
    double const ry = static_cast<double>(j) + dis(rng);

    double const px = p0x_ + rx * delta_x_x_ + ry * delta_y_x_;
    double const py = p0y_ + rx * delta_x_y_ + ry * delta_y_y_;
    double const pz = p0z_ + rx * delta_x_z_ + ry * delta_y_z_;

    double dx = px - OX;
    double dy = py - OY;
    double dz = pz - OZ;

    normalize_vector(dx, dy, dz);

    return RaySample{OX, OY, OZ, dx, dy, dz};
  }

  void CameraAOS::generate_primary_rays(std::size_t width, std::size_t height, std::size_t spp) {
    if (width == 0 or height == 0 or spp == 0) {
      rays.clear();
      return;
    }

    begin_frame(width, height);

    std::size_t const total_rays = width * height * spp;
    rays.clear();
    rays.reserve(total_rays);

    std::mt19937 rng(static_cast<std::mt19937::result_type>(ray_seed_));
    std::uniform_real_distribution<double> dis(0.0, 1.0);

    for (std::size_t j = 0; j < height; ++j) {
      for (std::size_t i = 0; i < width; ++i) {
        for (std::size_t s = 0; s < spp; ++s) {
          double const rx = static_cast<double>(i) + dis(rng);
          double const ry = static_cast<double>(j) + dis(rng);

          double const px = p0x_ + rx * delta_x_x_ + ry * delta_y_x_;
          double const py = p0y_ + rx * delta_x_y_ + ry * delta_y_y_;
          double const pz = p0z_ + rx * delta_x_z_ + ry * delta_y_z_;

          double dx = px - OX;
          double dy = py - OY;
          double dz = pz - OZ;

          normalize_vector(dx, dy, dz);

          rays.emplace_back(RayStruct{OX, OY, OZ, dx, dy, dz});
        }
      }
    }
  }

}  // namespace aos
