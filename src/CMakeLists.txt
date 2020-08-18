cmake_minimum_required(VERSION 3.4)

project(MagnumSimpleShadows CXX)

find_package(Corrade REQUIRED Main)
find_package(Magnum REQUIRED
    GL
    MeshTools
    Primitives
    Shaders
    SceneGraph
    GlfwApplication)

set_directory_properties(PROPERTIES CORRADE_USE_PEDANTIC_FLAGS ON)

corrade_add_resource(Shadows_RESOURCES resources.conf)

add_executable(magnum-simple-shadows
    ShadowsExample.cpp
    ShadowLight.h
    ShadowLight.cpp
    ShadowReceiverDrawable.cpp
    ShadowReceiverDrawable.h
    ShadowReceiverShader.cpp
    ShadowReceiverShader.h
    Types.h
    ${Shadows_RESOURCES})
target_link_libraries(magnum-simple-shadows PRIVATE
    Corrade::Main
    Magnum::Application
    Magnum::GL
    Magnum::Magnum
    Magnum::MeshTools
    Magnum::Primitives
    Magnum::SceneGraph
    Magnum::Shaders
    MagnumIntegration::ImGui
)

install(TARGETS magnum-simple-shadows DESTINATION ${MAGNUM_BINARY_INSTALL_DIR})
