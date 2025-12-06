add_executable(render-soa)
target_sources(render-soa 
    PRIVATE 
        src/main.cpp
        src/soa_image.cpp
        src/soa_color.cpp
        src/soa_camera.cpp
        src/soa_ray.cpp
        src/render_soa.cpp
)
target_include_directories(render-soa PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(render-soa PRIVATE Microsoft.GSL::GSL common)