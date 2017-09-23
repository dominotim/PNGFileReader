#include "stdafx.h"
#include "dmPNGChunkHelper.hpp"
#include "dmFilters.hpp"
#include "dmZlibConverter.hpp"
#include "dmCRCTable.hpp"

#include <algorithm>

namespace dm
{

namespace
{
union
{
    uint32 value;
    struct { byte b1; byte b2; byte b3; byte b4; } bytes;
} converter;

byte GetByteInPixel(const byte colorType, const byte bitDepth)
{
    switch (colorType)
    {
        case 0:return 1 * (byte)std::ceil(bitDepth * 1.0 / 8.);
        case 2:return 3 * (byte)std::ceil(bitDepth * 1.0 / 8.);
        case 3:return 1 * (byte)std::ceil(bitDepth * 1.0 / 8.);
        case 4:return 2 * (byte)std::ceil(bitDepth * 1.0 / 8.);
        case 6:return 4 * (byte)std::ceil(bitDepth * 1.0 / 8.);
        default: throw "e";
    }
}

std::vector<bytes> GetScanlines(const bytes& data, const data::HeaderChunk& header)
{
    std::vector<bytes> res(header.height);
    const byte BYTE_PER_PIXEL_COUNT =
        GetByteInPixel(header.colorType, header.bitDepth);
    const size_t lineLen = header.width * BYTE_PER_PIXEL_COUNT + 1;
    for (size_t i = 0, idx = 0; i < header.height; ++i)
    {
        res[i].resize(lineLen);
        std::for_each(res[i].begin(), res[i].end(),
            [&](byte& item) {item = data[idx++]; });
        switch (res[i][0])
        {
            case 0:dm::Unfilter::None(res, i, BYTE_PER_PIXEL_COUNT); break;
            case 1:dm::Unfilter::Sub(res, i, BYTE_PER_PIXEL_COUNT); break;
            case 2:dm::Unfilter::Up(res, i, BYTE_PER_PIXEL_COUNT); break;
            case 3:dm::Unfilter::Average(res, i, BYTE_PER_PIXEL_COUNT); break;
            case 4:dm::Unfilter::Paeth(res, i, BYTE_PER_PIXEL_COUNT); break;
            default: throw "e";
        }
    }
    std::for_each(res.begin(), res.end(),
        [](auto& vec)->void {vec.erase(vec.begin());});
    return res;
}

data::ImageType GetImageType(const byte bitDepth, const byte colorType)
{
    switch (colorType)
    {
    case 0: return data::GRAY_SCALE;
    case 2: return (bitDepth == 8 || bitDepth == 16) ? data::RGB : data::ERROR_TYPE;
    case 3: return (bitDepth <= 8) ? data::PALLET : data::ERROR_TYPE;
    case 4: return (bitDepth == 8 || bitDepth == 16) ? data::GRAY_SCALE_ALFA : data::ERROR_TYPE;
    case 6: return (bitDepth == 8 || bitDepth == 16) ? data::RGBA : data::ERROR_TYPE;
    default:return data::ERROR_TYPE;
    }
}

} // namespace

uint32 chunkHelper::GetInt32ValueAndIncIdx(const bytes& data, size_t& idx)
{
    converter.bytes = { data[idx + 3], data[idx + 2], data[idx + 1], data[idx] };
    idx += 4; return converter.value;
}

void chunkHelper::DecodeHeaderChunk(const bytes& data, data::HeaderChunk& chunk)
{
    size_t idx = 0;
    chunk.width = GetInt32ValueAndIncIdx(data, idx);
    chunk.height = GetInt32ValueAndIncIdx(data, idx);
    chunk.bitDepth = data[idx++];
    chunk.colorType = data[idx++];
    chunk.compressionMethod = data[idx++];
    chunk.filterMethod = data[idx++];
    chunk.interlaceMethod = data[idx++];
}

void chunkHelper::DecodeDataChunk(const bytes& data,
    const data::HeaderChunk& header, data::DataChunk& chunk, Decompressor& decoder)
{
    const std::vector<byte> uncompressed = decoder.Decompress(data);
    if (!uncompressed.empty())
        chunk.decodedScanlines = GetScanlines(uncompressed, header);
}

void chunkHelper::DecodePaletChunk(const bytes& data, data::PaletChunk& chunk)
{
    if (data.size() % 3 != 0)
        throw "Error";
    chunk.colorsByIdx.clear();
    for(size_t i = 0; i < data.size(); i += 3)
    {
        chunk.colorsByIdx.push_back(
            data::Pixel{ data[i], data[i + 1], data[i + 2], 255 });
    }
    chunk.initialized = true;
}

bool chunkHelper::IsValidChunk(data::ChunkInfo& chunk)
{
    // crc checking
    uint32 r = 0xffffffffu;
    converter.value = chunk.type;
    r = data::crcTable[(r ^ converter.bytes.b4) & 0xff] ^ (r >> 8);
    r = data::crcTable[(r ^ converter.bytes.b3) & 0xff] ^ (r >> 8);
    r = data::crcTable[(r ^ converter.bytes.b2) & 0xff] ^ (r >> 8);
    r = data::crcTable[(r ^ converter.bytes.b1) & 0xff] ^ (r >> 8);
    for (size_t i = 0; i < chunk.data.size(); ++i)
    {
        r = data::crcTable[(r ^ chunk.data[i]) & 0xff] ^ (r >> 8);
    }
    return ((r ^ 0xffffffffu) == chunk.crc);
}

data::DecodedImageInfo chunkHelper::CreateFullImageInfo(
    const data::DataChunk& data,
    const data::HeaderChunk& header,
    const data::PaletChunk& palet)
{
    data::DecodedImageInfo res;
    res.type = GetImageType(header.bitDepth, header.colorType);
    res.pixels.resize(header.height, std::vector<data::Pixel>(header.width));
    byte inc = GetByteInPixel(header.colorType, header.bitDepth);
    for (size_t i = 0; i < data.decodedScanlines.size(); ++i)
    {
        for (size_t j = 0, imIdx = 0; j < data.decodedScanlines[i].size(); j += inc, ++imIdx)
        {
            switch (res.type)
            {
            case data::GRAY_SCALE:
            {
                res.pixels[i][imIdx] =
                    { data.decodedScanlines[i][j],
                        data.decodedScanlines[i][j],
                        data.decodedScanlines[i][j],
                        255};
            } break;

            case data::RGB:
            {
                res.pixels[i][imIdx].red   = data.decodedScanlines[i][j];
                res.pixels[i][imIdx].green = data.decodedScanlines[i][j + 1];
                res.pixels[i][imIdx].blue  = data.decodedScanlines[i][j + 2];
                res.pixels[i][imIdx].alfa  = 255;
            } break;

            case data::PALLET:
            {
                if (!palet.initialized)
                    throw "e";
                res.pixels[i][imIdx] = palet.colorsByIdx[data.decodedScanlines[i][j]];
            } break;

            case data::GRAY_SCALE_ALFA:
            {
                res.pixels[i][imIdx].red = data.decodedScanlines[i][j];
                res.pixels[i][imIdx].green = data.decodedScanlines[i][j];
                res.pixels[i][imIdx].blue = data.decodedScanlines[i][j];
                res.pixels[i][imIdx].alfa = data.decodedScanlines[i][j + 1];
            } break;

            case data::RGBA:
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

} // dm