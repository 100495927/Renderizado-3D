#include "../include/soa_camera.hpp"
#include "../../common/include/config.hpp"
#include <cmath>
#include <cstddef>
#include <random>
#ifndef M_PI
 #define M_PI 3.14159265358979323846
#endif

namespace {

  inline void norm(double & x, double & y, double & z) {
    double const l = std::sqrt(x * x + y * y + z * z);
    if (l > 0.0) {
      double const inv = 1.0 / l;
      x *= inv;
      y *= inv;
      z *= inv;
    }
  }

  // Helper para normalizar north vector
  void normalize_north(double & nx, double & ny, double & nz) {
    double const north_mag = std::sqrt(nx * nx + ny * ny + nz * nz);
    if (north_mag > 0.0) {
      double const north_inv = 1.0 / north_mag;
      nx *= north_inv;
      ny *= north_inv;
      nz *= north_inv;
    }
  }

}  // anonymous namespace

namespace soa {

  CameraSOA::CameraSOA(ConfigParams const & c)
      : OX(c.camera_x), OY(c.camera_y), OZ(c.camera_z), FOV(c.field_of_view * M_PI / 180.0),
        ray_seed_{c.ray_rng_seed} {
    // Origen y vectores base
    double const dx = OX - c.target_x, dy = OY - c.target_y, dz = OZ - c.target_z;
    DF = std::sqrt(dx * dx + dy * dy + dz * dz);

    // w = unit(lookfrom - lookat)
    WX = dx;
    WY = dy;
    WZ = dz;
    norm(WX, WY, WZ);

    // Normalizar north vector
    double nx = c.north_x;
    double ny = c.north_y;
    double nz = c.north_z;
    normalize_north(nx, ny, nz);

    // u = unit(cross(vup, w))
    double const cx = ny * WZ - nz * WY;  // *CAMBIO*
    double const cy = nz * WX - nx * WZ;  // *CAMBIO*
    double const cz = nx * WY - ny * WX;
    UX              = cx;
    UY              = cy;
    UZ              = cz;
    norm(UX, UY, UZ);

    // v = cross(w, u)
    VX = UY * WZ - UZ * WY;
    VY = UZ * WX - UX * WZ;
    VZ = UX * WY - UY * WX;
    norm(VX, VY, VZ);
  }

  void CameraSOA::begin_frame(std::size_t w, std::size_t h) {
    if (w == 0 or h == 0) {
      // Reset derived values
      p0x_ = p0y_ = p0z_ = 0.0;
      dx_x_ = dx_y_ = dx_z_ = 0.0;
      dy_x_ = dy_y_ = dy_z_ = 0.0;
      return;
    }
    double const hp  = 2.0 * std::tan(FOV * 0.5) * DF;
    double const wp  = hp * static_cast<double>(w) / static_cast<double>(h);
    double const phx = wp * UX, phy = wp * UY, phz = wp * UZ;
    double const pvx = hp * VX, pvy = hp * VY, pvz = hp * VZ;
    p0x_  = OX - 0.5 * phx - 0.5 * pvx - DF * WX;
    p0y_  = OY - 0.5 * phy - 0.5 * pvy - DF * WY;
    p0z_  = OZ - 0.5 * phz - 0.5 * pvz - DF * WZ;
    dx_x_ = phx / static_cast<double>(w);
    dx_y_ = phy / static_cast<double>(w);
    dx_z_ = phz / static_cast<double>(w);
    dy_x_ = pvx / static_cast<double>(h);
    dy_y_ = pvy / static_cast<double>(h);
    dy_z_ = pvz / static_cast<double>(h);
  }

  void CameraSOA::generate_primary_rays(std::size_t w, std::size_t h, std::size_t spp) {
    if (w == 0 or h == 0 or spp == 0) {
      origins_x.clear(), origins_y.clear(), origins_z.clear(), dirs_x.clear(), dirs_y.clear(),
          dirs_z.clear();
      return;
    }
    double const hp  = 2.0 * std::tan(FOV * 0.5) * DF;
    double const wp  = hp * static_cast<double>(w) / static_cast<double>(h);
    double const phx = wp * UX, phy = wp * UY, phz = wp * UZ;
    double const pvx = hp * VX, pvy = hp * VY, pvz = hp * VZ;
    double const p0x  = OX - 0.5 * phx - 0.5 * pvx - DF * WX;
    double const p0y  = OY - 0.5 * phy - 0.5 * pvy - DF * WY;
    double const p0z  = OZ - 0.5 * phz - 0.5 * pvz - DF * WZ;
    double const dx_x = phx / double(w), dx_y = phy / double(w), dx_z = phz / double(w);
    double const dy_x = pvx / double(h), dy_y = pvy / double(h), dy_z = pvz / double(h);
    std::size_t const total = w * h * spp;
    origins_x.assign(total, OX);
    origins_y.assign(total, OY);
    origins_z.assign(total, OZ);
    dirs_x.resize(total);
    dirs_y.resize(total);
    dirs_z.resize(total);
    std::mt19937 rng(static_cast<std::mt19937::result_type>(ray_seed_));
    std::uniform_real_distribution<double> dis(0.0, 1.0);
    for (std::size_t j = 0; j < h; ++j) {
      for (std::size_t i = 0; i < w; ++i) {
        std::size_t const base = (j * w + i) * spp;
        for (std::size_t s = 0; s < spp; ++s) {
          double const rx = double(i) + dis(rng), ry = double(j) + dis(rng);
          double const px = p0x + rx * dx_x + ry * dy_x;
          double const py = p0y + rx * dx_y + ry * dy_y;
          double const pz = p0z + rx * dx_z + ry * dy_z;
          double dx = px - OX, dy = py - OY, dz = pz - OZ;
          norm(dx, dy, dz);
          dirs_x[base + s] = dx;
          dirs_y[base + s] = dy;
          dirs_z[base + s] = dz;
        }
      }
    }
  }

}  // namespace soa
