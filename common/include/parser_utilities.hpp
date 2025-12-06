#ifndef PARSER_UTILITIES_HPP
#define PARSER_UTILITIES_HPP

#include <iostream>
#include <string>
#include <vector>

std::vector<std::string> split_line(std::string const & line);
bool is_empty_line(std::string const & line);

inline void print_error(std::string const & message) {
  std::cerr << "Error: " << message << '\n';
}

#endif  // PARSER_UTILITIES_HPP
