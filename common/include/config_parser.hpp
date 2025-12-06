#ifndef CONFIG_PARSER_HPP
#define CONFIG_PARSER_HPP

#include "config.hpp"
#include <string>

bool parse_config(std::string const & filename, ConfigParams & config_params);

#endif  // CONFIG_PARSER_HPP
