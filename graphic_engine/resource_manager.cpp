#include "resource_manager.h"
#include "libpng/png.h"
#include "utils/logger.h"
#include "core/memory_allocator.h"

namespace cubicat {


TexturePtr loadPNG(FILE* file) {
    TexturePtr tex;
    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (!png) {
        return tex;
    }
    if (!file) {
        png_destroy_read_struct(&png, nullptr, nullptr);
        LOGI("open file failed");
        return tex;
    }
    png_infop info = png_create_info_struct(png);
    if (!info) {
        png_destroy_read_struct(&png, nullptr, nullptr);
        return tex;
    }
    if (setjmp(png_jmpbuf(png))) {
        png_destroy_read_struct(&png, &info, nullptr);
        return tex;
    }
    png_init_io(png, file);
    png_read_info(png, info);

    auto width = png_get_image_width(png, info);
    auto height = png_get_image_height(png, info);
    png_byte colorType = png_get_color_type(png, info);
    png_byte bitDepth = png_get_bit_depth(png, info);
    if (bitDepth != 8) {
        assert(false && "not support bit depth not 8 yet");
    }
    // 处理透明度
    if (colorType == PNG_COLOR_TYPE_PALETTE) {
        png_set_palette_to_rgb(png);
    }
    if (colorType == PNG_COLOR_TYPE_GRAY) {
        png_set_expand_gray_1_2_4_to_8(png);
    }
    if (png_get_valid(png, info, PNG_INFO_tRNS)) {
        png_set_tRNS_to_alpha(png);
    }
    if (bitDepth == 16) {
        png_set_strip_16(png);
    }
    // 设置转换后的颜色类型
    png_read_update_info(png, info);
    // 读取图像数据
    bool noAlpha = colorType == PNG_COLOR_TYPE_RGB;
    uint8_t bytePerPixel = noAlpha?3:4;
    uint8_t bitPerPixel = noAlpha?16:32;
    uint8_t* imgData = (uint8_t*)psram_prefered_malloc(width * height * bytePerPixel);
    if (!imgData) {
        png_destroy_read_struct(&png, &info, nullptr);
        return tex;
    }
    uint8_t* rowPointers[height];
    for (int y = 0; y < height; ++y) {
        rowPointers[y] = imgData + y * width * bytePerPixel;
    }
    png_read_image(png, rowPointers);
    if (noAlpha) {
        uint16_t* rgb565 = (uint16_t*)psram_prefered_malloc(width * height * 2);
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                uint8_t* pixel = rowPointers[y] + x * bytePerPixel;
                uint16_t r = pixel[0] >> 3;
                uint16_t g = pixel[1]  >> 2;
                uint16_t b = pixel[2]  >> 3;
                rgb565[y * width + x] = (uint16_t)((r << 11) | (g << 5) | b);
            }
        }
        free(imgData);
        imgData = (uint8_t*)rgb565;
    } else {
        uint32_t* rgba8888 = (uint32_t*)imgData;
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                uint8_t* pixel = rowPointers[y] + x * bytePerPixel;
                uint32_t r = pixel[0];
                uint32_t g = pixel[1];
                uint32_t b = pixel[2];
                uint32_t a = pixel[3];
                rgba8888[y * width + x] = (uint32_t)((r << 24) | (g << 16) | (b << 8) | a);
            }
        }
    }
    png_destroy_read_struct(&png, &info, nullptr);
    return cubicat::Texture::create(width, height, imgData, true, 1, 1, nullptr, bitPerPixel, !noAlpha);
}

TexturePtr ResourceManager::loadTexture(const std::string& name, bool fromFlash) {
    TexturePtr texture;
    auto itr = m_textures.find(name);
    if (itr != m_textures.end()) {
        return itr->second;
    }
    FILE* file = nullptr;
    if (fromFlash)
        file = openFileFlash(name.c_str(), true);
    else
        file = openFileSD(name.c_str(), true);
    if (file) {
        texture = loadPNG(file);
        if (texture)
            m_textures[name] = texture;
        fclose(file);
    } else {
        LOGW("load texture %s fail", name.c_str());
    }
    return texture;
}

void ResourceManager::removeTexture(const std::string& name) {
    auto it = m_textures.find(name);
    if (it != m_textures.end()) {
        if (it->second.use_count() == 1) {
            m_textures.erase(name);
        }
    }
}

TexturePtr ResourceManager::getTexture(const std::string& name) {
    auto it = m_textures.find(name);
    if (it != m_textures.end()) {
        return it->second;
    }
    return TexturePtr(nullptr);
}

} // namespace cubicat