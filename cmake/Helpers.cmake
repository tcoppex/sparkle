# -----------------------------------------------------------------------------
# 
# Some helpers functions for CMake.
# 
# (2017 - unlicense.org)
# 
# -----------------------------------------------------------------------------

# -----------------------------------------------------------------------------
# Check GCC version
# -----------------------------------------------------------------------------

# Find a GCC version that can match the minimum required
# if none were found, check on CMAKE_FIND_ROOT_PATH
function(find_gcc MIN_GCC_REQUIRED)
  if(CMAKE_C_COMPILER)
    set(Compiler ${CMAKE_C_COMPILER})
  else()
    set(Compiler ${CMAKE_CXX_COMPILER})
  endif()
  
  macro(check_gcc_version)
    execute_process(COMMAND ${Compiler} -dumpversion OUTPUT_VARIABLE GCC_VERSION)
  
    if(GCC_VERSION VERSION_LESS ${MIN_GCC_REQUIRED})
      set(OK_GCC FALSE)
    else()
      set(OK_GCC TRUE)
    endif()
  endmacro()

  check_gcc_version()

  # If no valid gcc found, check in CMAKE_FIND_ROOT_PATH
  if(NOT OK_GCC)
    if("${CMAKE_FIND_ROOT_PATH}" STREQUAL "")
      # default installation path in centos-6
      set(CMAKE_FIND_ROOT_PATH  /opt/rh/devtoolset-2/root/)
    endif()

    set(CMAKE_C_COMPILER   "${CMAKE_FIND_ROOT_PATH}/usr/bin/gcc")
    set(CMAKE_CXX_COMPILER "${CMAKE_FIND_ROOT_PATH}/usr/bin/g++")
  endif()
  
  check_gcc_version()

  # Still no chance, abort execution
  if(NOT ${OK_GCC})
    message(FATAL_ERROR "GCC ${MIN_GCC_REQUIRED}+ is required, please set CMAKE_FIND_ROOT_PATH.")
  endif()
endfunction(find_gcc)


# -----------------------------------------------------------------------------
# Specify generic output directories
# -----------------------------------------------------------------------------

function(set_global_output_directory OUTPUT_DIR)
  # Default output directory.
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR} CACHE PATH "")
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR} CACHE PATH "")
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIR} CACHE PATH "")
endfunction()

# Force output directory destination, especially for MSVC (@so7747857).
function(set_target_output_directory target ouput_dir)
  foreach(type RUNTIME LIBRARY ARCHIVE)
    set_target_properties(${target} PROPERTIES
      ${type}_OUTPUT_DIRECTORY         ${output_dir}
      ${type}_OUTPUT_DIRECTORY_DEBUG   ${output_dir}
      ${type}_OUTPUT_DIRECTORY_RELEASE ${output_dir}
    )
  endforeach()
endfunction()

# -----------------------------------------------------------------------------