set(COMMON_SRC_FILES 
  "${CMAKE_SOURCE_DIR}/common/src/config_parser.cpp"
  "${CMAKE_SOURCE_DIR}/common/src/geometry_logic.cpp"
  "${CMAKE_SOURCE_DIR}/common/src/hittable.cpp"
  "${CMAKE_SOURCE_DIR}/common/src/material_logic.cpp"
  "${CMAKE_SOURCE_DIR}/common/src/math_utilities.cpp"
  "${CMAKE_SOURCE_DIR}/common/src/parser_utilities.cpp"
  "${CMAKE_SOURCE_DIR}/common/src/scene_parser.cpp"
  "${CMAKE_SOURCE_DIR}/common/src/vector.cpp"
)

set(CURRENT_DIR_SRC_FILES 
  "${CMAKE_CURRENT_SOURCE_DIR}/test_config_parser.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/test_config.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/test_geometry_logic.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/test_hit_record.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/test_hittable.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/test_material_base.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/test_material_logic.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/test_materials.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/test_math_utilites.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/test_object_base.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/test_objects.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/test_parser_utilities.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/test_ray.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/test_scene_parser.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/test_vector.cpp"
)

add_unit_test_target(
  TARGET_NAME utcommon
  SOURCE_FILES ${COMMON_SRC_FILES} ${CURRENT_DIR_SRC_FILES}
  LIBRARY_FILTER common
  COVERAGE_DIR coverage-common
  LIBRARY_TO_LINK common
)