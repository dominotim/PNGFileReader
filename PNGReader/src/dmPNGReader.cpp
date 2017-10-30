#include "stdafx.h"
#include "dmPNGReader.hpp"
#include "dmImage.hpp"
#include "dmZlibConverter.hpp"

namespace png
{

png::PNGReader::PNGReader() : m_pos(0)
{
}

bool png::PNGReader::Read()
{
    if (!CheckHeader())
        return false;
    using namespace chunks;
    Header       header;
    Data         data;
    Pallet       palet;
    Transparent  transp;
    Decompressor inflator;
    for(ChunkInfo info = GetChunk(); info.type != IEND; info = GetChunk())
    {
        if(!helper::IsValidChunk(info))
            throw "Error";
        switch (info.type)
        {
            case IHDR: Header::Read(info.data, header); break;
            case IDAT: Data::Read(info.data, header, data, inflator); break;
            case PLTE: Pallet::Read(info.data, palet); break;
            case tRNS: Transparent::Read(info.data, header, transp); break;
            default: break;
        }
    }
    /*REMOVE LATER*/
    image::DecodedImageInfo inf = helper::CreateFullImageInfo(data, header, palet, transp);
    dmImage im(inf.pixels, inf.bitDepth == 16);
    DrawImage(im);
    /*REMOVE LATER*/
    return true;
}

void png::PNGReader::Read(const std::string& path)
{ 
    std::ifstream file;
    file.open(path.c_str(), std::ios::binary | std::ios::ate);
    std::ifstream::pos_type pos = file.tellg();
    file.seekg(0, std::ios::beg);
    Init(file, pos);
    Read();
    file.close();
}

void png::PNGReader::Init(
    std::ifstream& file,
    std::ifstream::pos_type pos)
{
    m_pos = 0;
    m_bytes.resize(pos);
    file.read(reinterpret_cast<char*>(&m_bytes[0]), pos);
}

bool png::PNGReader::CheckHeader()
{
    const bytes SYMBOLS =
    { 137, 80, 78, 71, 13, 10, 26, 10 };
    const byte LENGTH = 8;
    if (m_bytes.size() < LENGTH)
        return false;
    m_pos += LENGTH;
    return std::equal(m_bytes.begin(),
        m_bytes.begin() + LENGTH, SYMBOLS.begin());
}

png::chunks::ChunkInfo PNGReader::GetChunk()
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