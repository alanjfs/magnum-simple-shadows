cmake_minimum_required(VERSION 3.12.4)

project(
    MyBootstrap
    VERSION 1.0.0
    LANGUAGES CXX
)

# magnum global build options
#
set(BUILD_STATIC            ON CACHE BOOL "" FORCE)
set(BUILD_DEPRECATED        ON CACHE BOOL "" FORCE)

#
# magnum build options
#
set(TARGET_GLES2           OFF CACHE BOOL "" FORCE)
set(WITH_MESHTOOLS          ON CACHE BOOL "" FORCE)
set(WITH_PRIMITIVES         ON CACHE BOOL "" FORCE)
set(WITH_SHADERS            ON CACHE BOOL "" FORCE)
set(WITH_OBJIMPORTER        ON CACHE BOOL "" FORCE)
set(WITH_GLFWAPPLICATION    ON  CACHE BOOL "" FORCE)
set(WITH_IMGUI              ON CACHE BOOL "" FORCE)
set(MSVC2019_COMPATIBILITY  ON)  # Ensure consistent level of compatibility
                                 # even when using other compilers

#
# glfw build options
#
set(GLFW_BUILD_EXAMPLES     OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS        OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS         OFF CACHE BOOL "" FORCE)

# Where to find modules
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/modules/" ${CMAKE_MODULE_PATH})

# Where to find external non-CMake projects
set(CMAKE_PREFIX_PATH "${PROJECT_SOURCE_DIR}/external/imgui" ${CMAKE_PREFIX_PATH})

#
# Externals
#
add_subdirectory(external/glfw                 EXCLUDE_FROM_ALL)
add_subdirectory(external/corrade              EXCLUDE_FROM_ALL)
add_subdirectory(external/magnum               EXCLUDE_FROM_ALL)
add_subdirectory(external/magnum-integration   EXCLUDE_FROM_ALL)

add_subdirectory(src)
