# This file is published under public domain.

cmake_minimum_required(VERSION 3.1)

project(YueSampleApp)
set(APP_NAME "sample_app")

add_executable(${APP_NAME} ${APP_NAME}/main.cc)
set_property(TARGET ${APP_NAME} PROPERTY CXX_STANDARD 11)
target_include_directories(${APP_NAME} PRIVATE "include")

# macOS configuration.
if(APPLE)
  find_library(APPKIT AppKit)
  find_library(IOKIT IOKit)
  find_library(SECURITY Security)
  # Release configuration.
  target_link_libraries(${APP_NAME}
                        optimized ${APPKIT}
                        optimized ${IOKIT}
                        optimized ${SECURITY}
                        optimized ${CMAKE_SOURCE_DIR}/static_library/libyue.a)
  set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} \
      -Wl,-dead_strip")
  # Debug configuration.
  set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} \
      -Wl,-rpath,@loader_path \
      ${CMAKE_SOURCE_DIR}/shared_library/libbase.dylib \
      ${CMAKE_SOURCE_DIR}/shared_library/libnativeui.dylib")
endif()
