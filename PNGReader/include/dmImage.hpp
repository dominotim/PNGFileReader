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
    dmImage(const PixelsArray& pixels )
        :m_height(pixels.size()), m_width(pixels[0].size()), m_pixels(pixels)
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
    data::Pixel operator()(size_t i, size_t j) const
    {
        return m_pixels[i][j];
    }

private:
    PixelsArray m_pixels;
    size_t m_height;
    size_t m_width;
};

void DrawImage(const dmImage& src)
{
    TGAImage im(src.GetWidth(), src.GetHeight(), TGAImage::RGBA);
    for (size_t w = 0; w < src.GetWidth(); ++w)
    {
        for (size_t h = 0; h < src.GetHeight(); ++h)
        {
            im.set(w, h, TGAColor(src(h, w).red, src(h, w).green,
                src(h, w).blue, src(h, w).alfa));
        }
    }
    im.write_tga_file("output.tga");
}

}
#endif // _DM_IMAGE_