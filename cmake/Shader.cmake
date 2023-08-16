include_guard(GLOBAL)

# Find glslangValidator from system path
add_executable (glslang::validator IMPORTED)
find_program (GLSLANG_VALIDATOR "glslangValidator" HINTS $ENV{VULKAN_SDK}/bin REQUIRED)
set_property (TARGET glslang::validator PROPERTY IMPORTED_LOCATION "${GLSLANG_VALIDATOR}")

function(target_glsl_shaders TARGET_NAME)
  if(NOT GLSLANG_VALIDATOR)
    message(
      FATAL_ERROR "Cannot compile GLSL to SPIR-V is glslangValidator not found!"
    )
  endif()

  # Create output directory
  set(SHADER_OUT_DIR ${CMAKE_BINARY_DIR}/shader)
  add_custom_command (
    OUTPUT ${SHADER_OUT_DIR}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${SHADER_OUT_DIR}
  )

  set(OPTIONS)
  set(SINGLE_VALUE_KEYWORDS)
  set(MULTI_VALUE_KEYWORDS INTERFACE PUBLIC PRIVATE COMPILE_OPTIONS)
  cmake_parse_arguments(
    target_glsl_shaders "${OPTIONS}" "${SINGLE_VALUE_KEYWORDS}"
    "${MULTI_VALUE_KEYWORDS}" ${ARGN})

  foreach(GLSL_FILE IN LISTS target_glsl_shaders_INTERFACE)
    get_filename_component(FILE_NAME ${GLSL_FILE} NAME)
    set(SPIRV_FILE "${SHADER_OUT_DIR}/${FILE_NAME}.spv")
    add_custom_command(
        OUTPUT ${SPIRV_FILE}
        COMMAND ${GLSLANG_VALIDATOR} ${target_glsl_shaders_COMPILE_OPTIONS} -V
        "${GLSL_FILE}" -o "${SPIRV_FILE}"
        MAIN_DEPENDENCY ${GLSL_FILE} )
    list(APPEND SPIRV_BINARY_FILES ${SPIRV_FILE})
  endforeach()

  foreach(GLSL_FILE IN LISTS target_glsl_shaders_PUBLIC)
    get_filename_component(FILE_NAME ${GLSL_FILE} NAME)
    set(SPIRV_FILE "${SHADER_OUT_DIR}/${FILE_NAME}.spv")
    add_custom_command(
        OUTPUT ${SPIRV_FILE}
        COMMAND ${GLSLANG_VALIDATOR} ${target_glsl_shaders_COMPILE_OPTIONS} -V
        "${GLSL_FILE}" -o "${SPIRV_FILE}"
        MAIN_DEPENDENCY ${GLSL_FILE} )
    list(APPEND SPIRV_BINARY_FILES ${SPIRV_FILE})
  endforeach()

  foreach(GLSL_FILE IN LISTS target_glsl_shaders_PRIVATE)
    get_filename_component(FILE_NAME ${GLSL_FILE} NAME)
    set(SPIRV_FILE "${SHADER_OUT_DIR}/${FILE_NAME}.spv")
    add_custom_command(
        OUTPUT ${SPIRV_FILE}
        COMMAND ${GLSLANG_VALIDATOR} ${target_glsl_shaders_COMPILE_OPTIONS} -V
        "${GLSL_FILE}" -o "${SPIRV_FILE}"
        MAIN_DEPENDENCY ${GLSL_FILE} )
    list(APPEND SPIRV_BINARY_FILES ${SPIRV_FILE})
  endforeach()

  add_custom_target(
    ${TARGET_NAME}_shaders
    DEPENDS ${SHADER_OUT_DIR} ${SPIRV_BINARY_FILES})
  add_dependencies(${TARGET_NAME} ${TARGET_NAME}_shaders)
endfunction()
