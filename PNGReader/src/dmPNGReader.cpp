#include "stdafx.h"
#include "dmPNGReader.hpp"
#include "dmImage.hpp"
#include "dmPNGChunkHelper.hpp"

namespace dm
{

dm::PNGReader::PNGReader() : m_position(0)
{
}

bool dm::PNGReader::Parse()
{
    if (!CheckHeader())
        return false;
    data::HeaderChunk header;
    data::DataChunk data;
    data::PaletChunk palet;
    for (data::ChunkInfo info = NextChunk(); info.type != data::IEND; info = NextChunk())
    {
        if (!chunkHelper::IsValidChunk(info))
            throw "Error";
        switch (info.type)
        {
        case data::IHDR: chunkHelper::DecodeHeaderChunk(info.data, header); break;
        case data::IDAT: chunkHelper::DecodeDataChunk(info.data, header, data); break;
        case data::PLTE: chunkHelper::DecodePaletChunk(info.data, palet); break;
        default: break;
        }
    }
    /*REMOVE LATER*/
    data::DecodedImageInfo inf = chunkHelper::CreateFullImageInfo(data, header, palet);
    dmImage im(inf.pixels);
    DrawImage(im);
    /*REMOVE LATER*/
    return true;
}

void dm::PNGReader::Read(const std::string& file_path)
{
    try
    {
        std::ifstream inputStream;
        inputStream.open(file_path.c_str(), std::ios::binary | std::ios::ate);
        std::ifstream::pos_type pos = inputStream.tellg();
        inputStream.seekg(0, std::ios::beg);
        Init(inputStream, pos);
        Parse();
        inputStream.close();
    }
    catch (...)
    {
        throw "Error";
    }
}

bool dm::PNGReader::CheckHeader()
{
    const bytes HEADER_SYMBOLS = { 137, 80, 78, 71, 13, 10, 26, 10 };
    const byte HEADER_LENGTH = 8;
    if (m_bytes.size() < HEADER_LENGTH)
        return false;
    m_position += HEADER_LENGTH;
    return std::equal(m_bytes.begin(), m_bytes.begin() + HEADER_LENGTH, HEADER_SYMBOLS.begin());
}

void dm::PNGReader::Init(std::ifstream& file, std::ifstream::pos_type pos)
{
    m_position = 0;
    m_bytes.resize(pos);
    file.read(reinterpret_cast<char*>(&m_bytes[0]), pos);
}

dm::data::ChunkInfo PNGReader::NextChunk()
{
    data::ChunkInfo res;
    res.length = chunkHelper::GetInt32ValueAndIncIdx(m_bytes, m_position);
    res.type = (data::ChunkType)chunkHelper::GetInt32ValueAndIncIdx(m_bytes, m_position);
    res.data.assign(&m_bytes[m_position], &m_bytes[m_position + res.length]);
    m_position += res.length;
    res.crc = chunkHelper::GetInt32ValueAndIncIdx(m_bytes, m_position);
    return  res;
}

}