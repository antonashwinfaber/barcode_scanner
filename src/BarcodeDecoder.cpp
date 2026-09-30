#include "BarcodeDecoder.h"
#include <ReadBarcode.h>
#include <BarcodeFormat.h>
#include <DecodeHints.h>
#include <ImageView.h>

#define STB_IMAGE_IMPLEMENTATION
#include "third_party/stb_image.h"

BarcodeDecoder::BarcodeDecoder() {}

DecodeResult BarcodeDecoder::DecodeFromMemory(const uint8_t* data, size_t length) {
    DecodeResult res;
    if (!data || length == 0) return res;

    int width = 0, height = 0, channels = 0;
    // Request 1 channel (grayscale / luminance) for fast barcode processing
    unsigned char* pixels = stbi_load_from_memory(data, static_cast<int>(length), &width, &height, &channels, 1);
    if (!pixels) {
        return res;
    }

    res = DecodeRawLuminance(pixels, width, height);
    stbi_image_free(pixels);
    return res;
}

DecodeResult BarcodeDecoder::DecodeRawLuminance(const uint8_t* buffer, int width, int height) {
    DecodeResult res;
    if (!buffer || width <= 0 || height <= 0) return res;

    ZXing::ImageView image(buffer, width, height, ZXing::ImageFormat::Lum);
    ZXing::ReaderOptions hints;
    hints.setFormats(ZXing::BarcodeFormat::Any);
    hints.setTryHarder(true);
    hints.setTryRotate(true);
    hints.setTryInvert(true);
    hints.setTryDownscale(true);

    auto result = ZXing::ReadBarcode(image, hints);
    if (result.isValid()) {
        res.success = true;
        res.text = result.text();
        res.format = ZXing::ToString(result.format());
    }

    return res;
}
