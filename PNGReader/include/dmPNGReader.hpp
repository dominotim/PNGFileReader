/******************************************************************************
(C) 2017 Author: Artem Avdoshkin
******************************************************************************/
#ifndef _DM_PNG_READER_
#define _DM_PNG_READER_
#include "dmPNGChunks.hpp"
#include <fstream>

namespace png
{
class dmImage;

class PNGReader
{
public:
    PNGReader();
    void Read(const std::string& path, png::dmImage& image);
    void Write(const std::string& path, const png::dmImage& image);
private:
    bool Read(png::dmImage& image);
    bool CheckHeader();
    void Init(std::ifstream& file, std::ifstream::pos_type pos);
    chunks::ChunkInfo GetNetxChunk();

    std::vector<byte> m_bytes;
    size_t m_pos;
};
}
#endif //_DM_PNG_READER_