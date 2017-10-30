#include "stdafx.h"
#include "dmPNGChunks.hpp"
#include "..\include\dmCRCTable.hpp"
#include <array>

namespace png
{
namespace
{
union
{
    uint32 value;
    struct { byte b1; byte b2; byte b3; byte b4; } bytes;
}
converter;

byte GetSamplePerPixel(const byte colorType, const byte bitDepth)
{
    switch (colorType)
    {
    case 0:return 1;
    case 2:return 3;
    case 3:return 1;
    case 4:return 2;
    case 6:return 4;
    default: throw "e";
    }
}

image::ImageType GetImageType(const byte bitDepth, const byte colorType)
{
    switch (colorType)
    {
    case 0: return image::GRAY_SCALE;
    case 2: return (bitDepth == 8 || bitDepth == 16) ? image::RGB : image::ERROR_TYPE;
    case 3: return (bitDepth <= 8) ? image::PALLET : image::ERROR_TYPE;
    case 4: return (bitDepth == 8 || bitDepth == 16) ? image::GRAY_SCALE_ALFA : image::ERROR_TYPE;
    case 6: return (bitDepth == 8 || bitDepth == 16) ? image::RGBA : image::ERROR_TYPE;
    default : return image::ERROR_TYPE;
    }
}

std::vector<std::vector<uint16> > GetScanlines(const bytes& data, const chunks::Header& header)
{
    std::vector<bytes> bytes(header.height);
    const size_t MAXBITS = 16;
    const byte SAMPLES_PER_PIXEL =
        GetSamplePerPixel(header.colorType, header.bitDepth);
    const size_t lineLen = SAMPLES_PER_PIXEL
        * std::ceil(header.width * (header.bitDepth * 2. / MAXBITS)) + 1;
    const size_t bytesPerPixel = SAMPLES_PER_PIXEL * std::ceil(header.bitDepth * 1. / 8);
    for (size_t i = 0, idx = 0; i < header.height; ++i)
    {
        bytes[i].resize(lineLen);
        std::for_each(bytes[i].begin(), bytes[i].end(),
            [&](byte& item) {item = data[idx++]; });
        switch (bytes[i][0])
        {
        case 0:png::Unfilter::None(bytes, i, bytesPerPixel); break;
        case 1:png::Unfilter::Sub(bytes, i, bytesPerPixel); break;
        case 2:png::Unfilter::Up(bytes, i, bytesPerPixel); break;
        case 3:png::Unfilter::Average(bytes, i, bytesPerPixel); break;
        case 4:png::Unfilter::Paeth(bytes, i, bytesPerPixel); break;
        default: throw "e";
        }
    }
    std::for_each(bytes.begin(), bytes.end(),
    [](auto& vec)->void
    {
        vec.erase(vec.begin());
        if (vec.size() % 2 == 0) return;
        vec.push_back(0);
    });

    std::vector<std::vector<uint16> > res(header.height);
    const int shift = header.bitDepth;
    const size_t size = MAXBITS / shift;
    const size_t samplesCount = header.width * SAMPLES_PER_PIXEL;

    for (size_t i = 0; i < bytes.size(); ++i)
    {
        for (size_t j = 0; j < bytes[i].size(); j += 2)
        {
            const uint16 value = ((bytes[i][j] << 8) & 0xff00) + bytes[i][j + 1];
            for (size_t k = 0; k < size; k++)
            {
                res[i].push_back(((value << shift * k) & 0xffffu) >> (MAXBITS - shift));
                if (res[i].size() == samplesCount)
                    break;
            }
        }
    }
    return res;
}

} // namespace

    uint32 helper::GetInt32ValueAndIncIdx(const bytes& data, size_t& idx)
    {
        converter.bytes = { data[idx + 3], data[idx + 2], data[idx + 1], data[idx] };
        idx += 4; return converter.value;
    }

    bool helper::IsValidChunk(chunks::ChunkInfo& chunk)
    {
        // crc checking
        uint32 r = 0xffffffffu;
        converter.value = chunk.type;
        r = crcTable[(r ^ converter.bytes.b4) & 0xff] ^ (r >> 8);
        r = crcTable[(r ^ converter.bytes.b3) & 0xff] ^ (r >> 8);
        r = crcTable[(r ^ converter.bytes.b2) & 0xff] ^ (r >> 8);
        r = crcTable[(r ^ converter.bytes.b1) & 0xff] ^ (r >> 8);
        for (size_t i = 0; i < chunk.data.size(); ++i)
            r = crcTable[(r ^ chunk.data[i]) & 0xff] ^ (r >> 8);

        return ((r ^ 0xffffffffu) == chunk.crc);
    }


    image::DecodedImageInfo helper::CreateFullImageInfo(
        const chunks::Data& data,
        const chunks::Header& header,
        const chunks::Pallet& palet,
        const chunks::Transparent& trans)
    {
        image::DecodedImageInfo res;
        res.bitDepth = header.bitDepth;
        const double MAX_GAMMA = 100000;
        res.type = GetImageType(header.bitDepth, header.colorType);
        res.pixels.resize(header.height, std::vector<image::Pixel>(header.width));
        byte inc = GetSamplePerPixel(header.colorType, header.bitDepth);
        const uint16 MAX_CHANEL = (header.bitDepth == 16) ? 0xffffu : 0xffu;
        for (size_t i = 0; i < data.decodedScanlines.size(); ++i)
        {
            for (size_t j = 0, imIdx = 0; j < data.decodedScanlines[i].size(); j += inc, ++imIdx)
            {
                switch (res.type)
                {
                case image::GRAY_SCALE:
                {
                    res.pixels[i][imIdx] =
                    { data.decodedScanlines[i][j],
                        data.decodedScanlines[i][j],
                        data.decodedScanlines[i][j],
                        trans.initialized ? trans.transparent : MAX_CHANEL };
                } break;

                case image::RGB:
                {
                    res.pixels[i][imIdx].red = data.decodedScanlines[i][j];
                    res.pixels[i][imIdx].green = data.decodedScanlines[i][j + 1];
                    res.pixels[i][imIdx].blue = data.decodedScanlines[i][j + 2];
                    res.pixels[i][imIdx].alfa =
                        (trans.initialized
                            && res.pixels[i][imIdx].red == trans.transparentRGB[0]
                            && res.pixels[i][imIdx].green == trans.transparentRGB[1]
                            && res.pixels[i][imIdx].blue == trans.transparentRGB[2]) ? 0 : MAX_CHANEL;
                } break;

                case image::PALLET:
                {
                    const chunks::Pallet::Pix& curPixel = palet.colorsByIdx[data.decodedScanlines[i][j]];
                    res.pixels[i][imIdx] = image::Pixel(curPixel[0],curPixel[1], curPixel[2], curPixel[3]);
                    res.pixels[i][imIdx].alfa =
                        (trans.initialized
                            && trans.paleteTransparents.size() > data.decodedScanlines[i][j])
                        ? trans.paleteTransparents[data.decodedScanlines[i][j]] : MAX_CHANEL;
                } break;

                case image::GRAY_SCALE_ALFA:
                {
                    res.pixels[i][imIdx].red = data.decodedScanlines[i][j];
                    res.pixels[i][imIdx].green = data.decodedScanlines[i][j];
                    res.pixels[i][imIdx].blue = data.decodedScanlines[i][j];
                    res.pixels[i][imIdx].alfa = data.decodedScanlines[i][j + 1];
                } break;

                case image::RGBA:
                {
                    res.pixels[i][imIdx].red = data.decodedScanlines[i][j];
                    res.pixels[i][imIdx].green = data.decodedScanlines[i][j + 1];
                    res.pixels[i][imIdx].blue = data.decodedScanlines[i][j + 2];
                    res.pixels[i][imIdx].alfa = data.decodedScanlines[i][j + 3];
                } break;
                }
            }
        }
        return res;
    }

    void chunks::Header::Read(const bytes& data, chunks::Header& chunk)
    {
        size_t idx = 0;
        chunk.width = helper::GetInt32ValueAndIncIdx(data, idx);
        chunk.height = helper::GetInt32ValueAndIncIdx(data, idx);
        chunk.bitDepth = data[idx++];
        chunk.colorType = data[idx++];
        chunk.compressionMethod = data[idx++];
        chunk.filterMethod = data[idx++];
        chunk.interlaceMethod = data[idx++];
    }

    void chunks::Data::Read(const bytes& data,
        const chunks::Header& header, chunks::Data& chunk, Decompressor& decoder)
    {
        const std::vector<byte> uncompressed = decoder.Decompress(data);
        if (!uncompressed.empty())
            chunk.decodedScanlines = GetScanlines(uncompressed, header);
    }

void chunks::Pallet::Read(const bytes& data, chunks::Pallet& chunk)
{
    if (data.size() % 3 != 0)
        throw "Error";
    chunk.colorsByIdx.clear();
    for (size_t i = 0; i < data.size(); i += 3)
    {
        chunks::Pallet::Pix to = { data[i], data[i + 1], data[i + 2], 255 };
        chunk.colorsByIdx.push_back(to);
    }
    chunk.initialized = true;
}

void chunks::Transparent::Read(
    const bytes& data, const chunks::Header& header, chunks::Transparent& chunk)
{
    switch (header.colorType)
    {
        case 0:
        {
            chunk.transparentRGB[0] = (data[0] << 8) + data[1];
            chunk.transparentRGB[1] = (data[2] << 8) + data[3];
            chunk.transparentRGB[2] = (data[4] << 8) + data[5];
        }break;
        case 2:
        {
            chunk.transparent = (data[0] << 8) + data[1];
        }break;
        case 3:
        {
            chunk.paleteTransparents.clear();
            for (size_t i = 0; i < data.size(); ++i)
                chunk.paleteTransparents.push_back(data[i]);
        }break;
        default:
            break;
    }
    chunk.initialized = true;
}

}