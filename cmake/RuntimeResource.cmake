include_guard(GLOBAL)

function(target_runtime_resource TARGET_NAME)
  set(OPTIONS)
  set(SINGLE_VALUE_KEYWORDS)
  set(MULTI_VALUE_KEYWORDS INTERFACE PUBLIC PRIVATE COMPILE_OPTIONS)
  cmake_parse_arguments(
    target_runtime_resource "${OPTIONS}" "${SINGLE_VALUE_KEYWORDS}"
    "${MULTI_VALUE_KEYWORDS}" ${ARGN})

  # Create output directory
  set(RUNTIME_OUT_DIR ${CMAKE_BINARY_DIR}/runtime)
  add_custom_command(
    OUTPUT ${RUNTIME_OUT_DIR}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${RUNTIME_OUT_DIR}
  )

  foreach(RESOURCE_FILE IN LISTS target_runtime_resource_INTERFACE)
    get_filename_component(FILE_NAME ${RESOURCE_FILE} NAME)
    set(OUT_FILE "${RUNTIME_OUT_DIR}/${FILE_NAME}")
    add_custom_command(
        OUTPUT ${OUT_FILE}
        COMMAND ${CMAKE_COMMAND} -E copy "${RESOURCE_FILE}" "${OUT_FILE}"
        MAIN_DEPENDENCY ${RESOURCE_FILE} )
    list(APPEND OUT_RUNTIME_RESOURCE_FILE ${OUT_FILE})
  endforeach()

  foreach(RESOURCE_FILE IN LISTS target_runtime_resource_PUBLIC)
    get_filename_component(FILE_NAME ${RESOURCE_FILE} NAME)
    set(OUT_FILE "${RUNTIME_OUT_DIR}/${FILE_NAME}")
    add_custom_command(
        OUTPUT ${OUT_FILE}
        COMMAND ${CMAKE_COMMAND} -E copy "${RESOURCE_FILE}" "${OUT_FILE}"
        MAIN_DEPENDENCY ${RESOURCE_FILE} )
    list(APPEND OUT_RUNTIME_RESOURCE_FILE ${OUT_FILE})
  endforeach()

  foreach(RESOURCE_FILE IN LISTS target_runtime_resource_PRIVATE)
    get_filename_component(FILE_NAME ${RESOURCE_FILE} NAME)
    set(OUT_FILE "${RUNTIME_OUT_DIR}/${FILE_NAME}")
    add_custom_command(
        OUTPUT ${OUT_FILE}
        COMMAND ${CMAKE_COMMAND} -E copy "${RESOURCE_FILE}" "${OUT_FILE}"
        MAIN_DEPENDENCY ${RESOURCE_FILE} )
    list(APPEND OUT_RUNTIME_RESOURCE_FILE ${OUT_FILE})
  endforeach()

  add_custom_target(
    ${TARGET_NAME}_runtime_resource
    DEPENDS ${OUT_RUNTIME_RESOURCE_FILE})
  add_dependencies(${TARGET_NAME} ${TARGET_NAME}_runtime_resource)
endfunction()
