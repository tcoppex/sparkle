set(TARGET_NAME "${CMAKE_PROJECT_NAME}_demo")

list(APPEND Sources
  main.cc
  app.cc
  events.cc
  opengl.cc
  scene.cc

  api/append_consume_buffer.cc
  api/gpu_particle.cc
  api/random_buffer.cc
  api/vector_field.cc

  ui/controller.cc

  ui/views/Debug.cc
  ui/views/Main.cc
  ui/views/Rendering.cc
  ui/views/Simulation.cc
)

list(APPEND Headers
  app.h
  arcball_camera.h
  events.h
  opengl.h
  scene.h

  api/append_consume_buffer.h
  api/gpu_particle.h
  api/random_buffer.h
  api/vector_field.h

  ui/controller.h
  ui/view.h

  ui/views/Debug.h
  ui/views/Main.h
  ui/views/Rendering.h
  ui/views/Simulation.h
  ui/views/views.h
)

file(GLOB_RECURSE Miscs 
  ${SHADERS_DIR}/*.glsl 
  ${SHADERS_DIR}/sparkle/interop.h
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
