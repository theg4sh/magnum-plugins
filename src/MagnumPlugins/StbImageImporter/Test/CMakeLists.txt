#
#   This file is part of Magnum.
#
#   Copyright © 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018
#             Vladimír Vondruš <mosra@centrum.cz>
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
    set(PNGIMPORTER_TEST_DIR ".")
    set(JPEGIMPORTER_TEST_DIR ".")
    set(STBIMAGEIMPORTER_TEST_DIR ".")
else()
    set(PNGIMPORTER_TEST_DIR ${PROJECT_SOURCE_DIR}/src/MagnumPlugins/PngImporter/Test)
    set(JPEGIMPORTER_TEST_DIR ${PROJECT_SOURCE_DIR}/src/MagnumPlugins/JpegImporter/Test)
    set(STBIMAGEIMPORTER_TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR})
endif()

# CMake before 3.8 has broken $<TARGET_FILE*> expressions for iOS (see
# https://gitlab.kitware.com/cmake/cmake/merge_requests/404) and since Corrade
# doesn't support dynamic plugins on iOS, this sorta works around that. Should
# be revisited when updating Travis to newer Xcode (current has CMake 3.6).
if(NOT BUILD_PLUGINS_STATIC)
    set(STBIMAGEIMPORTER_PLUGIN_FILENAME $<TARGET_FILE:StbImageImporter>)

    # First replace ${} variables, then $<> generator expressions
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/configure.h.cmake
                   ${CMAKE_CURRENT_BINARY_DIR}/configure.h.in)
    file(GENERATE OUTPUT $<TARGET_FILE_DIR:StbImageImporterTest>/configure.h
        INPUT ${CMAKE_CURRENT_BINARY_DIR}/configure.h.in)
else()
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/configure.h.cmake
                   ${CMAKE_CURRENT_BINARY_DIR}/configure.h)
endif()

corrade_add_test(StbImageImporterTest StbImageImporterTest.cpp
    LIBRARIES Magnum::Trade
    FILES
        ../../PngImporter/Test/gray.png
        ../../PngImporter/Test/rgb.png
        ../../PngImporter/Test/rgba.png
        ../../PngImporter/Test/rgba-iphone.png
        ../../JpegImporter/Test/gray.jpg
        ../../JpegImporter/Test/rgb.jpg
        rgb.hdr)
if(NOT BUILD_PLUGINS_STATIC)
    target_include_directories(StbImageImporterTest PRIVATE $<TARGET_FILE_DIR:StbImageImporterTest>)
else()
    target_include_directories(StbImageImporterTest PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
    target_link_libraries(StbImageImporterTest PRIVATE StbImageImporter)
endif()
set_target_properties(StbImageImporterTest PROPERTIES FOLDER "MagnumPlugins/StbImageImporter/Test")
