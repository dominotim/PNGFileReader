/******************************************************************************
(C) 2017 Author: Artem Avdoshkin
******************************************************************************/
#include "dmDataStructure.hpp"
#include <fstream>

#ifndef _DM_PNG_READER_
#define _DM_PNG_READER_
namespace dm
{
class PNGReader
{
public:
    PNGReader();
    void Read(const std::string& file_path);
private:
    bool Parse();
    bool CheckHeader();
    void Init(std::ifstream& file, std::ifstream::pos_type pos);
    data::ChunkInfo NextChunk();
    std::vector<byte> m_bytes;
    size_t m_position;
};
}
#endif //_DM_PNG_READER_