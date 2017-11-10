#include "stdafx.h"
#include "dmPNGReader.hpp"
#include <array>
#include "dmImage.hpp"

int main()
{
   // dm::VAL::CHECK();
    png::PNGReader reader;
    png::dmImage image;
    reader.Read("D:/tim/7.png", image);
    reader.Write("D:/tim/my.png", image);
    return 0;
}

