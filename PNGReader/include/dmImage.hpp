#ifndef _DM_IMAGE_
#define _DM_IMAGE_

#include <vector>
#include "dmPNGChunks.hpp"
#include "tgaimage.hpp"
namespace png
{
class dmImage
{
public:
    typedef std::vector<png::image::Pixel> Pixels;
    typedef std::vector<Pixels>            PixelsArray;
    dmImage()
    {
    }
    dmImage(const PixelsArray& pixels, const bool is16Bit)
        :m_height(pixels.size()),
        m_width(pixels[0].size()),
        m_pixels(pixels),
        m_is16BitPepth(is16Bit),
        m_gamma(1)
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

    void SetGamma(const double gamma)
    {
        m_gamma = gamma;
    }

    const double GetGamma() const
    {
        return m_gamma;
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
    size_t m_height;
    size_t m_width;
    bool m_is16BitPepth;
    double m_gamma;
};

}
#endif // _DM_IMAGE_