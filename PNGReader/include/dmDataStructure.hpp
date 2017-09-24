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
typedef std::uint16_t       uint16;
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

    /*Ancillary chunks*/
    tRNS = CODE("tRNS"),
    gAMA = CODE("gAMA"),
    cHRM = CODE("cHRM"),
    sRGB = CODE("sRGB"),
   //need iCCP chunk but i didn't want to imlement it
    
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
    uint16 red;
    uint16 green;
    uint16 blue;
    uint16 alfa;
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

struct DataChunk
{
    std::vector<std::vector<uint16> > decodedScanlines;
};

struct TransParencyChunk
{
    std::vector<byte> paleteTransparents;
    uint16 transparent;
    uint16 transparentRGB[3];
    bool initialized = false;
};

struct GammaChunk
{
    uint32 gamma = 100000;
    bool initialized = false;
};

struct PrimaryChromaticitiesChunk
{
    uint32 whiteX = 100000;
    uint32 whiteY = 100000;
    uint32 redX = 100000;
    uint32 redY = 100000;
    uint32 greenX = 100000;
    uint32 greenY = 100000;
    uint32 blueX = 100000;
    uint32 blueY = 100000;
    bool initialized = false;
};

struct StandartRGBChunk
{
    byte  renderingIntent;
    bool initialized = false;
};

struct DecodedImageInfo
{
    std::vector<std::vector<Pixel> > pixels;
    ImageType type;
    byte bitDepth;
    /*SPECIAL INFO*/
    double gamma = 1;
    PrimaryChromaticitiesChunk primaryChromaticValues;
};
} // data
} // dm

#endif