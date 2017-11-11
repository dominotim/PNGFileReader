/******************************************************************************
(C) 2017 Author: Artem Avdoshkin
******************************************************************************/
#ifndef _DM_IMAGE_
#define _DM_IMAGE_

#include <vector>
#include "dmPNGChunks.hpp"

namespace png
{
class dmImage
{
public:
    typedef std::vector<image::Pixel>      Pixels;
    typedef std::vector<Pixels>            PixelsArray;
    dmImage()
    {
    }
    dmImage(const PixelsArray& pixels, const bool is16Bit)
        :m_height(static_cast<uint32>(pixels.size())),
        m_width(static_cast<uint32>(pixels[0].size())),
        m_pixels(pixels),
        m_is16BitPepth(is16Bit)
    {
    }
    const PixelsArray& GetPixels() const
    {
        return m_pixels;
    }
    const uint32 GetWidth() const
    {
        return m_width;
    }

    const uint32 GetHeight() const
    {
        return m_height;
    }

    const bool Is16BitDepth() const
    {
        return m_is16BitPepth;
    }

    const image::Pixel& operator()(size_t i, size_t j) const
    {
        return m_pixels[i][j];
    }

    const std::tuple<uint16, uint16, uint16, uint16> get(size_t i, size_t j) const
    {
        return std::make_tuple(m_pixels[i][j].red,
            m_pixels[i][j].green, m_pixels[i][j].blue, m_pixels[i][j].alfa);
    }

private:
    PixelsArray m_pixels;
    uint32 m_height;
    uint32 m_width;
    bool m_is16BitPepth;
};

}
#endif // _DM_IMAGE_