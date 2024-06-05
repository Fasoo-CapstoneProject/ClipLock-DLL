#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include "Windows.h"
#include "png.h"

std::vector<unsigned char> read_png_file(const char* filename, int& width, int& height);

int PngParseTest();