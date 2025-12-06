#ifndef CONFIG_PARSER_HPP
#define CONFIG_PARSER_HPP

#include <string>
#include "config.hpp"

bool parse_config(const std::string &filename, ConfigParams &config_params);


#endif // CONFIG_PARSER_HPP