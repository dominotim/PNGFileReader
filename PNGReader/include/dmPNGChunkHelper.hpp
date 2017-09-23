#ifndef _DM_PNG_DECODE_HELPER_
#define _DM_PNG_DECODE_HELPER_
#include "dmDataStructure.hpp"
#include "dmZlibConverter.hpp"

namespace dm
{
namespace chunkHelper
{
uint32 GetInt32ValueAndIncIdx(const bytes& data, size_t& idx);
void DecodeHeaderChunk(const bytes& data, data::HeaderChunk& chunk);
void DecodeDataChunk(const bytes& data, const data::HeaderChunk& header, data::DataChunk& chunk, Decompressor& decoder);
void DecodePaletChunk(const bytes& data, data::PaletChunk& chunk);
bool IsValidChunk(data::ChunkInfo& chunk);
data::DecodedImageInfo CreateFullImageInfo(const data::DataChunk& data,
    const data::HeaderChunk& header,const data::PaletChunk& palet);
}
}
#endif // _DM_PNG_DECODE_HELPER_