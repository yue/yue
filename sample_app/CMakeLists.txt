cmake_minimum_required(VERSION 3.4)

# Makefiles do not work without a build type.
if((CMAKE_GENERATOR STREQUAL "Unix Makefiles") AND (NOT CMAKE_BUILD_TYPE))
  message(FATAL_ERROR "You must specify a build type.")
endif()

# Target macOS version, Have to set this before creating project.
set(CMAKE_OSX_DEPLOYMENT_TARGET 10.10)

# Project name and executable name.
project(YueSampleApp)
set(APP_NAME "sample_app")

# The main executable.
add_executable(${APP_NAME} ${APP_NAME}/main.cc)

# Get the absolute path the libyue.
get_filename_component(LIBYUE_DIR "${CMAKE_SOURCE_DIR}" ABSOLUTE)

# Add libyue to include dirs.
set(CONFIG_NAME "$<$<CONFIG:Debug>:Debug>$<$<NOT:$<CONFIG:Debug>>:Release>")
target_include_directories(${APP_NAME}
                           PRIVATE "${LIBYUE_DIR}/include"
                           PRIVATE "${LIBYUE_DIR}/${CONFIG_NAME}/include")

# The defines from base library.
target_compile_definitions(${APP_NAME} PUBLIC
                           $<$<CONFIG:Debug>:_DEBUG>
                           $<$<CONFIG:Debug>:DYNAMIC_ANNOTATIONS_ENABLED=1>)

# Use C++14 standard.
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
  target_compile_definitions(${APP_NAME} PUBLIC OFFICIAL_BUILD)
  target_link_libraries(${APP_NAME}
                        ${APPKIT} ${IOKIT} ${SECURITY} ${WEBKIT}
                        optimized ${LIBYUE_DIR}/Release/libyue.a
                        debug ${LIBYUE_DIR}/Debug/libyue.a)
  set_target_properties(${APP_NAME} PROPERTIES LINK_FLAGS
                        "-Wl,-dead_strip")
endif()

# win32 configuration
if(WIN32)
  set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
               PROPERTY VS_STARTUP_PROJECT ${APP_NAME})
  set_target_properties(${APP_NAME} PROPERTIES LINK_FLAGS
                        "/DELAYLOAD:setupapi.dll \
                         /DELAYLOAD:powrprof.dll \
                         /DELAYLOAD:dwmapi.dll \
                         /SUBSYSTEM:WINDOWS")
  target_compile_definitions(${APP_NAME} PUBLIC NOMINMAX UNICODE _UNICODE)
  target_link_libraries(${APP_NAME}
                        setupapi.lib powrprof.lib ws2_32.lib dbghelp.lib
                        shlwapi.lib version.lib winmm.lib psapi.lib dwmapi.lib
                        propsys.lib comctl32.lib gdi32.lib gdiplus.lib
                        urlmon.lib
                        optimized ${LIBYUE_DIR}/Release/libyue.lib
                        debug ${LIBYUE_DIR}/Debug/libyue.lib)
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
  pkg_search_module(WEBKIT2GTK REQUIRED webkit2gtk-4.0)
  target_include_directories(${APP_NAME} PUBLIC
                             ${GTK3_INCLUDE_DIRS}
                             ${X11_INCLUDE_DIRS}
                             ${WEBKIT2GTK_INCLUDE_DIRS})
  target_compile_options(${APP_NAME} PUBLIC
                         ${GTK3_CFLAGS_OTHER}
                         ${X11_CFLAGS_OTHER}
                         ${WEBKIT2GTK_CFLAGS_OTHER})
  target_compile_definitions(${APP_NAME} PUBLIC
                             USE_GLIB=1 OFFICIAL_BUILD
                             $<$<CONFIG:Debug>:_GLIBCXX_DEBUG=1>)
  target_link_libraries(${APP_NAME}
                        optimized ${LIBYUE_DIR}/Release/libyue.a
                        debug ${LIBYUE_DIR}/Debug/libyue.a
                        pthread dl atomic
                        ${GTK3_LIBRARIES}
                        ${X11_LIBRARIES}
                        ${WEBKIT2GTK_LIBRARIES})
  set_target_properties(${APP_NAME} PROPERTIES LINK_FLAGS
                        "-fdata-sections -ffunction-sections -Wl,--gc-section")
endif()
