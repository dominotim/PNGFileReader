/******************************************************************************
(C) 2017 Author: Artem Avdoshkin
******************************************************************************/
#ifndef _DM_ZLIB_CONVERTER_
#define _DM_ZLIB_CONVERTER_
#include "zlib.h"
#include <vector>
#include <iostream>
namespace dm
{
template <typename byte>
std::vector<byte> Decompress(const std::vector<byte>& bytes)
{
    typedef unsigned long ulon;
    ulon sizeDataCompressed = bytes.size();
    ulon sizeDataUncompressed = (sizeDataCompressed * 100);
    byte * dataUncompressed = (byte*)malloc(sizeDataUncompressed);
    std::vector<char> data(bytes.begin(), bytes.end());
    Bytef* dataCompressed = (Bytef*)(&data[0]);
    int z_result = uncompress(dataUncompressed,
        &sizeDataUncompressed, dataCompressed, sizeDataCompressed);

    switch (z_result)
    {
    case Z_OK:
        std::cout << "***** SUCCESS! *****\n";
        break;

    case Z_MEM_ERROR:
        std::cout << "out of memory\n";
        return std::vector<byte>();

    case Z_BUF_ERROR:
        std::cout << "output buffer wasn't large enough!\n";
        return std::vector<byte>();
    }
    std::vector<byte> res(dataUncompressed, dataUncompressed + sizeDataUncompressed);
    free(dataUncompressed);
    return res;
}

}  // namespace dm
#endif  _DM_ZLIB_CONVERTER_