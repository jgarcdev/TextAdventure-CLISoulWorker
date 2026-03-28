cmake_minimum_required(VERSION 3.19)

if(DEFINED CMAKE_C_COMPILER AND CMAKE_C_COMPILER MATCHES "clang-cl")
  if(NOT DEFINED XWIN_ROOT)
    if(DEFINED ENV{HOME})
      set(XWIN_ROOT "$ENV{HOME}/.xwin")
    else()
      message(FATAL_ERROR "XWIN_ROOT not set and HOME not available in environment")
    endif()
  endif()

  set(X86_LIB_DIR "${XWIN_ROOT}/crt/lib/x86_64")
  set(SDK_LIB_UCRT "${XWIN_ROOT}/sdk/lib/ucrt/x86_64")
  set(SDK_LIB_UM "${XWIN_ROOT}/sdk/lib/um/x86_64")

  # Add flags so try_compile and generator tasks link correctly.
  set(_clang_cl_cflags "--target=x86_64-pc-windows-msvc -fms-extensions -Wno-nonportable-include-path")
  set(_clang_cl_ldflags "-fuse-ld=lld ${X86_LIB_DIR}/libvcruntime.lib ${SDK_LIB_UCRT}/libucrt.lib ${X86_LIB_DIR}/libcmt.lib ${X86_LIB_DIR}/oldnames.lib ${SDK_LIB_UM}/Uuid.Lib ${SDK_LIB_UM}/User32.Lib ${SDK_LIB_UM}/kernel32.lib")

  string(APPEND CMAKE_C_FLAGS " ${_clang_cl_cflags}")
  # Persist for generator and try_compile
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}" CACHE STRING "C flags for clang-cl cross compile" FORCE)

  string(APPEND CMAKE_EXE_LINKER_FLAGS " ${_clang_cl_ldflags}")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}" CACHE STRING "Linker flags for clang-cl cross compile" FORCE)

  message(STATUS "Detected clang-cl: adding target and linker flags for Windows cross-compile")
endif()

project(clisw)

# Compile Windows resource (.rc) files to .res using `zig rc` and attach to targets
find_program(ZIG_EXECUTABLE zig)
if(NOT ZIG_EXECUTABLE)
  message(FATAL_ERROR "zig not found; install Zig and ensure 'zig' is on PATH to build .rc resources")
endif()



set(RESOURCES_DIR "${CMAKE_SOURCE_DIR}/resources")
set(BINARY_RES_DIR "${CMAKE_BINARY_DIR}/resources")
file(MAKE_DIRECTORY ${BINARY_RES_DIR})

# set(RC_CLISW "${RESOURCES_DIR}/clisw.rc")
# set(RES_CLISW "${RESOURCES_DIR}/clisw.res")

# add_custom_command(
#   OUTPUT ${RES_CLISW}
#   COMMAND ${ZIG_EXECUTABLE} rc ${RC_CLISW}
#   DEPENDS ${RC_CLISW} "${CMAKE_SOURCE_DIR}/resources/clisw.ico"
#   WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/resources
#   COMMENT "Compiling ${RC_CLISW} -> ${RES_CLISW}"
#   )

# set(RC_LAUNCHER "${RESOURCES_DIR}/launcher.rc")
# set(RES_LAUNCHER "${RESOURCES_DIR}/launcher.res")

# add_custom_command(
#   OUTPUT ${RES_LAUNCHER}
#   COMMAND ${ZIG_EXECUTABLE} rc ${RC_LAUNCHER}
#   DEPENDS ${RC_LAUNCHER}
#   WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
#   COMMENT "Compiling ${RC_LAUNCHER} -> ${RES_LAUNCHER}"
# )

# add_custom_target(compile_resources DEPENDS ${RES_CLISW} ${RES_LAUNCHER})


set(CMAKE_RC_COMPILER "/home/aruel/c_cpp/c/CLISoulWorker/zigrc")
set(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> <SOURCE> /home/aruel/c_cpp/c/CLISoulWorker/build/<OBJECT>")
enable_language(RC)

set(SOURCES
  external/cJSON.c
  external/getline.c
  main.c
  structures/RoomTable.c
  structures/SoulWorker.c
  structures/Maze.c
  structures/DArray.c
  helpers/Error.c
  helpers/Misc.c
  control/Setup.c
  control/Keyboard.c
  control/SaveLoad.c
  control/Battle.c
  # ${RC_CLISW}
  ${CMAKE_CURRENT_SOURCE_DIR}/resources/clisw.rc
)

include_directories(headers)

set(SDK_INCLUDE_DIRS
  "${XWIN_ROOT}/sdk/include/ucrt"
  "${XWIN_ROOT}/crt/include"
  "${XWIN_ROOT}/sdk/include/um"
  "${XWIN_ROOT}/sdk/include/shared"
  "${XWIN_ROOT}/sdk/include"
)

# If CMAKE_C_COMPILER does not match "clang-cl", error out
if(NOT DEFINED CMAKE_C_COMPILER OR NOT CMAKE_C_COMPILER MATCHES "clang-cl")
  message(FATAL_ERROR "CMAKE_C_COMPILER is not set to clang-cl. Please set CMAKE_C_COMPILER to clang-cl for cross-compilation.")
endif()


add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE ${SDK_INCLUDE_DIRS})
target_link_directories(${PROJECT_NAME} PRIVATE ${X86_LIB_DIR} ${SDK_LIB_UCRT} ${SDK_LIB_UM})
target_compile_options(${PROJECT_NAME} PRIVATE "--target=x86_64-pc-windows-msvc" "-fms-extensions" "-Wno-nonportable-include-path")
target_link_options(${PROJECT_NAME} PRIVATE "-fuse-ld=lld")
target_link_libraries(${PROJECT_NAME} PRIVATE
  "${X86_LIB_DIR}/libvcruntime.lib"
  "${SDK_LIB_UCRT}/libucrt.lib"
  "${X86_LIB_DIR}/libcmt.lib"
  "${X86_LIB_DIR}/oldnames.lib"
  "${SDK_LIB_UM}/Uuid.Lib"
  "${SDK_LIB_UM}/User32.Lib"
  "${SDK_LIB_UM}/kernel32.lib"
)
# add_dependencies(${PROJECT_NAME} compile_resources)

add_definitions(-D_CRT_SECURE_NO_WARNINGS)

set(LAUNCHER_BIN "${CMAKE_BINARY_DIR}/clisw-launcher.exe")
add_custom_command(
  OUTPUT "${LAUNCHER_BIN}"
  COMMAND zig build-exe ${CMAKE_SOURCE_DIR}/launcher.zig
          "${CMAKE_SOURCE_DIR}/resources/launcher.rc"
          -target x86_64-windows
          # -O ReleaseSmall
          # -fstrip
          -fsingle-threaded
          --name clisw-launcher
  DEPENDS ${CMAKE_SOURCE_DIR}/launcher.zig ${RES_LAUNCHER}
)

add_custom_target(clisw-launcher ALL DEPENDS "${LAUNCHER_BIN}")


add_executable(clisw-installer installer.c external/getopt.c)
target_include_directories(clisw-installer PRIVATE ${CMAKE_SOURCE_DIR}/headers ${SDK_INCLUDE_DIRS})
target_link_directories(clisw-installer PRIVATE ${X86_LIB_DIR} ${SDK_LIB_UCRT} ${SDK_LIB_UM})
target_compile_options(clisw-installer PRIVATE "--target=x86_64-pc-windows-msvc" "-fms-extensions" "-Wno-nonportable-include-path")
target_link_options(clisw-installer PRIVATE "-fuse-ld=lld")
target_link_libraries(clisw-installer PRIVATE
  "${X86_LIB_DIR}/libvcruntime.lib"
  "${SDK_LIB_UCRT}/libucrt.lib"
  "${X86_LIB_DIR}/libcmt.lib"
  "${X86_LIB_DIR}/oldnames.lib"
  "${SDK_LIB_UM}/Uuid.Lib"
  "${SDK_LIB_UM}/User32.Lib"
  "${SDK_LIB_UM}/kernel32.lib"
)




add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  ${CMAKE_SOURCE_DIR}/data $<TARGET_FILE_DIR:${PROJECT_NAME}>/data)

set(PACKAGE_DIR "${CMAKE_BINARY_DIR}/CLISW")
set(PACKAGE_NAME "clisw_build.zip")

add_custom_target(package
  COMMAND ${CMAKE_COMMAND} -E make_directory ${PACKAGE_DIR}
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}>.exe ${PACKAGE_DIR}/
  COMMAND ${CMAKE_COMMAND} -E copy "${LAUNCHER_BIN}" ${PACKAGE_DIR}/
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/version ${PACKAGE_DIR}/
  COMMAND ${CMAKE_COMMAND} -E make_directory ${PACKAGE_DIR}/data/maps
  COMMAND ${CMAKE_COMMAND} -E make_directory ${PACKAGE_DIR}/data/misc
  COMMAND ${CMAKE_COMMAND} -E make_directory ${PACKAGE_DIR}/data/story
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/data/maps ${PACKAGE_DIR}/data/maps
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/data/misc ${PACKAGE_DIR}/data/misc
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/data/story ${PACKAGE_DIR}/data/story

  # Use external command for zipping
  COMMAND ${CMAKE_COMMAND} -E tar "cf" "${PACKAGE_NAME}" --format=zip ${PACKAGE_DIR}
  DEPENDS ${PROJECT_NAME} clisw-launcher
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)