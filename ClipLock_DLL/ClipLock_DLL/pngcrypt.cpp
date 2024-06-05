#define _CRT_SECURE_NO_WARNINGS

#include "pngcrypt.h"

// Struct to hold the PNG data in memory
struct PngBuffer {
    std::vector<char> data;
};

/* 메모리에서 png 데이터를 읽고 rgba 값을 vector로 반환하는 함수 */
void read_data_from_memory(png_structp png_ptr, png_bytep data, png_size_t length) {
    png_bytep* p = (png_bytep*)png_get_io_ptr(png_ptr);
    memcpy(data, *p, length);
    *p += length;
}

std::vector<unsigned char> read_png_from_memory(unsigned char* data, size_t data_size, int& width, int& height) {
    if (data == nullptr || data_size == 0) {
        throw std::runtime_error("Invalid PNG data");
    }

    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (!png) {
        throw std::runtime_error("Failed to create png read struct");
    }

    png_infop info = png_create_info_struct(png);
    if (!info) {
        png_destroy_read_struct(&png, nullptr, nullptr);
        throw std::runtime_error("Failed to create png info struct");
    }

    if (setjmp(png_jmpbuf(png))) {
        png_destroy_read_struct(&png, &info, nullptr);
        throw std::runtime_error("Error during png creation");
    }

    png_bytep data_ptr = data;
    png_set_read_fn(png, &data_ptr, read_data_from_memory);
    png_read_info(png, info);

    width = png_get_image_width(png, info);
    height = png_get_image_height(png, info);
    png_byte color_type = png_get_color_type(png, info);
    png_byte bit_depth = png_get_bit_depth(png, info);

    if (bit_depth == 16)
        png_set_strip_16(png);
    if (color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_palette_to_rgb(png);
    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
        png_set_expand_gray_1_2_4_to_8(png);
    if (png_get_valid(png, info, PNG_INFO_tRNS))
        png_set_tRNS_to_alpha(png);
    if (color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_filler(png, 0xFF, PNG_FILLER_AFTER);
    if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
        png_set_gray_to_rgb(png);

    png_read_update_info(png, info);

    std::vector<unsigned char> image_data(png_get_rowbytes(png, info) * height);
    std::vector<png_bytep> row_pointers(height);

    for (int y = 0; y < height; y++) {
        row_pointers[y] = &image_data[y * png_get_rowbytes(png, info)];
    }

    png_read_image(png, row_pointers.data());

    png_destroy_read_struct(&png, &info, nullptr);

    return image_data;
}

// Custom write function to write PNG data to memory
void write_data_to_memory(png_structp png_ptr, png_bytep data, png_size_t length) {
    PngBuffer* pngBuffer = reinterpret_cast<PngBuffer*>(png_get_io_ptr(png_ptr));
    pngBuffer->data.insert(pngBuffer->data.end(), data, data + length);
}

// Function to convert RGB data to PNG and return it as char*
char* create_png_from_rgb(const std::vector<unsigned char>& image_data, int width, int height, size_t& png_size) {
    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (!png) {
        throw std::runtime_error("Failed to create PNG write struct");
    }

    png_infop info = png_create_info_struct(png);
    if (!info) {
        png_destroy_write_struct(&png, nullptr);
        throw std::runtime_error("Failed to create PNG info struct");
    }

    if (setjmp(png_jmpbuf(png))) {
        png_destroy_write_struct(&png, &info);
        throw std::runtime_error("Error during PNG creation");
    }

    PngBuffer pngBuffer;
    png_set_write_fn(png, &pngBuffer, write_data_to_memory, nullptr);

    // Write header (8 bit depth, RGB)
    png_set_IHDR(png, info, width, height, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
        PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_write_info(png, info);

    // Write image data
    std::vector<png_bytep> row_pointers(height);
    for (int y = 0; y < height; ++y) {
        row_pointers[y] = (png_bytep)&image_data[y * width * 3];
    }
    png_write_image(png, row_pointers.data());

    // End write
    png_write_end(png, nullptr);

    png_destroy_write_struct(&png, &info);

    // Allocate memory for the char* output
    png_size = pngBuffer.data.size();
    char* output = new char[png_size];
    std::copy(pngBuffer.data.begin(), pngBuffer.data.end(), output);

    return output;
}

void xorPng(std::vector<unsigned char> &imageData)
{
    for (int i = 0; i < imageData.size(); i++) {
        imageData[i] ^= 0xAA;
    }
}

// 위 함수들을 사용하는 방법입니다.
/*
int PngParseTest()
{
    try {
        // 여기에 PNG 데이터가 있는 메모리 버퍼와 크기를 설정합니다.
        unsigned char* png_data;  // 실제 PNG 데이터로 초기화합니다.
        size_t png_data_size;     // PNG 데이터의 크기로 초기화합니다.

        // 예제 PNG 데이터를 설정합니다. 이 부분은 실제로 PNG 데이터를 메모리로 읽어오는 코드로 대체되어야 합니다.
        // 예시: png_data = ...;
        // 예시: png_data_size = ...;

        int width, height;
        std::vector<unsigned char> image_data = read_png_from_memory(png_data, png_data_size, width, height);

        // 이미지 데이터 처리
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                int index = (y * width + x) * 4; // 4 channels (RGBA)
                unsigned char r = image_data[index];
                unsigned char g = image_data[index + 1];
                unsigned char b = image_data[index + 2];
                unsigned char a = image_data[index + 3];
                // r, g, b, a 값을 처리
            }
        }
        xorPng(image_data);
        std::cout << "Image processed successfully." << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }


    return EXIT_SUCCESS;

}
*/

