/******************************************************************************
(C) 2017 Author: Artem Avdoshkin
******************************************************************************/
#ifndef _DM_DATA_STRUCTURES_
#define _DM_DATA_STRUCTURES_

#include "dmFilters.hpp"
#include "dmZlibConverter.hpp"

#include <vector>
#include <array>

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

const bytes HEAD_SYMBOLS =
{ 137, 80, 78, 71, 13, 10, 26, 10 };

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
    uint32 length;
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
    static void Read(const bytes& data, Header& chunk);
    static void Write(const Header& chunk, bytes& data);
};

struct Data
{
    std::vector<std::vector<uint16> > decodedScanlines;
    static void Read(const bytes& data,
        const Header& header, Data& chunk, Decompressor& decoder);
    static void Write(const chunks::Data& chunk,
        Decompressor& decoder, bytes& data, const bool is16Bit);
};

struct Pallet
{
    typedef std::array<uint16, 4> Pix;
    std::vector<Pix> colorsByIdx;
    bool initialized = false;
    static void Read(const bytes& data, Pallet& chunk);
};

struct Transparent
{
    std::vector<byte> paleteTransparents;
    uint16 transparent;
    std::array<uint16, 3> transparentRGB;
    bool initialized = false;
    static void Read(const bytes& data,
        const Header& header, Transparent& chunk);
};

} // chunks
class dmImage;
namespace helper
{
uint32 GetInt32ValueAndIncIdx(const bytes& data, size_t& idx);

std::tuple<byte, byte, byte, byte> GetBytesFromInt32(const uint32 value);

bool IsValidChunk(chunks::ChunkInfo& chunk);

uint32 GetCrc(chunks::ChunkInfo& chunk);

std::vector<std::vector<uint16> > GetScanlines(const png::dmImage& src);

void AddInt32ValueToByteArray(const uint32 num, bytes& str);

image::DecodedImageInfo CreateFullImageInfo(
    const chunks::Data& data,
    const chunks::Header& header,
    const chunks::Pallet& palet,
    const chunks::Transparent& trans);
} // helper

} // dm

#endif