/*
    This file is part of Magnum.

    Copyright © 2010, 2011, 2012, 2013, 2014, 2015, 2016
              Vladimír Vondruš <mosra@centrum.cz>
    Copyright © 2016 Alice Margatroid <loveoverwhelming@gmail.com>

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

#include "DrWavImporter.h"

#include <Corrade/Utility/Assert.h>
#include <Corrade/Utility/Debug.h>
#include <Corrade/Utility/Endianness.h>

#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"

#include <memory>

namespace Magnum { namespace Audio {

namespace {

#define _v(value) Buffer::Format::value
/* Number of channels = 1-8, number of bits = 1-5 */
constexpr const Buffer::Format PcmFormatTable[8][4] = {
    {_v(Mono8),   _v(Mono16),   _v(MonoFloat),   _v(MonoDouble)}, /* Mono */
    {_v(Stereo8), _v(Stereo16), _v(StereoFloat), _v(StereoDouble)}, /* Stereo */
    {Buffer::Format{}, Buffer::Format{}, Buffer::Format{}, Buffer::Format{}}, /* Not a thing */
    {_v(Quad8), _v(Quad16), _v(Quad32), _v(Quad32)},    /* Quad */
    {Buffer::Format{}, Buffer::Format{}, Buffer::Format{}, Buffer::Format{}}, /* Also not a thing */
    {_v(Surround51Channel8), _v(Surround51Channel16), _v(Surround51Channel32), _v(Surround51Channel32)}, /* 5.1 */
    {_v(Surround61Channel8), _v(Surround61Channel16), _v(Surround61Channel32), _v(Surround61Channel32)}, /* 6.1 */
    {_v(Surround71Channel8), _v(Surround71Channel16), _v(Surround71Channel32), _v(Surround71Channel32)}  /* 7.1 */
};

/* Number of channels = 1-8, divisible by 32 = 1 or 2 */
constexpr const Buffer::Format IeeeFormatTable[8][2] = {
    {_v(MonoFloat), _v(MonoDouble)},                    /* Mono */
    {_v(StereoFloat), _v(StereoDouble)},                /* Stereo */
    {Buffer::Format{}, Buffer::Format{}},               /* Not a thing */
    {_v(Quad32), _v(Quad32)},                           /* Quad */
    {Buffer::Format{}, Buffer::Format{}},               /* Also not a thing */
    {_v(Surround51Channel32), _v(Surround51Channel32)}, /* 5.1 */
    {_v(Surround61Channel32), _v(Surround61Channel32)}, /* 6.1 */
    {_v(Surround71Channel32), _v(Surround71Channel32)}  /* 7.1 */
};

/* ALaw is always 8 bits, 1/2 channels */
constexpr const Buffer::Format ALawFormatTable[2][1] = {
    {_v(MonoALaw)},
    {_v(StereoALaw)}
};

/* MuLaw is always 8 bits, 1/2 channels; higher channel support is possible */
constexpr const Buffer::Format MuLawFormatTable[2][1] = {
    {_v(MonoMuLaw)},
    {_v(StereoMuLaw)}
};
#undef _v

/* Converts 32-bit PCM into lower bit levels by skipping bytes */
Containers::Array<char> convert32Pcm(const Containers::Array<char>& container, UnsignedInt samples, UnsignedInt size) {
    Containers::Array<char> convertData(samples*size);

    UnsignedInt skip = -1, index = 0;
    for(char item : container) {
        ++skip;

        if(skip > 3) skip = 0;
        if(skip < 4 - size) continue;

        convertData[index] = item;
        ++index;
    }

    return convertData;
}

/* Reads generic audio into most compatible format; also adjusts format */
Containers::Array<char> read32fPcm(drwav* handle, UnsignedInt samples, UnsignedInt numChannels, Buffer::Format& format) {
    format = IeeeFormatTable[numChannels-1][0];

    Containers::Array<char> tempData(samples*sizeof(Float));
    drwav_read_f32(handle, samples, reinterpret_cast<float*>(tempData.begin()));

    return tempData;
}

/* Reads raw data; be sure size is exact! */
Containers::Array<char> readRaw(drwav* handle, UnsignedInt samples, UnsignedInt size) {
    Containers::Array<char> tempData(samples*size);
    drwav_read_raw(handle, samples*size, reinterpret_cast<void*>(tempData.begin()));

    return tempData;
}

/* Makes sure the drwav pointer is released appropriately */
struct DrWavDeleter {
    void operator()(drwav* handle) { drwav_close(handle); }
};

}

DrWavImporter::DrWavImporter() = default;

DrWavImporter::DrWavImporter(PluginManager::AbstractManager& manager, std::string plugin): AbstractImporter(manager, std::move(plugin)) {}

auto DrWavImporter::doFeatures() const -> Features { return Feature::OpenData; }

bool DrWavImporter::doIsOpened() const { return _data; }

void DrWavImporter::doOpenData(Containers::ArrayView<const char> data) {
    std::unique_ptr<drwav, DrWavDeleter> handle(drwav_open_memory(data.data(), data.size()));
    if(!handle) {
        Error() << "Audio::DrWavImporter::openData(): failed to open and decode WAV data";
        return;
    }

    std::uint64_t samples = handle->totalSampleCount;
    std::uint32_t frequency = handle->sampleRate;
    std::uint8_t numChannels = handle->channels;
    std::uint8_t bitsPerSample = handle->bitsPerSample;

    /* If the bits per sample is exact, we can read data raw */
    Int notExactBitsPerSample = ((bitsPerSample % 8) ? 1 : 0);

    /* Normalize bit amounts to multiples of 8, rounding up */
    UnsignedInt normalizedBytesPerSample = (bitsPerSample / 8) + notExactBitsPerSample;

    if(numChannels == 0 || numChannels == 3 || numChannels == 5 || numChannels > 8 ||
       normalizedBytesPerSample == 0 || normalizedBytesPerSample > 8) {
        Error() << "Audio::DrWavImporter::openData(): unsupported channel count"
                << numChannels << "with" << bitsPerSample
                << "bits per sample";
        return;
    }

    /* Can't load something with no samples */
    if(samples == 0) {
        Error() << "Audio::DrWavImporter::openData(): no samples";
        return;
    }

    _frequency = frequency;

    /* PCM has a lot of special cases, as we can read many formats directly */
    if(handle->translatedFormatTag == DR_WAVE_FORMAT_PCM) {
        _format = PcmFormatTable[numChannels-1][normalizedBytesPerSample-1];
        CORRADE_INTERNAL_ASSERT(_format != Buffer::Format{});

        /* If the data is exactly 8 or 16 bits, we can read it raw */
        if(!notExactBitsPerSample && normalizedBytesPerSample < 3) {
            _data = readRaw(handle.get(), samples, normalizedBytesPerSample);
            return;

        /* If the data is approximately 24 bits or has many channels, a float is more than enough */
        } else if(normalizedBytesPerSample == 3 || (normalizedBytesPerSample > 3 && numChannels > 3)) {
            _data = read32fPcm(handle.get(), samples, numChannels, _format);
            return;

        /* If the data is close to 8 or 16 bits, we can convert it from 32-bit PCM */
        } else if(normalizedBytesPerSample == 1 || normalizedBytesPerSample == 2) {
            Containers::Array<char> tempData(samples*sizeof(Int));
            drwav_read_s32(handle.get(), samples, reinterpret_cast<Int*>(tempData.begin()));

            /* 32-bit PCM can be sliced down to 8 or 16 for direct reading */
            _data = convert32Pcm(tempData, samples, normalizedBytesPerSample);

            /* Convert 8 bit data to unsigned */
            if(normalizedBytesPerSample == 1)
                for(char& item : _data) item = item - 128;
            return;
        }

        /** @todo Allow loading of 32/64 bit streams to Double format to preserve all information */

    /* ALaw of 8/16 bits with 1/2 channels can be loaded directly */
    } else if(handle->translatedFormatTag == DR_WAVE_FORMAT_ALAW) {
        if(numChannels < 3 && !notExactBitsPerSample && (bitsPerSample == 8 || bitsPerSample == 16) ) {
            _format = ALawFormatTable[numChannels-1][normalizedBytesPerSample-1];
            _data = readRaw(handle.get(), samples, normalizedBytesPerSample);
            return;
        }

    /* MuLaw of 8/16 bits with 1/2 channels can be loaded directly */
    } else if(handle->translatedFormatTag == DR_WAVE_FORMAT_MULAW) {
        if(numChannels < 3 && !notExactBitsPerSample && (bitsPerSample == 8 || bitsPerSample == 16) ) {
            _format = MuLawFormatTable[numChannels-1][normalizedBytesPerSample-1];
            _data = readRaw(handle.get(), samples, normalizedBytesPerSample);
            return;
        }

    /* IEEE float or double can be loaded directly */
    } else if(handle->translatedFormatTag == DR_WAVE_FORMAT_IEEE_FLOAT) {
        if(!notExactBitsPerSample && (bitsPerSample == 32 || bitsPerSample == 64)) {
            _format = IeeeFormatTable[numChannels-1][(normalizedBytesPerSample / 4)-1];
            _data = readRaw(handle.get(), samples, normalizedBytesPerSample);
            return;
        }
    }

    /* If we don't know what the format is, read it out as 32 bit float for compatibility */
    _data = read32fPcm(handle.get(), samples, numChannels, _format);
    return;
}

void DrWavImporter::doClose() { _data = nullptr; }

Buffer::Format DrWavImporter::doFormat() const { return _format; }

UnsignedInt DrWavImporter::doFrequency() const { return _frequency; }

Containers::Array<char> DrWavImporter::doData() {
    Containers::Array<char> copy(_data.size());
    std::copy(_data.begin(), _data.end(), copy.begin());
    return copy;
}

}}