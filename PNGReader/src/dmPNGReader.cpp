#include "stdafx.h"
#include "dmPNGReader.hpp"
#include "dmImage.hpp"
#include "dmZlibConverter.hpp"

namespace png
{

png::PNGReader::PNGReader() : m_position(0)
{
}

bool png::PNGReader::Read()
{
    if (!CheckHeader())
        return false;

    chunks::Header      header;
    chunks::Data        data;
    chunks::Pallet      palet;
    chunks::Transparent transp;
    Decompressor inflator;

    for(chunks::ChunkInfo info = NextChunk(); info.type != chunks::IEND; info = NextChunk())
    {
        if(!helper::IsValidChunk(info))
            throw "Error";
        switch (info.type)
        {
            case chunks::IHDR: chunks::Header::Read(info.data, header); break;
            case chunks::IDAT: chunks::Data::Read(info.data, header, data, inflator); break;
            case chunks::PLTE: chunks::Pallet::Read(info.data, palet); break;
            case chunks::tRNS: chunks::Transparent::Read(info.data, header, transp); break;
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

void png::PNGReader::Read(const std::string& file_path)
{
    try
    {
        std::ifstream inputStream;
        inputStream.open(file_path.c_str(), std::ios::binary | std::ios::ate);
        std::ifstream::pos_type pos = inputStream.tellg();
        inputStream.seekg(0, std::ios::beg);
        Init(inputStream, pos);
        Read();
        inputStream.close();
    }
    catch (...)
    {
        throw "Error";
    }
}

bool png::PNGReader::CheckHeader()
{
    const bytes HEADER_SYMBOLS = { 137, 80, 78, 71, 13, 10, 26, 10 };
    const byte HEADER_LENGTH = 8;
    if (m_bytes.size() < HEADER_LENGTH)
        return false;
    m_position += HEADER_LENGTH;
    return std::equal(m_bytes.begin(),
                      m_bytes.begin() + HEADER_LENGTH,
                      HEADER_SYMBOLS.begin());
}

void png::PNGReader::Init(std::ifstream& file, std::ifstream::pos_type pos)
{
    m_position = 0;
    m_bytes.resize(pos);
    file.read(reinterpret_cast<char*>(&m_bytes[0]), pos);
}

png::chunks::ChunkInfo PNGReader::NextChunk()
{
    chunks::ChunkInfo res;
    res.length = helper::GetInt32ValueAndIncIdx(m_bytes, m_position);
    res.type = static_cast<chunks::ChunkType>(helper::GetInt32ValueAndIncIdx(m_bytes, m_position));
    res.data.assign(&m_bytes[m_position], &m_bytes[m_position + res.length]);
    m_position += res.length;
    res.crc = helper::GetInt32ValueAndIncIdx(m_bytes, m_position);
    return  res;
}

}