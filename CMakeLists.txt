# -----------------------------------------------------------------------------
# CMake configuration file
#
# The variables GLM_INCLUDE_DIR and GLFW_LOCATION might need to be set for
# compilation to succeed.
# -----------------------------------------------------------------------------


cmake_minimum_required(VERSION 3.1)

project(sparkle)

# -----------------------------------------------------------------------------

# Options
# -------

#cmake_policy(SET CMP0054 NEW)

# Force the use of GLEW - if found - to handle OpenGL extensions,
# otherwise a python script will generate functions loaders.
set(USE_GLEW ON)

# -----------------------------------------------------------------------------

# Paths
# -----

# cmake extensions path
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# project paths
set(SOURCE_DIR      "${CMAKE_SOURCE_DIR}/src")
set(SHADERS_DIR     "${SOURCE_DIR}/shaders")
set(THIRDPARTY      "${CMAKE_SOURCE_DIR}/thirdparty")
set(TOOLS           "${CMAKE_SOURCE_DIR}/tools")
set(OUTPUT_DIR      "${CMAKE_SOURCE_DIR}/bin")

add_definitions(-DSHADERS_DIR="${SHADERS_DIR}")

# Default output directory.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIR})

# Force output directory destination, especially for MSVC (@so7747857).
function(setTargetOutputDirectory target)
  foreach(type RUNTIME LIBRARY ARCHIVE)
    set_target_properties(${target} PROPERTIES
      ${type}_OUTPUT_DIRECTORY         ${CMAKE_${type}_OUTPUT_DIRECTORY}
      ${type}_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_${type}_OUTPUT_DIRECTORY}
      ${type}_OUTPUT_DIRECTORY_RELEASE ${CMAKE_${type}_OUTPUT_DIRECTORY}
    )
  endforeach()
endfunction()


# -----------------------------------------------------------------------------

# Compiler detection
# ------------------

if(   (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
   OR (CMAKE_CXX_COMPILER_ID STREQUAL "Clang"))
  set(CMAKE_CXX_FLAGS         "${CMAKE_CXX_FLAGS} -std=c++11")
  set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG} \
    -g -O0 -Wall -Wextra -pedantic \
    -Wno-unused-function -Wno-unused-parameter -Wno-missing-field-initializers")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2 -DNDEBUG")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  set(CMAKE_CXX_FLAGS         "${CMAKE_CXX_FLAGS} /Za")
  set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG} /W2 /DEBUG")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MP /Ox /INCREMENTAL:NO /DNDEBUG")
  add_definitions(/D_CRT_SECURE_NO_WARNINGS)
elseif(CMAKE_CXX_COMPILER)
  message(WARNING "Untested compiler: \"${CMAKE_CXX_COMPILER_ID}\".")
else()
  message(FATAL_ERROR "No compiler found.")
endif()


# -----------------------------------------------------------------------------

# Dependencies
# ------------

find_package(GLFW REQUIRED)
find_package(OpenGL REQUIRED)

if(USE_GLEW)
  find_package(GLEW 1.13 REQUIRED)
  if(${GLEW_FOUND})
    add_definitions(-DUSE_GLEW)
  endif()
endif(USE_GLEW)

# -----------------------------------------------------------------------------

# Main target build
# -----------------

set(TARGET_NAME "${CMAKE_PROJECT_NAME}_demo")

file(GLOB Sources ${SOURCE_DIR}/*.cc ${SOURCE_DIR}/api/*.cc )
file(GLOB Headers ${SOURCE_DIR}/*.h ${SOURCE_DIR}/api/*.h )
file(GLOB_RECURSE Miscs ${SHADERS_DIR}/*.glsl ${SHADERS_DIR}/*.h)

add_executable(${TARGET_NAME}
  ${Sources}
  ${Headers}
  ${Miscs}
)

include_directories(
  ${SOURCE_DIR}
  ${SHADERS_DIR}
  ${THIRDPARTY}
  ${GLFW_INCLUDE_DIR}
  ${OPENGL_INCLUDE_DIRS}
  ${GLEW_INCLUDE_DIRS}
  ${GLM_INCLUDE_DIR}
)

target_link_libraries(${TARGET_NAME}
  ${GLFW_LIBRARIES}
  ${OPENGL_LIBRARIES}
  ${GLEW_LIBRARIES}
)

# -----------------------------------------------------------------------------

# Custom command
# --------------

if(NOT USE_GLEW)
  # Automatically generate header to load OpenGL extensions
  find_package(PythonInterp)
  if(${PYTHONINTERP_FOUND})
    set(GLEXTGEN_PATH "${TOOLS}/glextgen")
    set(GLEXTGEN_OUTPUT_DIR "${SOURCE_DIR}/ext")
    add_custom_target(
      OpenGLExtensionGenerator
      COMMAND
        ${PYTHON_EXECUTABLE} "${GLEXTGEN_PATH}/main.py"
                             "${GLEXTGEN_PATH}/extensions"
                             "${GLEXTGEN_OUTPUT_DIR}"
      WORKING_DIRECTORY
        ${CMAKE_SOURCE_DIR}
      COMMENT
        "Generate OpenGL extensions headers" VERBATIM
      SOURCES
          "${GLEXTGEN_PATH}/main.py"
          "${GLEXTGEN_PATH}/extensions"
    )
    # TODO not ideal cause it force recompilation every time
    # best to specify outputs file where TARGET depends on.
    add_dependencies(${TARGET_NAME} OpenGLExtensionGenerator)
  endif()

  list(APPEND ADDITIONAL_MAKE_CLEAN_FILES "${GLEXTGEN_OUTPUT_DIR}")
endif()

# -----------------------------------------------------------------------------
