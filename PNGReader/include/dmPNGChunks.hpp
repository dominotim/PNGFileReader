/******************************************************************************
(C) 2017 Author: Artem Avdoshkin
******************************************************************************/
#ifndef _DM_DATA_STRUCTURES_
#define _DM_DATA_STRUCTURES_

#include <vector>
#include <map>
#include <functional>
#include <algorithm>
#include <array>
#include "dmFilters.hpp"
#include "dmZlibConverter.hpp"


namespace png
{
typedef std::uint32_t       uint32;
typedef std::uint16_t       uint16;
typedef const uint16        cunint16;
typedef std::uint8_t        byte;
typedef std::vector<byte>   bytes;

namespace image
{

struct Pixel
{
    Pixel() : red(0), green(0), blue(0), alfa(0) {}
    Pixel(cunint16 r, cunint16 g, cunint16 b, cunint16 a)
        : red(r), green(g), blue(b), alfa(a) {}
    uint16 red;
    uint16 green;
    uint16 blue;
    uint16 alfa;
};

enum ImageType : uint32
{
    GRAY_SCALE,
    RGB,
    PALLET,
    GRAY_SCALE_ALFA,
    RGBA,
    ERROR_TYPE
};

struct DecodedImageInfo
{
    std::vector<std::vector<Pixel> > pixels;
    ImageType type;
    byte bitDepth;
};

} // image

namespace chunks
{

constexpr uint32 CODE(const char* const str)
{
    return (str[0] << 24) + (str[1] << 16) + (str[2] << 8) + str[3];
}

enum ChunkType : uint32
{
    IHDR = CODE("IHDR"),
    PLTE = CODE("PLTE"),
    IDAT = CODE("IDAT"),
    IEND = CODE("IEND"),
    tRNS = CODE("tRNS"),
    UNDEFINED
};

struct ChunkInfo
{
    std::vector<byte> data;
    size_t length;
    ChunkType type;
    uint32 crc;
};

struct Header
{
    uint32 width;
    uint32 height;
    byte bitDepth;
    byte colorType;
    byte compressionMethod;
    byte filterMethod;
    byte interlaceMethod;
    static void Read(const bytes& helper, Header& chunk);
};

struct Data
{
    std::vector<std::vector<uint16> > decodedScanlines;
    static void Read(const bytes& helper,
        const Header& header, Data& chunk, Decompressor& decoder);
};

struct Pallet
{
    typedef std::array<uint16, 4> Pix;
    std::vector<Pix> colorsByIdx;
    bool initialized = false;
    static void Read(const bytes& helper, Pallet& chunk);
};

struct Transparent
{
    std::vector<byte> paleteTransparents;
    uint16 transparent;
    std::array<uint16, 3> transparentRGB;
    bool initialized = false;
    static void Read(const bytes& helper,
        const Header& header, Transparent& chunk);
};

} // chunks

namespace helper
{
uint32 GetInt32ValueAndIncIdx(const bytes& data, size_t& idx);
bool IsValidChunk(chunks::ChunkInfo& chunk);
image::DecodedImageInfo CreateFullImageInfo(
    const chunks::Data& data,
    const chunks::Header& header,
    const chunks::Pallet& palet,
    const chunks::Transparent& trans);
} // helper

} // dm

#endif