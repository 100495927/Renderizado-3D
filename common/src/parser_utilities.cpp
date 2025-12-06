/*#include "../include/parser_utilities.hpp"
#include <algorithm>
#include <cctype>
#include <iostream>
#include <sstream>
#include <vector>

#ifdef cerr
 #undef cerr
#endif

// Dividimos la línea por palabras separadas por espacios
namespace {

  std::vector<std::string> split_line(std::string const & line) {
    std::vector<std::string> tokens;
    std::istringstream iss(line);
    std::string token;

    while (iss >> token) {
      tokens.push_back(token);
    }
    return tokens;
  }

}  // namespace

// Verificamos si la línea está vacía o solo blancos/tabs
bool is_empty_line(std::string const & line) {
  return std::ranges::all_of(line, [](char c) { return std::isspace(c); });
}

// Imprimimos un mensaje de error estándar
namespace {

  void print_error(std::string const & message) {
    std::cerr << "Error: " << message << '\n';
  }

}  // namespace*/

#include "../include/parser_utilities.hpp"
#include <algorithm>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>

#ifdef cerr
 #undef cerr
#endif

// Funciones públicas (SIN namespace anónimo)

// Dividimos la línea por palabras separadas por espacios
std::vector<std::string> split_line(std::string const & line) {
  std::vector<std::string> tokens;
  std::istringstream iss(line);
  std::string token;

  while (iss >> token) {
    tokens.push_back(token);
  }
  return tokens;
}

// Verificamos si la línea está vacía o solo blancos/tabs
bool is_empty_line(std::string const & line) {
  return std::ranges::all_of(line, [](char c) { return std::isspace(c); });
}
