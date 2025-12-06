#include "../include/config_parser.hpp"
#include "../include/config.hpp"
#include "../include/parser_utilities.hpp"

#include <cstddef>
#include <exception>
#include <fstream>
#include <functional>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

namespace {

  // Lightweight parsing helpers (internal linkage)
  bool try_parse_int(std::vector<std::string> const & tokens, size_t idx, int & out) {
    if (idx >= tokens.size()) {
      return false;
    }
    try {
      out = std::stoi(tokens[idx]);
      return true;
    } catch (...) {
      return false;
    }
  }

  bool try_parse_double(std::vector<std::string> const & tokens, size_t idx, double & out) {
    if (idx >= tokens.size()) {
      return false;
    }
    try {
      out = std::stod(tokens[idx]);
      return true;
    } catch (...) {
      return false;
    }
  }

  bool check_excess_tokens(std::vector<std::string> const & tokens, size_t expected_size,
                           std::string const & key) {
    if (tokens.size() <= expected_size) {
      return true;
    }
    std::string extra;
    for (size_t i = expected_size; i < tokens.size(); ++i) {
      if (!extra.empty()) {
        extra += ' ';
      }
      extra += tokens[i];
    }
    std::cerr << "Extra data after configuration value for key " << key << "\nExtra: " << extra
              << '\n';
    return false;
  }

  using Handler =
      std::function<bool(ConfigParams &, std::vector<std::string> const &, std::string const &)>;

  // Individual handlers moved out to reduce cognitive complexity of get_handlers()
  bool handle_aspect_ratio(ConfigParams & config, std::vector<std::string> const & tokens,
                           std::string const & line) {
    if (tokens.size() < 3) {
      std::cerr << "Invalid value for key aspect_ratio\nLine: " << line << '\n';
      return false;
    }
    if (!check_excess_tokens(tokens, 3, "aspect_ratio")) {
      return false;
    }
    int w{};
    int h{};
    if (!try_parse_int(tokens, 1, w) or !try_parse_int(tokens, 2, h) or w <= 0 or h <= 0) {
      std::cerr << "Invalid value for key aspect_ratio\nLine: " << line << '\n';
      return false;
    }
    config.aspect_width  = w;
    config.aspect_height = h;
    return true;
  }

  bool handle_imagewidth(ConfigParams & config, std::vector<std::string> const & tokens,
                         std::string const & line) {
    if (tokens.size() < 2) {
      std::cerr << "Invalid value for key image_width\nLine: " << line << '\n';
      return false;
    }
    if (!check_excess_tokens(tokens, 2, "image_width")) {
      return false;
    }
    int w{};
    if (!try_parse_int(tokens, 1, w) or w <= 0) {
      std::cerr << "Invalid value for key image_width\nLine: " << line << '\n';
      return false;
    }
    config.image_width = w;
    return true;
  }

  bool handle_gamma(ConfigParams & config, std::vector<std::string> const & tokens,
                    std::string const & line) {
    if (tokens.size() < 2) {
      std::cerr << "Invalid value for key gamma\nLine: " << line << '\n';
      return false;
    }
    if (!check_excess_tokens(tokens, 2, "gamma")) {
      return false;
    }
    double value{};
    if (!try_parse_double(tokens, 1, value)) {
      std::cerr << "Invalid value for key gamma\nLine: " << line << '\n';
      return false;
    }
    config.gamma = value;
    return true;
  }

  bool handle_cameraposition(ConfigParams & config, std::vector<std::string> const & tokens,
                             std::string const & line) {
    if (tokens.size() < 4) {
      std::cerr << "Invalid value for key camera_position\nLine: " << line << '\n';
      return false;
    }
    if (!check_excess_tokens(tokens, 4, "camera_position")) {
      return false;
    }
    double x{};
    double y{};
    double z{};
    if (!try_parse_double(tokens, 1, x) or
        !try_parse_double(tokens, 2, y) or
        !try_parse_double(tokens, 3, z))
    {
      std::cerr << "Invalid value for key camera_position\nLine: " << line << '\n';
      return false;
    }
    config.camera_x = x;
    config.camera_y = y;
    config.camera_z = z;
    return true;
  }

  bool handle_cameratarget(ConfigParams & config, std::vector<std::string> const & tokens,
                           std::string const & line) {
    if (tokens.size() < 4) {
      std::cerr << "Invalid value for key camera_target\nLine: " << line << '\n';
      return false;
    }
    if (!check_excess_tokens(tokens, 4, "camera_target")) {
      return false;
    }
    double x{};
    double y{};
    double z{};
    if (!try_parse_double(tokens, 1, x) or
        !try_parse_double(tokens, 2, y) or
        !try_parse_double(tokens, 3, z))
    {
      std::cerr << "Invalid value for key camera_target\nLine: " << line << '\n';
      return false;
    }
    config.target_x = x;
    config.target_y = y;
    config.target_z = z;
    return true;
  }

  bool handle_cameranorth(ConfigParams & config, std::vector<std::string> const & tokens,
                          std::string const & line) {
    if (tokens.size() < 4) {
      std::cerr << "Invalid value for key camera_north\nLine: " << line << '\n';
      return false;
    }
    if (!check_excess_tokens(tokens, 4, "camera_north")) {
      return false;
    }
    double x{};
    double y{};
    double z{};
    if (!try_parse_double(tokens, 1, x) or
        !try_parse_double(tokens, 2, y) or
        !try_parse_double(tokens, 3, z))
    {
      std::cerr << "Invalid value for key camera_north\nLine: " << line << '\n';
      return false;
    }
    config.north_x = x;
    config.north_y = y;
    config.north_z = z;
    return true;
  }

  bool handle_fieldofview(ConfigParams & config, std::vector<std::string> const & tokens,
                          std::string const & line) {
    if (tokens.size() < 2) {
      std::cerr << "Invalid value for key field_of_view\nLine: " << line << '\n';
      return false;
    }
    if (!check_excess_tokens(tokens, 2, "field_of_view")) {
      return false;
    }
    double fov{};
    if (!try_parse_double(tokens, 1, fov) or fov <= 0 or fov >= 180) {
      std::cerr << "Invalid value for key field_of_view\nLine: " << line << '\n';
      return false;
    }
    config.field_of_view = fov;
    return true;
  }

  bool handle_samplesperpixel(ConfigParams & config, std::vector<std::string> const & tokens,
                              std::string const & line) {
    if (tokens.size() < 2) {
      std::cerr << "Invalid value for key samples_per_pixel\nLine: " << line << '\n';
      return false;
    }
    if (!check_excess_tokens(tokens, 2, "samples_per_pixel")) {
      return false;
    }
    int samples{};
    if (!try_parse_int(tokens, 1, samples) or samples <= 0) {
      std::cerr << "Invalid value for key samples_per_pixel\nLine: " << line << '\n';
      return false;
    }
    config.samples_per_pixel = samples;
    return true;
  }

  bool handle_maxdepth(ConfigParams & config, std::vector<std::string> const & tokens,
                       std::string const & line) {
    if (tokens.size() < 2) {
      std::cerr << "Invalid value for key max_depth\nLine: " << line << '\n';
      return false;
    }
    if (!check_excess_tokens(tokens, 2, "max_depth")) {
      return false;
    }
    int depth{};
    if (!try_parse_int(tokens, 1, depth) or depth <= 0) {
      std::cerr << "Invalid value for key max_depth\nLine: " << line << '\n';
      return false;
    }
    config.max_depth = depth;
    return true;
  }

  bool handle_materialrngseed(ConfigParams & config, std::vector<std::string> const & tokens,
                              std::string const & line) {
    if (tokens.size() < 2) {
      std::cerr << "Invalid value for key material_rng_seed\nLine: " << line << '\n';
      return false;
    }
    if (!check_excess_tokens(tokens, 2, "material_rng_seed")) {
      return false;
    }
    int seed{};
    if (!try_parse_int(tokens, 1, seed) or seed <= 0) {
      std::cerr << "Invalid value for key material_rng_seed\nLine: " << line << '\n';
      return false;
    }
    config.material_rng_seed = seed;
    return true;
  }

  bool handle_rayrngseed(ConfigParams & config, std::vector<std::string> const & tokens,
                         std::string const & line) {
    if (tokens.size() < 2) {
      std::cerr << "Invalid value for key ray_rng_seed\nLine: " << line << '\n';
      return false;
    }
    if (!check_excess_tokens(tokens, 2, "ray_rng_seed")) {
      return false;
    }
    int seed{};
    if (!try_parse_int(tokens, 1, seed) or seed <= 0) {
      std::cerr << "Invalid value for key ray_rng_seed\nLine: " << line << '\n';
      return false;
    }
    config.ray_rng_seed = seed;
    return true;
  }

  bool handle_backgrounddarkcolor(ConfigParams & config, std::vector<std::string> const & tokens,
                                  std::string const & line) {
    if (tokens.size() < 4) {
      std::cerr << "Invalid value for key background_dark_color\nLine: " << line << '\n';
      return false;
    }
    if (!check_excess_tokens(tokens, 4, "background_dark_color")) {
      return false;
    }
    double r{};
    double g{};
    double b{};
    if (!try_parse_double(tokens, 1, r) or
        !try_parse_double(tokens, 2, g) or
        !try_parse_double(tokens, 3, b))
    {
      std::cerr << "Invalid value for key background_dark_color\nLine: " << line << '\n';
      return false;
    }
    if (r < 0 or r > 1 or g < 0 or g > 1 or b < 0 or b > 1) {
      std::cerr << "Invalid value for key background_dark_color\nLine: " << line << '\n';
      return false;
    }
    config.background_dark_color_r = r;
    config.background_dark_color_g = g;
    config.background_dark_color_b = b;
    return true;
  }

  bool handle_backgroundlightcolor(ConfigParams & config, std::vector<std::string> const & tokens,
                                   std::string const & line) {
    if (tokens.size() < 4) {
      std::cerr << "Invalid value for key background_light_color\nLine: " << line << '\n';
      return false;
    }
    if (!check_excess_tokens(tokens, 4, "background_light_color")) {
      return false;
    }
    double r{};
    double g{};
    double b{};
    if (!try_parse_double(tokens, 1, r) or
        !try_parse_double(tokens, 2, g) or
        !try_parse_double(tokens, 3, b))
    {
      std::cerr << "Invalid value for key background_light_color\nLine: " << line << '\n';
      return false;
    }
    if (r < 0 or r > 1 or g < 0 or g > 1 or b < 0 or b > 1) {
      std::cerr << "Invalid value for key background_light_color\nLine: " << line << '\n';
      return false;
    }
    config.background_light_color_r = r;
    config.background_light_color_g = g;
    config.background_light_color_b = b;
    return true;
  }

  // Handlers map (file-local)
  std::unordered_map<std::string, Handler> const & get_handlers() {
    static std::unordered_map<std::string, Handler> const handlers = {
      {          "aspect_ratio",         handle_aspect_ratio},
      {           "image_width",           handle_imagewidth},
      {                 "gamma",                handle_gamma},
      {       "camera_position",       handle_cameraposition},
      {         "camera_target",         handle_cameratarget},
      {          "camera_north",          handle_cameranorth},
      {         "field_of_view",          handle_fieldofview},
      {     "samples_per_pixel",      handle_samplesperpixel},
      {             "max_depth",             handle_maxdepth},
      {     "material_rng_seed",      handle_materialrngseed},
      {          "ray_rng_seed",           handle_rayrngseed},
      { "background_dark_color",  handle_backgrounddarkcolor},
      {"background_light_color", handle_backgroundlightcolor},
    };
    return handlers;
  }

  bool process_tokens(ConfigParams & config, std::vector<std::string> const & tokens,
                      std::string const & line) {
    if (tokens.empty()) {
      return true;
    }
    std::string const & key_with_colon = tokens[0];
    if (key_with_colon.empty() or key_with_colon.back() != ':') {
      std::cerr << "Unknown configuration key: " << key_with_colon << '\n';
      return false;
    }
    std::string const key = key_with_colon.substr(0, key_with_colon.length() - 1);

    auto const & handlers = get_handlers();
    auto it               = handlers.find(key);
    if (it != handlers.end()) {
      return it->second(config, tokens, line);
    }
    std::cerr << "Unknown configuration key: " << key_with_colon << '\n';
    return false;
  }

  // Internal implementation with internal linkage
  bool parse_config_impl(std::string const & filename, ConfigParams & config) {
    std::ifstream file(filename);
    if (!file.is_open()) {
      std::cerr << "Cannot open config file: " << filename << '\n';
      return false;
    }

    std::string line;
    while (std::getline(file, line)) {
      if (is_empty_line(line)) {
        continue;
      }
      auto tokens = split_line(line);
      if (tokens.empty()) {
        continue;
      }
      try {
        if (!process_tokens(config, tokens, line)) {
          return false;
        }
      } catch (std::exception const & e) {
        std::cerr << "Invalid value while parsing line: " << line << '\n';
        return false;
      }
    }
    return true;
  }

}  // namespace

// Public entrypoint (declared in header)
bool parse_config(std::string const & filename, ConfigParams & config_params) {
  return parse_config_impl(filename, config_params);
}
