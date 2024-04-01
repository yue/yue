cmake_minimum_required(VERSION 3.13)

if(APPLE)
  # Target macOS version, Have to set this before creating project.
  set(CMAKE_OSX_DEPLOYMENT_TARGET 11.0)
elseif(WIN32)
  # Use Windows 10 SDK.
  set(CMAKE_SYSTEM_VERSION 10.0)
endif()

# Project name.
project(Yue)
set(LIBYUE "Yue")

# Define the sample_app if this is the top dir.
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  set(APP_NAME "sample_app")
  add_executable(${APP_NAME} sample_app/main.cc)
  if(WIN32)
    target_sources(${APP_NAME} PRIVATE sample_app/exe.manifest)
  elseif(UNIX AND NOT APPLE)
    target_compile_options(${APP_NAME} PRIVATE -Wno-unknown-attributes)
  endif()
  target_link_libraries(${APP_NAME} ${LIBYUE})
  set_property(DIRECTORY ${CMAKE_SOURCE_DIR}
               PROPERTY VS_STARTUP_PROJECT ${APP_NAME})
endif()

# The Yue library to link to.
if(APPLE)
  file(GLOB_RECURSE YUE_SOURCES src/mac/*.cc src/mac/*.mm src/mac/*.S src/mac/*.c)
  # Separate files that disable ARC.
  set(YUE_NO_ARC_SOURCES ${YUE_SOURCES})
  set(MAC_NO_ARC_SOURCES allocator|nativeui)
  list(FILTER YUE_SOURCES EXCLUDE REGEX ${MAC_NO_ARC_SOURCES})
  list(FILTER YUE_NO_ARC_SOURCES INCLUDE REGEX ${MAC_NO_ARC_SOURCES})
elseif(WIN32)
  file(GLOB_RECURSE YUE_SOURCES src/win/*.cc src/win/*.c)
elseif(UNIX AND NOT APPLE)
  file(GLOB_RECURSE YUE_SOURCES src/linux/*.cc src/linux/*.c)
endif()
add_library(${LIBYUE} ${YUE_SOURCES})
target_include_directories(${LIBYUE} PUBLIC include include/base/allocator/partition_allocator/src)

# Use C++20 standard.
target_compile_features(${LIBYUE} PUBLIC cxx_std_20)

# macOS configuration.
if(APPLE)
  find_library(APPKIT AppKit)
  find_library(CARBON Carbon)
  find_library(IOKIT IOKit)
  find_library(SECURITY Security)
  find_library(WEBKIT WebKit)
  find_library(OD OpenDirectory)
  target_link_libraries(${LIBYUE} PUBLIC
                        ${APPKIT} ${CARBON} ${IOKIT} ${SECURITY} ${WEBKIT} ${OD})
  target_link_options(${LIBYUE} PUBLIC
                      -Wl,-dead_strip)
  target_compile_definitions(${LIBYUE} PUBLIC
                             SYSTEM_NATIVE_UTF8)
  target_compile_options(${LIBYUE} PRIVATE
                         -Wno-deprecated-declarations
                         -Wno-unguarded-availability-new)
  # Compile files that disable ARC as a separate target.
  set(LIBYUE_NO_ARC "YueNoARC")
  add_library(${LIBYUE_NO_ARC} ${YUE_NO_ARC_SOURCES})
  # Copy properties to the new target.
  foreach(PROPERTY INCLUDE_DIRECTORIES COMPILE_FEATURES COMPILE_DEFINITIONS COMPILE_OPTIONS LINK_LIBRARIES LINK_OPTIONS)
    get_target_property(COPIED ${LIBYUE} ${PROPERTY})
    set_target_properties(${LIBYUE_NO_ARC} PROPERTIES ${PROPERTY} "${COPIED}")
  endforeach()
  # Add the new target as a dependency to LIBYUE.
  set_property(TARGET ${LIBYUE} APPEND PROPERTY INTERFACE_LINK_LIBRARIES
               ${LIBYUE_NO_ARC})
  # Enable ARC for LIBYUE.
  set_property(TARGET ${LIBYUE} APPEND PROPERTY COMPILE_OPTIONS
               -fobjc-arc -fno-objc-arc-exceptions)
endif()

# win32 configuration
if(WIN32)
  target_compile_definitions(${LIBYUE} PUBLIC
                             _WINDOWS WIN32 WIN32_LEAN_AND_MEAN NOMINMAX
                             _UNICODE UNICODE
                             # base::StackContainer is unhappy with it
                             $<$<CONFIG:Debug>:_ITERATOR_DEBUG_LEVEL=0>)
  target_link_libraries(${LIBYUE} PUBLIC
                        setupapi.lib powrprof.lib ws2_32.lib dbghelp.lib
                        shlwapi.lib version.lib winmm.lib wbemuuid.lib
                        psapi.lib dwmapi.lib propsys.lib comctl32.lib
                        gdi32.lib gdiplus.lib urlmon.lib userenv.lib
                        uxtheme.lib delayimp.lib runtimeobject.lib ntdll.lib
                        shcore.lib pdh.lib)
  target_compile_options(${LIBYUE} PRIVATE
                        /wd4005 /wd4101 /wd4244 /wd4267 /wd4996)
  target_link_options(${LIBYUE} PUBLIC
                      /SUBSYSTEM:WINDOWS
                      /DELAYLOAD:setupapi.dll
                      /DELAYLOAD:powrprof.dll
                      /DELAYLOAD:dwmapi.dll)
endif()

# Linux configuration
if(UNIX AND NOT APPLE)
  find_package(PkgConfig)
  pkg_search_module(FONTCONFIG REQUIRED fontconfig)
  pkg_search_module(PANGO REQUIRED pangoft2)
  pkg_search_module(GTK3 REQUIRED gtk+-3.0)
  pkg_search_module(X11 REQUIRED x11)
  pkg_search_module(WEBKIT2GTK REQUIRED webkit2gtk-4.0)
  target_include_directories(${LIBYUE} PUBLIC
                             ${FONTCONFIG_INCLUDE_DIRS}
                             ${PANGO_INCLUDE_DIRS}
                             ${GTK3_INCLUDE_DIRS}
                             ${X11_INCLUDE_DIRS}
                             ${WEBKIT2GTK_INCLUDE_DIRS})
  target_compile_options(${LIBYUE} PUBLIC
                         ${FONTCONFIG_CFLAGS_OTHER}
                         ${PANGO_CFLAGS_OTHER}
                         ${GTK3_CFLAGS_OTHER}
                         ${X11_CFLAGS_OTHER}
                         ${WEBKIT2GTK_CFLAGS_OTHER})
  target_link_libraries(${LIBYUE} PUBLIC
                        pthread dl atomic
                        ${FONTCONFIG_LIBRARIES}
                        ${PANGO_LIBRARIES}
                        ${GTK3_LIBRARIES}
                        ${X11_LIBRARIES}
                        ${WEBKIT2GTK_LIBRARIES})
  target_compile_definitions(${LIBYUE} PUBLIC USE_GLIB)
  target_compile_options(${LIBYUE} PRIVATE
                         -Wno-deprecated-declarations
                         -Wno-unknown-attributes
                         -fdata-sections
                         -ffunction-sections)
  target_link_options(${LIBYUE} PUBLIC
                      -Wl,--as-needed,--gc-section)
endif()
