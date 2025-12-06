set(COMMON_SRC_FILES 
  "${CMAKE_SOURCE_DIR}/aos/src/aos_camera.cpp"
  "${CMAKE_SOURCE_DIR}/aos/src/aos_color.cpp"
  "${CMAKE_SOURCE_DIR}/aos/src/aos_image.cpp"
  "${CMAKE_SOURCE_DIR}/aos/src/aos_ray.cpp"
  "${CMAKE_SOURCE_DIR}/aos/src/render_aos.cpp"
)

set(CURRENT_DIR_SRC_FILES 
  "${CMAKE_CURRENT_SOURCE_DIR}/test_aos_camera.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/test_aos_color.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/test_aos_image.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/test_aos_ray.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/test_main.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/test_render_aos.cpp"
)

  add_unit_test_target(
  TARGET_NAME utaos
  SOURCE_FILES ${COMMON_SRC_FILES} ${CURRENT_DIR_SRC_FILES}
  LIBRARY_FILTER aos
  COVERAGE_DIR coverage-aos
  LIBRARY_TO_LINK common
  INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/aos/include
)