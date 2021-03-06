/******************************************************************************
(C) 2017 Author: Artem Avdoshkin
******************************************************************************/
#ifndef _DM_ZLIB_CONVERTER_
#define _DM_ZLIB_CONVERTER_
#include "zlib.h"
#include <vector>
#include <iostream>
namespace png
{
class Decompressor
{
public:
    Decompressor() : m_initialized(false), m_readBefore(0){}
    typedef unsigned long ulon;
    typedef std::uint8_t  byte;

    std::vector<byte> Decompress(const std::vector<byte>& bytes)
    {
        const ulon sizeDataCompressed = static_cast<ulon>(bytes.size());
        const Bytef* dataCompressed   = (Bytef*)(&bytes[0]);
        ulon sizeDataUncompressed     = (sizeDataCompressed * 100);
        byte * dataUncompressed       = (byte*)malloc(sizeDataUncompressed);

        bool isLast = false;
        const int code = Inflate(dataUncompressed,
            &sizeDataUncompressed, dataCompressed, sizeDataCompressed, isLast);
        switch (code)
        {
            case Z_MEM_ERROR: throw "e"; break;
            case Z_BUF_ERROR: throw "e"; break;
        }

        const size_t size = sizeDataUncompressed - m_readBefore;
        m_readBefore = sizeDataUncompressed;
        m_rawData.insert(m_rawData.end(), dataUncompressed, dataUncompressed + size);
        free(dataUncompressed);
        const std::vector<byte> res = isLast ? m_rawData : std::vector<byte>();
        if (isLast) { m_initialized = false; m_readBefore = 0; }
        return res;
    }

    std::vector<byte> Compress(const std::vector<byte>& bytes)
    {
        z_stream defstream;
        defstream.zalloc = Z_NULL;
        defstream.zfree = Z_NULL;
        defstream.opaque = Z_NULL;
        Bytef* data = (Bytef*)(&bytes[0]);
        byte * compressed = (byte*)malloc(bytes.size());
        // setup "a" as the input and "b" as the compressed output
        defstream.avail_in = (uInt)bytes.size() + 1; // size of input, string + terminator
        defstream.next_in = data; // input char array
        defstream.avail_out = (uInt)bytes.size(); // size of output
        defstream.next_out = (Bytef *)compressed; // output char array

                                         // the actual compression work.
        deflateInit(&defstream, Z_BEST_COMPRESSION);
        deflate(&defstream, Z_FINISH);
        deflateEnd(&defstream);
        return std::vector<byte>(compressed, compressed + defstream.total_out);
    }

private:
    int Inflate(
        Bytef *inflated,
        uLongf *inflatedBufSize,
        const Bytef *source,
        const uLong sourceBufSize,
        bool& isFinalBlock)
    {
        m_stream.next_in   = (Bytef*)source;
        m_stream.avail_in  = (uInt)sourceBufSize;
        m_stream.next_out  = inflated;
        m_stream.avail_out = (uInt)*inflatedBufSize;
        m_stream.zalloc    = (alloc_func)0;
        m_stream.zfree     = (free_func)0;

        int resultCode = Z_OK;
        if (!m_initialized)
        {
            resultCode = inflateInit(&m_stream);
            m_initialized = true;
        }
        if (resultCode != Z_OK) return resultCode;
        resultCode = inflate(&m_stream, Z_SYNC_FLUSH);
        *inflatedBufSize = m_stream.total_out;
        isFinalBlock = resultCode == Z_STREAM_END;

        if (!isFinalBlock)
            return (resultCode == Z_NEED_DICT
                || (resultCode == Z_BUF_ERROR && m_stream.avail_in == 0))
                ? Z_DATA_ERROR : resultCode;
        return inflateEnd(&m_stream);
    }

private:
    z_stream          m_stream;
    size_t            m_readBefore;
    std::vector<byte> m_rawData;
    bool              m_initialized;
};

}  // namespace dm
#endif  _DM_ZLIB_CONVERTER_