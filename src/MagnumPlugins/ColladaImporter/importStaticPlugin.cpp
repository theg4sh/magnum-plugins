/*
    This file is part of Magnum.

    Copyright © 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018
              Vladimír Vondruš <mosra@centrum.cz>

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included
    in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
    THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.
*/

#include <Corrade/Utility/Macros.h>
#include <Magnum/configure.h>

#include "MagnumPlugins/ColladaImporter/configure.h"

#ifndef MAGNUM_BUILD_DEPRECATED
#error scheduled for removal, consider switching to AssimpImporter or a different asset format
#endif

/* I still have a test for this class and it shouldn't pollute the log there */
CORRADE_DEPRECATED_FILE("scheduled for removal, consider switching to AssimpImporter or a different asset format")

#ifdef MAGNUM_COLLADAIMPORTER_BUILD_STATIC
#include <Corrade/PluginManager/AbstractManager.h>

static int magnumColladaImporterStaticImporter() {
    CORRADE_PLUGIN_IMPORT(ColladaImporter)
    return 1;
} CORRADE_AUTOMATIC_INITIALIZER(magnumColladaImporterStaticImporter)
#endif
