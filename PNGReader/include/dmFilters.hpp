/******************************************************************************
(C) 2017 Author: Artem Avdoshkin
******************************************************************************/

#ifndef _DM_FILTERS_
#define _DM_FILTERS_
#include <vector>
namespace png
{
namespace
{
typedef std::uint8_t byte;

int PaethPredictor(const int a, const int b, const int c)
{
    const int p = a + b - c;
    const int pa = std::abs(p - a);
    const int pb = std::abs(p - b);
    const int pc = std::abs(p - c);
    if (pa <= pb && pa <= pc)
        return a;
    else if (pb <= pc)
        return b;
    else return c;
}
}
namespace Unfilter
{
void None(std::vector<std::vector<byte> >& /*scanlines*/, const size_t /*slIdx*/, const size_t /*shift*/)
{
}

void Sub(std::vector<std::vector<byte> >& scanlines, const size_t slIdx, const size_t shift)
{
    for (size_t i = shift + 1; i < scanlines[slIdx].size(); ++i)
    {
        scanlines[slIdx][i] = ((int)scanlines[slIdx][i] + (int)scanlines[slIdx][i - shift]) % 256;
    }
}

void Up(std::vector<std::vector<byte> >& scanlines, const size_t slIdx, const size_t shift)
{
    if (slIdx == 0)
        return;
    for (size_t i = 1; i < scanlines[slIdx].size(); ++i)
    {
        scanlines[slIdx][i] = ((int)scanlines[slIdx][i] + (int)scanlines[slIdx - 1][i]) % 256;
    }
}

void Average(std::vector<std::vector<byte> >& scanlines, const size_t slIdx, const size_t shift)
{
    for (size_t i = 1; i < scanlines[slIdx].size(); ++i)
    {
        scanlines[slIdx][i] = ((int)scanlines[slIdx][i] +  int(std::floor(1.0* (
            ((slIdx == 0) ? 0 : (int)scanlines[slIdx - 1][i] )
            + ((i <= shift) ? 0 : (int)scanlines[slIdx][i - shift])) /  2.))) % 256;
    }
}

void Paeth(std::vector<std::vector<byte> >& scanlines, const size_t slIdx, const size_t shift)
{
    for (size_t i = 1; i < scanlines[slIdx].size(); ++i)
    {
        scanlines[slIdx][i] = ((int)scanlines[slIdx][i]
            + PaethPredictor(
                (i <= shift ? 0 : scanlines[slIdx][i - shift]),
                (slIdx == 0 ? 0 : scanlines[slIdx - 1][i]),
                ((slIdx == 0 || i <= shift) ? 0 : scanlines[slIdx-1][i - shift]))) % 256;
    }
}
} // Unfilter
} // dm
#endif _DM_FILTERS_