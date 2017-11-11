/******************************************************************************
(C) 2017 Author: Artem Avdoshkin
******************************************************************************/
#include "stdafx.h"
#include "dmPNGReader.hpp"
#include "dmImage.hpp"
#include "dmZlibConverter.hpp"

namespace png
{
namespace
{

void AddChunkToByteArray(bytes& str, const chunks::ChunkInfo& toWrite)
{
    helper::AddInt32ValueToByteArray(toWrite.length, str);
    helper::AddInt32ValueToByteArray(toWrite.type, str);
    str.insert(str.end(), toWrite.data.begin(), toWrite.data.end());
    helper::AddInt32ValueToByteArray(toWrite.crc, str);
}

bytes ConvertImgToByteArray(const png::dmImage& src)
{
    bytes res(chunks::HEAD_SYMBOLS);
    chunks::Header       header;
    chunks::Data         data;
    Decompressor         inflator;

    header.bitDepth = 8;
    header.colorType = 6;
    header.compressionMethod = 0;
    header.filterMethod = 0;
    header.interlaceMethod = 0;
    header.height = src.GetHeight();
    header.width = src.GetWidth();

    chunks::ChunkInfo firstChunk;
    firstChunk.type = chunks::IHDR;
    chunks::Header::Write(header, firstChunk.data);
    firstChunk.length = static_cast<uint32>(firstChunk.data.size());
    firstChunk.crc = helper::GetCrc(firstChunk);
    data.decodedScanlines = helper::GetScanlines(src);

    chunks::ChunkInfo secondChunk;
    secondChunk.type = chunks::IDAT;
    chunks::Data::Write(data, inflator, secondChunk.data, src.Is16BitDepth());
    secondChunk.length = static_cast<uint32>(secondChunk.data.size());
    secondChunk.crc = helper::GetCrc(secondChunk);
    AddChunkToByteArray(res, firstChunk);
    AddChunkToByteArray(res, secondChunk);

    chunks::ChunkInfo thirdChunk;
    thirdChunk.type = chunks::IEND;
    thirdChunk.length = static_cast<uint32>(secondChunk.data.size());
    thirdChunk.crc = helper::GetCrc(secondChunk);
    AddChunkToByteArray(res, thirdChunk);
    return res;
}

}

png::PNGReader::PNGReader() : m_pos(0) {}

void png::PNGReader::Read(const std::string& path, png::dmImage& image)
{
    std::ifstream file;
    file.open(path.c_str(), std::ios::binary | std::ios::ate);
    std::ifstream::pos_type pos = file.tellg();
    file.seekg(0, std::ios::beg);
    Init(file, pos);
    Read(image);
    file.close();
}

void png::PNGReader::Write(const std::string& path, const png::dmImage& image)
{
    std::ofstream file;
    file.open(path.c_str(), std::ios::binary);
    bytes converted = ConvertImgToByteArray(image);
    char* str = reinterpret_cast<char*>(&converted[0]);
    file.write(str, converted.size());
    file.close();
}

bool png::PNGReader::Read(png::dmImage& image)
{
    if (!CheckHeader())
        return false;
    chunks::Header       header;
    chunks::Data         data;
    chunks::Pallet       palet;
    chunks::Transparent  transp;
    Decompressor         inflator;
    using namespace chunks;
    for(ChunkInfo info = GetNetxChunk(); info.type != IEND; info = GetNetxChunk())
    {
        if(!helper::IsValidChunk(info))
            throw std::runtime_error("Invalid chunk crc sum");
        switch (info.type)
        {
            case IHDR: Header::Read(info.data, header); break;
            case IDAT: Data::Read(info.data, header, data, inflator); break;
            case PLTE: Pallet::Read(info.data, palet); break;
            case tRNS: Transparent::Read(info.data, header, transp); break;
            default: break;
        }
    }
    image::DecodedImageInfo inf =
        helper::CreateFullImageInfo(data, header, palet, transp);
    std::swap(image, dmImage(inf.pixels, inf.bitDepth == 16));
    return true;
}

void png::PNGReader::Init(
    std::ifstream& file,
    const std::ifstream::pos_type pos)
{
    m_pos = 0;
    m_bytes.resize(pos);
    file.read(reinterpret_cast<char*>(&m_bytes[0]), pos);
}

bool png::PNGReader::CheckHeader()
{
    if (m_bytes.size() < chunks::HEAD_SYMBOLS.size())
        return false;
    m_pos += chunks::HEAD_SYMBOLS.size();
    return std::equal(m_bytes.begin(),
        m_bytes.begin() + chunks::HEAD_SYMBOLS.size(), chunks::HEAD_SYMBOLS.begin());
}

png::chunks::ChunkInfo PNGReader::GetNetxChunk()
{
    const auto Get32 = helper::GetInt32ValueAndIncIdx;
    chunks::ChunkInfo res;
    res.length = Get32(m_bytes, m_pos);
    res.type = chunks::ChunkType(Get32(m_bytes, m_pos));
    res.data.assign(&m_bytes[m_pos], &m_bytes[m_pos + res.length]);
    m_pos += res.length;
    res.crc = Get32(m_bytes, m_pos);
    return  res;
}

}