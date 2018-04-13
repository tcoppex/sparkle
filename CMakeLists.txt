# -----------------------------------------------------------------------------
# CMake configuration file
# 
# -----------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.2.0)

project(sparkle VERSION 0.1 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)

# -----------------------------------------------------------------------------
# CMake options.
# -----------------------------------------------------------------------------

# GLEW is optional and not provided, by default extensions are loaded manually.
option(USE_GLEW OFF)

# -----------------------------------------------------------------------------
# CMake includes.
# -----------------------------------------------------------------------------

# CMake module path.
set(MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
list(APPEND CMAKE_MODULE_PATH ${MODULE_PATH})

# Custom CMake helpers.
include(Helpers)

# -----------------------------------------------------------------------------
# Build parameters.
# -----------------------------------------------------------------------------

if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
  set(DEBUG TRUE)
else()
  set(DEBUG FALSE)
endif()

# -----------------------------------------------------------------------------
# Paths.
# -----------------------------------------------------------------------------

# Project's paths.
set(SOURCE_DIR            ${CMAKE_SOURCE_DIR}/src)
set(SHADERS_DIR           ${SOURCE_DIR}/shaders)
set(OUTPUT_DIR            ${CMAKE_SOURCE_DIR}/bin)
set(THIRDPARTY            ${CMAKE_SOURCE_DIR}/thirdparty)
set(THIRDPARTY_INCLUDE    ${THIRDPARTY}/include)
set(TOOLS                 ${CMAKE_SOURCE_DIR}/tools)

# -----------------------------------------------------------------------------
# Compiler settings.
# -----------------------------------------------------------------------------

# Used to bypass MSVC settings.
set_global_output_directory(${OUTPUT_DIR})

# Check the compiler version.
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  find_gcc(5.3.1)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  if (MSVC_VERSION LESS 1900)
    message(FATAL_ERROR 
      "Your version of MSVC is too low, please upgrade to version 14.x+"
    )
  endif()
endif()

# Compiler and linker flags.
if(   (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
   OR (CMAKE_CXX_COMPILER_ID STREQUAL "Clang"))
  list(APPEND CXX_FLAGS 
    -Wall -Wextra -Wpedantic
    -Wnon-virtual-dtor -Woverloaded-virtual
    -Wuseless-cast -Wcast-align
    #-Wold-style-cast
    #-Wshadow
    -Wlogical-op
    -Wno-unused-function -Wno-unused-parameter -Wno-missing-field-initializers
    -fvisibility=hidden -fno-strict-aliasing -fno-builtin-memcmp
  )
  list(APPEND CXX_FLAGS_RELEASE -O2)
  list(APPEND CXX_FLAGS_DEBUG   -g -O0 -Wconversion)

  #list(APPEND LINK_FLAGS -Wl,--no-undefined)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  list(APPEND CXX_FLAGS    
    /W4
    /MP /EHsc-
  )
  list(APPEND CXX_FLAGS_DEBUG   /Zi)
  list(APPEND CXX_FLAGS_RELEASE /W0 /Ox /INCREMENTAL:NO /DEBUG:NONE /DNDEBUG)

  list(APPEND Definitions 
    NOMINMAX
    WIN32
    _WINDOWS
    _CRT_SECURE_NO_WARNINGS
    _USE_MATH_DEFINES
  )
else()
  message(WARNING "The compiler ${CMAKE_CXX_COMPILER_ID} has not been tested yet.")
endif()

# -----------------------------------------------------------------------------
# Thirdparties dependencies.
# -----------------------------------------------------------------------------

find_package(OpenGL REQUIRED)

# Extensions loader.
if(USE_GLEW)
  find_package(GLEW 1.13 REQUIRED)
  if(${GLEW_FOUND})
    list(APPEND Definitions -DUSE_GLEW)
  endif()
else()
  # Automatically generate header to load OpenGL extensions.
  find_package(PythonInterp REQUIRED)
  if(${PYTHONINTERP_FOUND})
    set(GLEXTGEN_PATH "${TOOLS}/glextgen")
    set(GLEXTGEN_OUTPUT_DIR "${SOURCE_DIR}/ext")
    add_custom_command(
      OUTPUT
        ${GLEXTGEN_OUTPUT_DIR}/_extensions.h
      COMMAND
        ${PYTHON_EXECUTABLE} "${GLEXTGEN_PATH}/main.py"
                             "${GLEXTGEN_PATH}/extensions"
                             "${GLEXTGEN_OUTPUT_DIR}"
      DEPENDS
        "${GLEXTGEN_PATH}/main.py"
        "${GLEXTGEN_PATH}/extensions"
      WORKING_DIRECTORY
        ${CMAKE_SOURCE_DIR}
      COMMENT
        "Generate OpenGL extensions headers !!" VERBATIM
    )
    list(APPEND GLEXTGEN_OUTPUTS 
      ${GLEXTGEN_OUTPUT_DIR}/_extensions.h
      ${GLEXTGEN_OUTPUT_DIR}/_extensions.inl
    )
    add_custom_target(GLEXTGEN_target
      ALL
      DEPENDS ${GLEXTGEN_OUTPUTS}
    )
  else()
    message(FATAL_ERROR "You need either the Python interpreter "
                        "or the GLEW library to handle OpenGL extensions.")
  endif()
  list(APPEND ADDITIONAL_MAKE_CLEAN_FILES "${GLEXTGEN_OUTPUT_DIR}")
endif(USE_GLEW)

# -----------------------------------------------------------------------------
# Thirdparties Submodule dependencies.
# -----------------------------------------------------------------------------

# GLFW, as window manager.
set(GLFW_ROOT ${THIRDPARTY}/glfw)
set(GLFW_LIBRARY glfw)
set(GLFW_INCLUDE_DIR ${GLFW_ROOT}/include)
if(NOT EXISTS ${GLFW_INCLUDE_DIR})
  message(FATAL_ERROR "You need to initialize git submodules via the command : \n"
                      "git submodule init\n"
                      "git submodule update"
  )
endif()
set(BUILD_SHARED_LIBS   TRUE  CACHE BOOL "Build GLFW as shared library.")
set(GLFW_BUILD_EXAMPLES FALSE CACHE BOOL "Do not build GLFW examples.")
set(GLFW_BUILD_TESTS    FALSE CACHE BOOL "Do not run GLFW tests.")
set(GLFW_BUILD_DOCS     FALSE CACHE BOOL "Do not build GLFW documentation.")
add_subdirectory(${GLFW_ROOT} glfw EXCLUDE_FROM_ALL)

# GLM, as math library.
set(GLM_INCLUDE_DIR ${THIRDPARTY}/glm)

# imgui, as user interface.
set(IMGUI_INCLUDE_DIR ${THIRDPARTY}/imgui)
set(IMGUI_SOURCES     ${IMGUI_INCLUDE_DIR}/imgui.cpp)

# -----------------------------------------------------------------------------
# Compilation settings.
# -----------------------------------------------------------------------------

# Definitions
list(APPEND Definitions 
  -DSHADERS_DIR="${SHADERS_DIR}"
)

# Include directories
list(APPEND IncludeDirs
  ${SOURCE_DIR}
  ${SHADERS_DIR}
  ${THIRDPARTY_INCLUDE}
  ${GLFW_INCLUDE_DIR}
  ${OPENGL_INCLUDE_DIRS}
  ${GLEW_INCLUDE_DIRS}
  ${GLM_INCLUDE_DIR}
  ${IMGUI_INCLUDE_DIR}
)

# Libraries
list(APPEND Libs
  ${GLFW_LIBRARY}
  ${OPENGL_LIBRARIES}
  ${GLEW_LIBRARIES}
)

# -----------------------------------------------------------------------------
# Files
# -----------------------------------------------------------------------------

add_subdirectory(${SOURCE_DIR})

# -----------------------------------------------------------------------------