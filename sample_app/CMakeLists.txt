cmake_minimum_required(VERSION 3.1)

# Have to set this before creating project.
set(CMAKE_OSX_DEPLOYMENT_TARGET 10.10)

if((CMAKE_GENERATOR STREQUAL "Unix Makefiles") AND (NOT CMAKE_BUILD_TYPE))
  message(FATAL_ERROR "You must specify a build type.")
endif()

project(YueSampleApp)
set(APP_NAME "sample_app")

add_executable(${APP_NAME} ${APP_NAME}/main.cc)

target_include_directories(${APP_NAME} PRIVATE "include")
set_target_properties(${APP_NAME} PROPERTIES
                      CXX_STANDARD 14
                      CXX_STANDARD_REQUIRED ON
                      CXX_EXTENSIONS ON)

# macOS configuration.
if(APPLE)
  find_library(APPKIT AppKit)
  find_library(IOKIT IOKit)
  find_library(SECURITY Security)
  find_library(WEBKIT WebKit)
  target_link_libraries(${APP_NAME}
                        ${APPKIT} ${IOKIT} ${SECURITY} ${WEBKIT}
                        optimized ${CMAKE_SOURCE_DIR}/lib/libyue.a
                        debug ${CMAKE_SOURCE_DIR}/lib/libyued.a)
  set_target_properties(${APP_NAME} PROPERTIES LINK_FLAGS
                        "-Wl,-dead_strip")
endif()

# win32 configuration
if(WIN32)
  target_compile_definitions(${APP_NAME} PUBLIC NOMINMAX)
  set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
               PROPERTY VS_STARTUP_PROJECT ${APP_NAME})
  set_target_properties(${APP_NAME} PROPERTIES LINK_FLAGS
                        "/DELAYLOAD:setupapi.dll \
                         /DELAYLOAD:powrprof.dll \
                         /DELAYLOAD:dwmapi.dll \
                         /SUBSYSTEM:WINDOWS")
  target_link_libraries(${APP_NAME}
                        setupapi.lib powrprof.lib ws2_32.lib dbghelp.lib
                        shlwapi.lib version.lib winmm.lib psapi.lib dwmapi.lib
                        comctl32.lib gdi32.lib gdiplus.lib
                        optimized ${CMAKE_SOURCE_DIR}/lib/libyue.lib
                        debug ${CMAKE_SOURCE_DIR}/lib/libyued.lib)
  foreach(flag_var
           CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
           CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
    string(REPLACE "/MD" "-MT" ${flag_var} "${${flag_var}}")
  endforeach()
endif()

# Linux configuration
if(UNIX AND NOT APPLE)
  find_package(PkgConfig)
  pkg_search_module(GTK3 REQUIRED gtk+-3.0)
  pkg_search_module(X11 REQUIRED x11)
  target_include_directories(${APP_NAME} PUBLIC
                             ${GTK3_INCLUDE_DIRS} ${X11_INCLUDE_DIRS})
  target_compile_options(${APP_NAME} PUBLIC
                         ${GTK3_CFLAGS_OTHER} ${X11_CFLAGS_OTHER})
  target_link_libraries(${APP_NAME}
                        optimized ${CMAKE_SOURCE_DIR}/lib/libyue.a
                        debug ${CMAKE_SOURCE_DIR}/lib/libyued.a
                        pthread dl atomic ${GTK3_LIBRARIES} ${X11_LIBRARIES})
  set_target_properties(${APP_NAME} PROPERTIES LINK_FLAGS
                        "-fdata-sections -ffunction-sections -Wl,--gc-section")
endif()
