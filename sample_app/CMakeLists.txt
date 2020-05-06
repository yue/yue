cmake_minimum_required(VERSION 3.13)

# Target macOS version, Have to set this before creating project.
set(CMAKE_OSX_DEPLOYMENT_TARGET 10.10)

# Project name.
project(Yue)
set(LIBYUE "Yue")

# Define the sample_app if this is the top dir.
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  set(APP_NAME "sample_app")
  add_executable(${APP_NAME} sample_app/main.cc)
  target_link_libraries(${APP_NAME} ${LIBYUE})
  set_property(DIRECTORY ${CMAKE_SOURCE_DIR}
               PROPERTY VS_STARTUP_PROJECT ${APP_NAME})
endif()

# The Yue library to link to.
if(APPLE)
  file(GLOB_RECURSE YUE_SOURCES src/mac/*.cc src/mac/*.mm src/mac/*.S src/mac/*.c)
elseif(WIN32)
  file(GLOB_RECURSE YUE_SOURCES src/win/*.cc src/win/*.c)
elseif(UNIX AND NOT APPLE)
  file(GLOB_RECURSE YUE_SOURCES src/linux/*.cc src/linux/*.c)
endif()
add_library(${LIBYUE} ${YUE_SOURCES})
target_include_directories(${LIBYUE} PUBLIC include)

# Use C++14 standard.
target_compile_features(${LIBYUE} PUBLIC cxx_std_14)

# macOS configuration.
if(APPLE)
  target_include_directories(${LIBYUE} PRIVATE src/mac)
  find_library(APPKIT AppKit)
  find_library(IOKIT IOKit)
  find_library(SECURITY Security)
  find_library(WEBKIT WebKit)
  target_link_libraries(${LIBYUE} PUBLIC
                        ${APPKIT} ${IOKIT} ${SECURITY} ${WEBKIT})
  set_target_properties(${LIBYUE} PROPERTIES COMPILE_FLAGS
                        "-Wno-deprecated-declarations")
  target_link_options(${LIBYUE} PUBLIC
                      "-Wl,-dead_strip")
endif()

# win32 configuration
if(WIN32)
  target_include_directories(${LIBYUE} PRIVATE src/win)
  target_compile_definitions(${LIBYUE} PUBLIC
                             _WINDOWS WIN32 WIN32_LEAN_AND_MEAN NOMINMAX
                             _UNICODE UNICODE)
  target_link_libraries(${LIBYUE} PUBLIC
                        setupapi.lib powrprof.lib ws2_32.lib dbghelp.lib
                        shlwapi.lib version.lib winmm.lib psapi.lib dwmapi.lib
                        propsys.lib comctl32.lib gdi32.lib gdiplus.lib
                        urlmon.lib userenv.lib)
  set_target_properties(${LIBYUE} PROPERTIES COMPILE_FLAGS
                        "/wd4244 /wd4267 /wd4312 /wd4838 /wd4996")
  target_link_options(${LIBYUE} PUBLIC
                      "/DELAYLOAD:setupapi.dll \
                       /DELAYLOAD:powrprof.dll \
                       /DELAYLOAD:dwmapi.dll \
                       /SUBSYSTEM:WINDOWS")
endif()

# Linux configuration
if(UNIX AND NOT APPLE)
  target_include_directories(${LIBYUE} PRIVATE src/linux)
  find_package(PkgConfig)
  pkg_search_module(GTK3 REQUIRED gtk+-3.0)
  pkg_search_module(X11 REQUIRED x11)
  pkg_search_module(WEBKIT2GTK REQUIRED webkit2gtk-4.0)
  target_include_directories(${LIBYUE} PUBLIC
                             ${GTK3_INCLUDE_DIRS}
                             ${X11_INCLUDE_DIRS}
                             ${WEBKIT2GTK_INCLUDE_DIRS})
  target_compile_options(${LIBYUE} PUBLIC
                         ${GTK3_CFLAGS_OTHER}
                         ${X11_CFLAGS_OTHER}
                         ${WEBKIT2GTK_CFLAGS_OTHER})
  target_link_libraries(${LIBYUE} PUBLIC
                        pthread dl atomic
                        ${GTK3_LIBRARIES}
                        ${X11_LIBRARIES}
                        ${WEBKIT2GTK_LIBRARIES})
  set_target_properties(${LIBYUE} PROPERTIES COMPILE_FLAGS
                        "-fdata-sections -ffunction-sections -Wno-deprecated-declarations")
  target_link_options(${LIBYUE} PUBLIC
                      "-Wl,--as-needed,--gc-section")
endif()
