/******************************************************************************
(C) 2017 Author: Artem Avdoshkin
******************************************************************************/
#include "stdafx.h"
#include "dmPNGChunks.hpp"
#include "dmImage.hpp"
#include "dmCRCTable.hpp"

#include <map>
#include <array>
#include <algorithm>
#include <functional>

namespace png
{
namespace
{
union Converter
{
    uint32 value;
    struct { byte b1; byte b2; byte b3; byte b4; } bytes;
};

byte GetSamplePerPixel(const byte colorType, const byte bitDepth)
{
    switch (colorType)
    {
    case 0:return  1; case 2:return  3;
    case 3:return  1; case 4:return  2;
    case 6:return  4; default:return 1;
    }
}

image::ImageType GetImageType(const byte bitDepth, const byte colorType)
{
    switch (colorType)
    {
    case 0: return image::GRAY_SCALE;
    case 2: return (bitDepth == 8 || bitDepth == 16)
        ? image::RGB : image::ERROR_TYPE;
    case 3: return (bitDepth <= 8) ? image::PALLET 
        : image::ERROR_TYPE;
    case 4: return (bitDepth == 8 || bitDepth == 16)
        ? image::GRAY_SCALE_ALFA : image::ERROR_TYPE;
    case 6: return (bitDepth == 8 || bitDepth == 16)
        ? image::RGBA : image::ERROR_TYPE;
    default : return image::ERROR_TYPE;
    }
}

std::vector<std::vector<uint16> > GetScanlines(const bytes& data, const chunks::Header& header)
{
    std::vector<bytes> bytes(header.height);
    const size_t MAXBITS = 16;
    const byte SAMPLES_PER_PIXEL =
        GetSamplePerPixel(header.colorType, header.bitDepth);
    const size_t lineLen = static_cast<size_t>(SAMPLES_PER_PIXEL
        * std::ceil(header.width * (header.bitDepth * 2. / MAXBITS)) + 1);
    const size_t bytesPerPixel = static_cast<size_t>(
        SAMPLES_PER_PIXEL * std::ceil(header.bitDepth * 1. / 8));
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
        default: throw std::runtime_error("Invalid type of png filter");
        }
    }
    std::for_each(bytes.begin(), bytes.end(), [](png::bytes& vec)->void {
        vec.erase(vec.begin());
        if (vec.size() % 2 != 0) vec.push_back(0); });

    const int shift = header.bitDepth;
    const size_t size = MAXBITS / shift;
    std::vector<std::vector<uint16> > res(header.height);
    const size_t samplesCount = header.width * SAMPLES_PER_PIXEL;
    for (size_t i = 0; i < bytes.size(); ++i)
    {
        for (size_t j = 0; j < bytes[i].size(); j += 2)
        {
            const uint16 value = ((bytes[i][j] << 8) & 0xff00) + bytes[i][j + 1];
            for (size_t k = 0; k < size && res[i].size() != samplesCount; ++k)
            {
                res[i].push_back(((value << shift * k) & 0xffffu) >> (MAXBITS - shift));
            }
        }
    }
    return res;
}

} // namespace

uint32 helper::GetInt32ValueAndIncIdx(const bytes& data, size_t& idx)
{
    Converter converter;
    converter.bytes = { data[idx + 3], data[idx + 2], data[idx + 1], data[idx] };
    idx += 4;
    return converter.value;
}

std::tuple<byte, byte, byte, byte> helper::GetBytesFromInt32(const uint32 value)
{
    Converter converter;
    converter.value = value;
    return std::make_tuple(converter.bytes.b4,
        converter.bytes.b3, converter.bytes.b2, converter.bytes.b1);
}

uint32 helper::GetCrc(chunks::ChunkInfo& chunk)
{
    uint32 r = 0xffffffffu;
    Converter converter;
    converter.value = chunk.type;
    r = crcTable[(r ^ converter.bytes.b4) & 0xff] ^ (r >> 8);
    r = crcTable[(r ^ converter.bytes.b3) & 0xff] ^ (r >> 8);
    r = crcTable[(r ^ converter.bytes.b2) & 0xff] ^ (r >> 8);
    r = crcTable[(r ^ converter.bytes.b1) & 0xff] ^ (r >> 8);
    for (size_t i = 0; i < chunk.data.size(); ++i)
        r = crcTable[(r ^ chunk.data[i]) & 0xff] ^ (r >> 8);

    return (r ^ 0xffffffffu);
}

bool helper::IsValidChunk(chunks::ChunkInfo& chunk)
{
    return GetCrc(chunk) == chunk.crc;
}

std::vector<std::vector<uint16> > helper::GetScanlines(const dmImage& src)
{
    const byte SAMPLES_PER_PIXEL = 4;
    std::vector<std::vector<uint16> > res(src.GetHeight());
    for (size_t i = 0; i < src.GetHeight(); ++i)
    {
        res[i].resize(src.GetWidth()* SAMPLES_PER_PIXEL);
        for (size_t j = 0, idx = 0; j < src.GetWidth() * SAMPLES_PER_PIXEL; j += 4)
        {
            std::tie(res[i][j], res[i][j + 1], res[i][j + 2], res[i][j + 3]) = src.get(i, idx);
            ++idx;
        }
    }
    return res;
}

void helper::AddInt32ValueToByteArray(const uint32 num, bytes& str)
{
    const size_t size = str.size();
    str.resize(size + 4);
    std::tie(str[size],
        str[size + 1],
        str[size + 2],
        str[size + 3]) = helper::GetBytesFromInt32(num);
}

image::DecodedImageInfo helper::CreateFullImageInfo(
    const chunks::Data& data,
    const chunks::Header& header,
    const chunks::Pallet& palet,
    const chunks::Transparent& trans)
{
    const double MAX_GAMMA = 100000;
    const  bool isTInit = trans.initialized;
    const std::array<uint16, 3>& tRGB = trans.transparentRGB;
    const std::vector<byte>& tPalet = trans.paleteTransparents;
    const uint16 MAX_CHANEL = (header.bitDepth == 16) ? 0xffffu : 0xffu;
    const byte inc = GetSamplePerPixel(header.colorType, header.bitDepth);

    image::DecodedImageInfo res;
    res.bitDepth = header.bitDepth;
    res.type = GetImageType(header.bitDepth, header.colorType);
    res.pixels.resize(header.height, std::vector<image::Pixel>(header.width));

    for(size_t i = 0; i < data.decodedScanlines.size(); ++i)
    {
        for(size_t j = 0, imIdx = 0; j < data.decodedScanlines[i].size(); j += inc, ++imIdx)
        {
            image::Pixel& pix = res.pixels[i][imIdx];
            const std::vector<uint16>& line = data.decodedScanlines[i];
            const chunks::Pallet::Pix& paletPix = palet.colorsByIdx[line[j]];
            switch (res.type)
            {
            case image::GRAY_SCALE: pix =
                { line[j], line[j], line[j], isTInit ? trans.transparent : MAX_CHANEL };
            break;
            case image::RGB: pix = { line[j], line[j + 1], line[j + 2],
                (isTInit && line[j] == tRGB[0]
                    && line[j + 1] == tRGB[1]
                    && line[j + 2] == tRGB[2]) ? 0u : MAX_CHANEL };
            break;
            case image::PALLET: pix = image::Pixel(paletPix[0], paletPix[1], paletPix[2],
                    (isTInit && tPalet.size() > line[j]) ? tPalet[line[j]] : MAX_CHANEL);
            break;
            case image::GRAY_SCALE_ALFA:
                pix = { line[j], line[j], line[j], line[j + 1] }; break;
            case image::RGBA:
                pix = { line[j], line[j+1], line[j+2], line[j + 3] }; break;
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

void chunks::Header::Write(const chunks::Header& chunk, bytes& data)
{
    size_t start = data.size();
    data.resize(start + 13);
    std::tie(data[start], data[start + 1], data[start + 2], data[start + 3]) =
        helper::GetBytesFromInt32(chunk.width);
    std::tie(data[start + 4], data[start + 5], data[start + 6], data[start + 7]) =
        helper::GetBytesFromInt32(chunk.height);
    data[start + 8] = chunk.bitDepth;
    data[start + 9] = chunk.colorType;
    data[start + 10] = chunk.compressionMethod;
    data[start + 11] = chunk.filterMethod;
    data[start + 12] = chunk.interlaceMethod;
}

void chunks::Data::Read(const bytes& data,
    const chunks::Header& header, chunks::Data& chunk, Decompressor& decoder)
{
    const std::vector<byte> uncompressed = decoder.Decompress(data);
    if(!uncompressed.empty())
        chunk.decodedScanlines = GetScanlines(uncompressed, header);
}

bytes ConvertScanlineToByteArray(const std::vector<std::vector<uint16> >& scanlines, const bool is16Bit)
{
    const size_t lineLen =  scanlines[0].size() + 1;
    bytes res(scanlines.size() * lineLen);
    for (size_t i = 0, idx = 0; i < scanlines.size(); ++i)
    {
        res[idx++] = 0;
        for(size_t j = 0; j < scanlines[i].size(); ++j)
        {
            res[idx++] = static_cast<byte>(
                is16Bit ? 255 * (1. * scanlines[i][j] / 0xffffu) : scanlines[i][j]);
        }
    }
    return res;
}
void chunks::Data::Write(const chunks::Data& chunk,
    Decompressor& decoder, bytes& data, const bool is16Bit)
{
    bytes uncompressed = ConvertScanlineToByteArray(chunk.decodedScanlines, is16Bit);
    uncompressed = decoder.Compress(uncompressed);
    size_t start = data.size();
    data.resize(start + uncompressed.size());
    std::copy(uncompressed.begin(), uncompressed.end(), data.begin() + start);
}

void chunks::Pallet::Read(const bytes& data, chunks::Pallet& chunk)
{
    if(data.size() % 3 != 0)
        throw std::runtime_error("Invalid number of elements in pallet chunk");
    chunk.colorsByIdx.clear();
    for(size_t i = 0; i < data.size(); i += 3)
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
        case 0 : chunk.transparentRGB = {
            static_cast<uint16>((data[0] << 8) + data[1]),
            static_cast<uint16>((data[2] << 8) + data[3]),
            static_cast<uint16>((data[4] << 8) + data[5]) };
        break;
        case 2 : chunk.transparent = (data[0] << 8) + data[1]; break;
        case 3 : chunk.paleteTransparents.clear();
            for(size_t i = 0; i < data.size(); ++i)
                chunk.paleteTransparents.push_back(data[i]);
        break;
    }
    chunk.initialized = true;
}

}