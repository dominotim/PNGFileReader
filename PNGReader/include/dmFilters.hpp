/******************************************************************************
(C) 2017 Author: Artem Avdoshkin
******************************************************************************/

#ifndef _DM_FILTERS_
#define _DM_FILTERS_
#include <vector>
namespace png
{

namespace Unfilter
{
void None(std::vector<std::vector<std::uint8_t> >& scanlines, const size_t slIdx, const size_t shift);

void Sub(std::vector<std::vector<std::uint8_t> >& scanlines, const size_t slIdx, const size_t shift);

void Up(std::vector<std::vector<std::uint8_t> >& scanlines, const size_t slIdx, const size_t shift);

void Average(std::vector<std::vector<std::uint8_t> >& scanlines, const size_t slIdx, const size_t shift);

void Paeth(std::vector<std::vector<std::uint8_t> >& scanlines, const size_t slIdx, const size_t shift);
} // Unfilter
} // dm
#endif _DM_FILTERS_