#include "stdafx.h"
#include <string>
#include <fstream>
#include <vector>
#include <iterator>
#include <iostream>
#include <tuple>
#include <windows.h>
#include "dmZlibConverter.hpp"
#include "dmFilters.hpp"
#include "tgaimage.hpp"
#include "dmPNGReader.hpp"

int main()
{
    dm::PNGReader reader;
    reader.Read("D:/tim/3.png");
    return 0;
}

