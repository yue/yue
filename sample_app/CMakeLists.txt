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
  target_link_libraries(${APP_NAME}
                        optimized ${APPKIT}
                        optimized ${IOKIT}
                        optimized ${SECURITY}
                        optimized ${CMAKE_SOURCE_DIR}/lib/libyue.a
                        debug ${CMAKE_SOURCE_DIR}/lib/libyued.a)
  set_target_properties(${APP_NAME} PROPERTIES LINK_FLAGS
                        "-Wl,-dead_strip")
endif()

# win32 configuration
if(WIN32)
  target_compile_definitions(${APP_NAME} PUBLIC NOMINMAX)
  set_target_properties(${APP_NAME} PROPERTIES LINK_FLAGS
                        "/DELAYLOAD:setupapi.dll \
                         /DELAYLOAD:powrprof.dll \
                         /DELAYLOAD:dwmapi.dll \
                         /SUBSYSTEM:WINDOWS")
  set_target_properties(${APP_NAME} PROPERTIES LINK_FLAGS_RELEASE
                        "/LTCG")
  target_link_libraries(${APP_NAME}
                        setupapi.lib powrprof.lib ws2_32.lib dbghelp.lib
                        shlwapi.lib version.lib winmm.lib psapi.lib dwmapi.lib
                        gdi32.lib gdiplus.lib
                        optimized ${CMAKE_SOURCE_DIR}/lib/libyue.lib
                        debug ${CMAKE_SOURCE_DIR}/lib/libyued.lib)
  foreach(flag_var
           CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
           CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
    string(REPLACE "/MD" "-MT" ${flag_var} "${${flag_var}}")
  endforeach()
endif()
