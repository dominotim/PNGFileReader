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
class Decompressor
{
public:

    Decompressor() : m_initialized(false), m_readedSize(0){}
    typedef unsigned long ulon;
    template <typename byte>
    std::pair<std::vector<byte>, bool> Decompress(const std::vector<byte>& bytes)
    {
        ulon sizeDataCompressed = bytes.size();
        ulon sizeDataUncompressed = (sizeDataCompressed * 100);
        byte * dataUncompressed = (byte*)malloc(sizeDataUncompressed);
        std::vector<char> data(bytes.begin(), bytes.end());
        Bytef* dataCompressed = (Bytef*)(&data[0]);
        bool isLast = false;
        int z_result = Inflate(dataUncompressed,
            &sizeDataUncompressed, dataCompressed, sizeDataCompressed, isLast);
        
        switch (z_result)
        {
        case Z_OK:
            std::cout << "***** SUCCESS! *****\n";
            break;

        case Z_MEM_ERROR:
            std::cout << "out of memory\n";
            break;

        case Z_BUF_ERROR:
            std::cout << "output buffer wasn't large enough!\n";
            break;
        }
        const size_t currentIdx = sizeDataUncompressed - m_readedSize;
        m_readedSize = sizeDataUncompressed;
        std::vector<byte> res(dataUncompressed, dataUncompressed + currentIdx);
        free(dataUncompressed);
        return std::make_pair(res, isLast);
    }
private:
    int Inflate(
        Bytef *dest,
        uLongf *destLen,
        const Bytef *source,
        uLong sourceLen,
        bool& isEndBlock)
    {
        m_stream.next_in = (Bytef*)source;
        m_stream.avail_in = (uInt)sourceLen;
        m_stream.next_out = dest;
        m_stream.avail_out = (uInt)*destLen;
            m_stream.zalloc = (alloc_func)0;
            m_stream.zfree = (free_func)0;
        int err = Z_OK;
        if (!m_initialized)
        {

            err = inflateInit(&m_stream);
            m_initialized = true;
        }
        if (err != Z_OK) return err;

        err = inflate(&m_stream, Z_SYNC_FLUSH);
        *destLen = m_stream.total_out;
        isEndBlock = err == Z_STREAM_END;
        if (err != Z_STREAM_END) {
            if (err == Z_NEED_DICT || (err == Z_BUF_ERROR && m_stream.avail_in == 0))
                return Z_DATA_ERROR;
            return err;
        }
        err = inflateEnd(&m_stream);
        return err;
    }
    z_stream m_stream;
    size_t m_readedSize;
    bool m_initialized;
};

}  // namespace dm
#endif  _DM_ZLIB_CONVERTER_