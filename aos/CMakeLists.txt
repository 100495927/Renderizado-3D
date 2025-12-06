add_executable(render-aos)
target_sources(render-aos 
    PRIVATE 
        src/main.cpp
        src/aos_image.cpp
        src/aos_color.cpp
        src/aos_camera.cpp
        src/aos_ray.cpp
        src/render_aos.cpp
)
target_include_directories(render-aos PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(render-aos PRIVATE Microsoft.GSL::GSL common)