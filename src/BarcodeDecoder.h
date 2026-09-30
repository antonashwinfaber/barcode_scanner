#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <cstddef>

struct DecodeResult {
    bool success = false;
    std::string text;
    std::string format;
};

class BarcodeDecoder {
public:
    BarcodeDecoder();

    // Decodes an encoded image file buffer (JPEG, PNG, BMP, etc.) from memory
    DecodeResult DecodeFromMemory(const uint8_t* data, size_t length);

    // Decodes directly from raw grayscale/luminance pixels
    DecodeResult DecodeRawLuminance(const uint8_t* buffer, int width, int height);
};
