add_library(common STATIC)

target_sources(common 
    PRIVATE 
        src/vector.cpp
        src/scene_parser.cpp
        src/config_parser.cpp
        src/parser_utilities.cpp
        src/hittable.cpp
        src/geometry_logic.cpp
        src/material_logic.cpp
        src/math_utilities.cpp
)

target_include_directories(common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(common PUBLIC Microsoft.GSL::GSL TBB::tbb) 