cmake_minimum_required(VERSION 3.20)
project(VulkanTriangle LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ── Dependencies ─────────────────────────────────────────────────────────────
find_package(Vulkan REQUIRED)
find_package(glfw3  REQUIRED)

add_executable(${PROJECT_NAME} src/main.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Vulkan::Vulkan
    glfw
)

target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra)

# ── Shader compilation ───────────────────────────────────────────────────────
find_program(GLSLC glslc REQUIRED HINTS
    "/usr/bin"
    "/usr/local/bin"
)

set(SHADER_DIR  "${CMAKE_SOURCE_DIR}/shaders")
set(SPIRV_DIR   "${CMAKE_BINARY_DIR}/shaders")

file(GLOB SHADER_SOURCES
    "${SHADER_DIR}/*.vert"
    "${SHADER_DIR}/*.frag"
    "${SHADER_DIR}/*.comp"
    "${SHADER_DIR}/*.geom"
)

foreach(SHADER ${SHADER_SOURCES})
  get_filename_component(SHADER_NAME ${SHADER} NAME)
  set(SPIRV_OUT "${SPIRV_DIR}/${SHADER_NAME}.spv")

  add_custom_command(
        OUTPUT  ${SPIRV_OUT}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${SPIRV_DIR}
        COMMAND ${GLSLC} ${SHADER} -o ${SPIRV_OUT}
        DEPENDS ${SHADER}
        COMMENT "Compiling shader: ${SHADER_NAME}"
    )
  list(APPEND SPIRV_OUTPUTS ${SPIRV_OUT})
endforeach()

add_custom_target(Shaders ALL DEPENDS ${SPIRV_OUTPUTS})
add_dependencies(${PROJECT_NAME} Shaders)

# Copy compiled shaders next to the binary so the relative path "shaders/*.spv" works
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${SPIRV_DIR}
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders
    COMMENT "Copying SPIR-V shaders to output directory"
)
