#
#   This file is part of Magnum.
#
#   Copyright © 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018
#             Vladimír Vondruš <mosra@centrum.cz>
#   Copyright © 2018 Tobias Stein <stein.tobi@t-online.de>
#   Copyright © 2018 Jonathan Hale <squareys@googlemail.com>
#
#   Permission is hereby granted, free of charge, to any person obtaining a
#   copy of this software and associated documentation files (the "Software"),
#   to deal in the Software without restriction, including without limitation
#   the rights to use, copy, modify, merge, publish, distribute, sublicense,
#   and/or sell copies of the Software, and to permit persons to whom the
#   Software is furnished to do so, subject to the following conditions:
#
#   The above copyright notice and this permission notice shall be included
#   in all copies or substantial portions of the Software.
#
#   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
#   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
#   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
#   DEALINGS IN THE SOFTWARE.
#

if(CORRADE_TARGET_EMSCRIPTEN OR CORRADE_TARGET_ANDROID)
    set(TINYGLTFIMPORTER_TEST_DIR ".")
else()
    set(TINYGLTFIMPORTER_TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR})
endif()

# CMake before 3.8 has broken $<TARGET_FILE*> expressions for iOS (see
# https://gitlab.kitware.com/cmake/cmake/merge_requests/404) and since Corrade
# doesn't support dynamic plugins on iOS, this sorta works around that. Should
# be revisited when updating Travis to newer Xcode (current has CMake 3.6).
if(NOT BUILD_PLUGINS_STATIC)
    set(TINYGLTFIMPORTER_PLUGIN_FILENAME $<TARGET_FILE:TinyGltfImporter>)
    if(WITH_STBIMAGEIMPORTER)
        set(STBIMAGEIMPORTER_PLUGIN_FILENAME $<TARGET_FILE:StbImageImporter>)
    endif()

    # First replace ${} variables, then $<> generator expressions
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/configure.h.cmake
                   ${CMAKE_CURRENT_BINARY_DIR}/configure.h.in)
    file(GENERATE OUTPUT $<TARGET_FILE_DIR:TinyGltfImporterTest>/configure.h
        INPUT ${CMAKE_CURRENT_BINARY_DIR}/configure.h.in)
else()
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/configure.h.cmake
                   ${CMAKE_CURRENT_BINARY_DIR}/configure.h)
endif()

corrade_add_resource(TinyGltfImporterTest_RESOURCES resources.conf)

corrade_add_test(TinyGltfImporterTest
    TinyGltfImporterTest.cpp
    ${TinyGltfImporterTest_RESOURCES}
    LIBRARIES Magnum::Trade
    FILES
        empty.gltf
        empty.glb
        animation.gltf
        animation.glb
        animation.bin
        animation-embedded.gltf
        animation-embedded.glb
        animation-patching.bin
        animation-patching.gltf
        animation-splines-sharing.gltf
        animation-wrong.gltf
        camera.gltf
        camera.glb
        # external-data.* packed via a resource
        image.gltf
        image.glb
        image-embedded.gltf
        image-embedded.glb
        image-buffer.gltf
        image-buffer.glb
        image-buffer-embedded.gltf
        image-buffer-embedded.glb
        light.gltf
        light.glb
        material-blinnphong.gltf
        material-blinnphong.glb
        material-metallicroughness.gltf
        material-metallicroughness.glb
        material-properties.gltf
        material-properties.glb
        material-specularglossiness.gltf
        material-specularglossiness.glb
        mesh.gltf
        mesh.bin
        mesh.glb
        mesh-embedded.gltf
        mesh-embedded.glb
        mesh-multiple-primitives.gltf
        mesh-primitives.gltf
        mesh-primitives.bin
        mesh-primitives.glb
        mesh-primitives-embedded.gltf
        mesh-primitives-embedded.glb
        mesh-colors.gltf
        mesh-colors.bin
        mesh-colors.glb
        mesh-colors-embedded.gltf
        mesh-colors-embedded.glb
        mesh-with-stride.gltf
        mesh-with-stride.glb
        scene.gltf
        scene.glb
        scene-nodefault.gltf
        scene-nodefault.glb
        object-transformation.gltf
        object-transformation.glb
        object-transformation-patching.gltf
        texture.gltf
        texture.glb
        texture.png
        texture-default-sampler.gltf
        texture-default-sampler.glb)
if(NOT BUILD_PLUGINS_STATIC)
    target_include_directories(TinyGltfImporterTest PRIVATE $<TARGET_FILE_DIR:TinyGltfImporterTest>)
else()
    target_include_directories(TinyGltfImporterTest PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
    target_link_libraries(TinyGltfImporterTest PRIVATE TinyGltfImporter StbImageImporter)
endif()
set_target_properties(TinyGltfImporterTest PROPERTIES FOLDER "MagnumPlugins/TinyGltfImporter/Test")
