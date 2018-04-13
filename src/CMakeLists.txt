set(TARGET_NAME "${CMAKE_PROJECT_NAME}_demo")

set(Sources
  app.cc
  events.cc
  main.cc
  opengl.cc
  scene.cc
  ui.cc

  api/append_consume_buffer.cc
  api/gpu_particle.cc
  api/random_buffer.cc
  api/vector_field.cc
)

set(Headers
  app.h
  arcball_camera.h
  events.h
  opengl.h
  scene.h
  ui.h

  api/append_consume_buffer.h
  api/gpu_particle.h
  api/random_buffer.h
  api/vector_field.h
)

file(GLOB_RECURSE Miscs 
  ${SHADERS_DIR}/*.glsl 
  ${SHADERS_DIR}/*.h
)

add_executable(${TARGET_NAME}
  ${Sources}
  ${Headers}
  ${Miscs}
  ${IMGUI_SOURCES}
)

target_compile_options(${TARGET_NAME} PRIVATE
  "${CXX_FLAGS}"
  "$<$<CONFIG:Debug>:${CXX_FLAGS_DEBUG}>"
  "$<$<CONFIG:Release>:${CXX_FLAGS_RELEASE}>"
)
target_include_directories(${TARGET_NAME} PRIVATE ${IncludeDirs})
target_link_libraries(${TARGET_NAME} ${Libs})
target_compile_definitions(${TARGET_NAME} PRIVATE ${Definitions})

set_target_output_directory(${TARGET_NAME} ${OUTPUT_DIR})
