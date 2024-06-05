#define _CRT_SECURE_NO_WARNINGS

#include "pngcrypt.h"

// png 파일을 
std::vector<unsigned char> read_png_file(const char* filename, int& width, int& height)
{
    FILE* fp = fopen(filename, "rb");
    if (!fp) {
        throw std::runtime_error("Failed to open file");
    }

    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (!png) {
        fclose(fp);
        throw std::runtime_error("Failed to create png read struct");
    }

    png_infop info = png_create_info_struct(png);
    if (!info) {
        png_destroy_read_struct(&png, nullptr, nullptr);
        fclose(fp);
        throw std::runtime_error("Failed to create png info struct");
    }

    if (setjmp(png_jmpbuf(png))) {
        png_destroy_read_struct(&png, &info, nullptr);
        fclose(fp);
        throw std::runtime_error("Error during png creation");
    }

    png_init_io(png, fp);
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

    fclose(fp);
    png_destroy_read_struct(&png, &info, nullptr);

    return image_data;
}

int PngParseTest()
{
    char buf[256];
    bool testFlag = false;

    try {
        int width, height;

        // png 경로 설정이 필요
        std::vector<unsigned char> image_data = read_png_file("red.png", width, height);

        // Process the image_data as needed
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                int index = (y * width + x) * 4; // 4 channels (RGBA)
                unsigned char r = image_data[index];
                unsigned char g = image_data[index + 1];
                unsigned char b = image_data[index + 2];
                unsigned char a = image_data[index + 3];
                // Do something with r, g, b, a

                if (testFlag == false) {
                    testFlag = true;
                    sprintf_s(buf, sizeof(buf), "rgba data : 0x%x%x%x%x", (int)r, (int)g, (int)b, (int)a);
                    OutputDebugStringA(buf);
                } 
            }
        }
        OutputDebugStringA("png reading test success");
    }
    catch (const std::exception& e) {
        OutputDebugStringA("png reading test failed");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;

}


