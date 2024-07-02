#pragma once

#include <iostream>
#include "windows.h"
#include <atlstr.h>
#include <sstream>
#include <iomanip>
#include <locale>
#include <codecvt>
#include <fstream>
#include <objidl.h>
#include <vector>
#include <string>

using namespace std;


void saveToBinaryFile(const char* filename, const char* data, size_t length);
void write_vector_to_binary_file(const string& filename, const std::vector<unsigned char>& data);
std::pair<int, int> find_factors(int n);