/******************************************************************************
(C) 2017 Author: Artem Avdoshkin
******************************************************************************/
#include <vector>
#include <map>
#include <functional>

#ifndef _DM_DATA_STRUCTURES_
#define _DM_DATA_STRUCTURES_
namespace dm
{
typedef std::uint32_t       uint32;
typedef std::uint8_t        byte;
typedef std::vector<byte>   bytes;

namespace data
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
    UNDEFINED
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

struct ChunkInfo
{
    std::vector<byte> data;
    size_t length;
    ChunkType type;
    uint32 crc;
};

struct Pixel
{
    byte red;
    byte green;
    byte blue;
    byte alfa;
};
const size_t HEADER_LENGTH = 8;
const std::vector<byte> HEADER_SYMBOLS = { 137, 80, 78, 71, 13, 10, 26, 10 };
struct PNGHeaderChunk
{
    size_t m_headerLength;
    std::vector<byte> m_header;
};

struct HeaderChunk
{
    uint32 width;
    uint32 height;
    byte bitDepth;
    byte colorType;
    byte compressionMethod;
    byte filterMethod;
    byte interlaceMethod;
};

struct PaletChunk
{
    std::vector<Pixel> colorsByIdx;
    bool initialized = false;
};

struct DecodedImageInfo
{
    std::vector<std::vector<Pixel> > pixels;
    ImageType type;
};

struct DataChunk
{
    std::vector<std::vector<byte> > decodedScanlines;
};

} // data
} // dm

#endif