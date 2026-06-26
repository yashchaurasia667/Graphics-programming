cmake_minimum_required(VERSION 3.20)
project(VulkanTriangle LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# EXTERNAL LIBRARIES
find_package(Vulkan REQUIRED)
find_package(SDL3 REQUIRED)
find_package(volk CONFIG REQUIRED)
find_package(VulkanMemoryAllocator CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(tinyobjloader REQUIRED)

find_library(Slang_LIBRARY
  NAMES slang slang-compiler
  HINTS /usr/include
  REQUIRED
)

find_library(SHADERC_LIBRARY 
  NAMES shaderc_shared
  HINTS /usr/include
)

set(KTX_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/ktx")
include_directories("${KTX_DIR}/include")
include_directories("${KTX_DIR}/other_include")

file(GLOB KTX_SOURCES "${KTX_DIR}/lib/*.c")
if(NOT KTX_SOURCES)
  message(FATAL_ERROR "No ktx sources found in ${KTX_DIR}/lib/")
endif()

add_library(ktx STATIC ${KTX_SOURCES})
target_compile_features(ktx PRIVATE c_std_99)

set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "src/")
file(GLOB_RECURSE SOURCES "src/*.cpp")
add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} PRIVATE
  Vulkan::Vulkan
  SDL3::SDL3
  volk
  GPUOpen::VulkanMemoryAllocator
  glm::glm
  tinyobjloader::tinyobjloader
  ${Slang_LIBRARY}
  ${SHADERC_LIBRARY}
  ktx
)

target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra)
find_program(GLSLC glslc REQUIRED HINTS
  /usr/bin
  /usr/local/bin
)


# set(SHADER_DIR "${CMAKE_SOURCE_DIR}/shaders")
# set(SPIRV_DIR "${CMAKE_BINARY_DIR}/shaders")
#
# file(GLOB SHADER_SOURCES
#   "${SHADER_DIR}/*.vert"
#   "${SHADER_DIR}/*.frag"
#   "${SHADER_DIR}/*.comp"
#   "${SHADER_DIR}/*.geom"
# )
#
# foreach(SHADER ${SHADER_SOURCES})
#   get_filename_component(SHADER_NAME ${SHADER} NAME)
#   set(SPIRV_OUT "${SPIRV_DIR}/${SHADER_NAME}.spv")
#
#   add_custom_command(
#     OUTPUT  ${SPIRV_OUT}
#     COMMAND ${CMAKE_COMMAND} -E make_directory ${SPIRV_DIR}
#     COMMAND ${GLSLC} ${SHADER} -o ${SPIRV_OUT}
#     DEPENDS ${SHADER}
#     COMMENT "Compiling shader: ${SHADER_NAME}"
#   )
#   list(APPEND SPIRV_OUTPUTS ${SPIRV_OUT})
# endforeach()
#
# add_custom_target(Shaders ALL DEPENDS ${SPIRV_OUTPUTS})
# add_dependencies(${PROJECT_NAME} Shaders)
#
# add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
#   COMMAND ${CMAKE_COMMAND} -E copy_directory
#     ${SPIRV_DIR}
#     $<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders
#   COMMENT "Copying SPIR-V shaders to output directory"
# )
