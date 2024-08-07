#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include "Windows.h"
#include "png.h"


std::vector<unsigned char> read_png_from_memory(unsigned char* data, size_t data_size, int& width, int& height);
char* create_png_from_rgb(const std::vector<unsigned char>& image_data, int width, int height, size_t& png_size);
void xorPng(std::vector<unsigned char>& imageData);

int PngParseTest();

