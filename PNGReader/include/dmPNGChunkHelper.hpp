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
void DecodeTransparencyChunk(
    const bytes& data,
    const data::HeaderChunk& header,
    data::TransParencyChunk& chunk);
void DecodeGammaChunk(const bytes& data, data::GammaChunk& chunk);
void DecodeChromatChunk(const bytes& data, data::PrimaryChromaticitiesChunk& chunk);
void DecodeStandartRGBChunk(const bytes& data, data::StandartRGBChunk& chunk);
bool IsValidChunk(data::ChunkInfo& chunk);
data::DecodedImageInfo CreateFullImageInfo(
    const data::DataChunk& data,
    const data::HeaderChunk& header,
    const data::PaletChunk& palet,
    const data::TransParencyChunk& trans,
    const data::GammaChunk& gamma);
}
}
#endif // _DM_PNG_DECODE_HELPER_