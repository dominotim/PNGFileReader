#ifndef _DM_IMAGE_
#define _DM_IMAGE_

#include <vector>
#include "dmDataStructure.hpp"
#include "tgaimage.hpp"
#include "dmPNGChunkHelper.hpp"

namespace dm
{
class dmImage
{
public:
    typedef std::vector<dm::data::Pixel>    Pixels;
    typedef std::vector<Pixels>         PixelsArray;
    dmImage(const PixelsArray& pixels, const bool is16Bit)
        :m_height(pixels.size()),
        m_width(pixels[0].size()),
        m_pixels(pixels),
        m_is16BitPepth(is16Bit)
    {
    }
    const PixelsArray& GetPixels() const
    {
        return m_pixels;
    }
    const size_t GetWidth() const
    {
        return m_width;
    }

    const size_t GetHeight() const
    {
        return m_height;
    }

    const bool Is16BitDepth() const
    {
        return m_is16BitPepth;
    }

    data::Pixel operator()(size_t i, size_t j) const
    {
        return m_pixels[i][j];
    }

private:
    PixelsArray m_pixels;
    size_t m_height;
    size_t m_width;
    bool m_is16BitPepth;
};

void DrawImage(const dmImage& src)
{
    TGAImage im(src.GetWidth(), src.GetHeight(), TGAImage::RGBA);
    for (size_t w = 0; w < src.GetWidth(); ++w)
    {
        for (size_t h = 0; h < src.GetHeight(); ++h)
        {
            byte r = src.Is16BitDepth() ? 255 * (1. * src(h, w).red / 0xffffu)   : src(h, w).red;
            byte g = src.Is16BitDepth() ? 255 * (1. * src(h, w).green / 0xffffu) : src(h, w).green;
            byte b = src.Is16BitDepth() ? 255 * (1. * src(h, w).blue / 0xffffu)  : src(h, w).blue;
            byte a = src.Is16BitDepth() ? 255 * (1. * src(h, w).alfa / 0xffffu)  : src(h, w).alfa;
            im.set(w, h, TGAColor(r, g, b, a));
        }
    }
    im.write_tga_file("output.tga");
}

}
#endif // _DM_IMAGE_