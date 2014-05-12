#include <d3d9.h>
#include "FreeImage.h"
#include "LptaD3DColorUtils.h"
#include "resources/errors/TextureD3DFailure.h"
#include "resources/errors/TextureFileNotFound.h"
#include "resources/LptaD3DTexture.h"
using namespace lpta_d3d_utils;

namespace lpta_d3d
{
// D3DLoadTextureFile

inline void CopyFImageToD3DRect32Bit(FIBITMAP &image, 
    D3DLOCKED_RECT d3dLockedRect);

inline bool EqualRGB(const lpta::LptaColor &color, const Color32Bit &colorStruct)
{
    return color.GetRed() == colorStruct.r &&
        color.GetGreen() == colorStruct.g &&
        color.GetBlue() == colorStruct.b;
}
inline bool LesserAlpha(const lpta::LptaColor &color, const Color32Bit &colorStruct) {
    return (MAX_ARGB * color.GetAlpha()) < colorStruct.a;
}
inline bool LesserAlpha(float alpha, const Color32Bit &colorStruct) {
    return (MAX_ARGB * alpha) < colorStruct.a;
}

LptaD3DTexture::LptaD3DTexture(LPDIRECT3DDEVICE9 d3ddev, ID id, const string &filename, 
    float alpha, const COLOR_KEYS &colorKeys) : 
    LptaTexture(id, filename, (DATA)D3DLoadTextureFile(d3ddev, filename, alpha != 1.0f), alpha, colorKeys)
{
}

LptaD3DTexture::LptaD3DTexture(ID id, const string &name, LPDIRECT3DTEXTURE9 data, 
    float alpha, const COLOR_KEYS &colorKeys) :
    LptaTexture(id, name, (DATA)data, alpha, colorKeys)
{
}

LptaD3DTexture::~LptaD3DTexture(void)
{
}

struct FileNotFound {};
#define TO_4_BYTE 4
LPDIRECT3DTEXTURE9 LptaD3DTexture::D3DLoadTextureFile(LPDIRECT3DDEVICE9 d3ddev, 
    const string &filename, bool alpha)
{
    try {
        FIBITMAP *image = FreeImage_Load(FreeImage_GetFileType(filename.c_str(), 0), filename.c_str());
        if (!image) {
            throw FileNotFound();
        }
        D3DFORMAT format = D3DFMT_A8R8G8B8;
        LPDIRECT3DTEXTURE9 textureData = nullptr;
        D3DLOCKED_RECT d3dLockedRect;
        HRESULT result = d3ddev->CreateTexture(
            FreeImage_GetWidth(image), FreeImage_GetHeight(image),
            1, 0,
            format,
            D3DPOOL_MANAGED,
            &textureData,
            0
        );
        if (FAILED(result)) {
            throw TextureD3DFailure("could not allocate memory for texture");
        }
        result = textureData->LockRect(0, &d3dLockedRect, nullptr, 0);
        if (FAILED(result)) {
            textureData->Release();
            throw TextureD3DFailure("could not lock image buffer for copy");
        }

        CopyFImageToD3DRect32Bit(*image, d3dLockedRect);
        textureData->UnlockRect(0);
        FreeImage_Unload(image);
        return textureData;
    }
    catch (FileNotFound) {
        throw TextureFileNotFound(filename);
    }
}

void CopyFImageToD3DRect32Bit(FIBITMAP &image, 
    D3DLOCKED_RECT d3dLockedRect)
{
    // todo confirm 32 bit 
    bool hasAlphaChannel = (FreeImage_IsTransparent(&image) == TRUE);
    unsigned int colorsPerLine = d3dLockedRect.Pitch >> 2;
    Color32Bit *buffer = static_cast<Color32Bit*>(d3dLockedRect.pBits);
    for (unsigned int y = 0; y < FreeImage_GetHeight(&image); ++y) {
        for (unsigned int x = 0; x < FreeImage_GetWidth(&image); ++x) {
            RGBQUAD rgb;
            FreeImage_GetPixelColor(&image, x, y, &rgb);
            Color32Bit color;
            color.a = hasAlphaChannel? rgb.rgbReserved : 0xFF,
            color.a = 0xFF;
            color.r = rgb.rgbRed;
            color.g = rgb.rgbGreen;
            color.b = rgb.rgbBlue;
            buffer[(y * colorsPerLine) + x] = color;
        }
    }
}

void LptaD3DTexture::SetAlphaKey(const lpta::LptaColor &colorKey) const
{
    LPDIRECT3DTEXTURE9 texture = static_cast<LPDIRECT3DTEXTURE9>(this->GetData());
    D3DSURFACE_DESC d3dDesc;
    D3DLOCKED_RECT d3dLockedRect;
    texture->GetLevelDesc(0, &d3dDesc);
    if (FAILED(texture->LockRect(0, &d3dLockedRect, nullptr, 0))) {
        throw TextureD3DFailure("could not lock image buffer to set color keys");
    }
    Color32Bit *buffer = static_cast<Color32Bit*>(d3dLockedRect.pBits);
    for (unsigned int y = 0; y < d3dDesc.Height; ++y) {
        for (unsigned int x = 0; x < d3dDesc.Width; ++x) {
            unsigned int pixelIndex = (y * d3dDesc.Width) + x;
            Color32Bit pixelColor = buffer[pixelIndex];
            if (EqualRGB(colorKey, pixelColor) && LesserAlpha(colorKey, pixelColor)) {
                pixelColor.a = (unsigned char)(MAX_ARGB * colorKey.GetAlpha());
                buffer[pixelIndex] = pixelColor;
            }
        }
    }
    texture->UnlockRect(0);
}

void LptaD3DTexture::SetTransparency(float alpha) const
{
    LPDIRECT3DTEXTURE9 texture = static_cast<LPDIRECT3DTEXTURE9>(this->GetData());
    D3DSURFACE_DESC d3dDesc;
    D3DLOCKED_RECT d3dLockedRect;
    texture->GetLevelDesc(0, &d3dDesc);
    if (FAILED(texture->LockRect(0, &d3dLockedRect, nullptr, 0))) {
        throw TextureD3DFailure("could not lock image buffer to set alpha");
    }
    Color32Bit *buffer = static_cast<Color32Bit*>(d3dLockedRect.pBits);
    for (unsigned int y = 0; y < d3dDesc.Height; ++y) {
        for (unsigned int x = 0; x < d3dDesc.Width; ++x) {
            unsigned int pixelIndex = (y * d3dDesc.Width) + x;
            Color32Bit pixelColor = buffer[pixelIndex];
            if (LesserAlpha(alpha, pixelColor)) {
                pixelColor.a = (unsigned char)(MAX_ARGB * alpha);
                buffer[pixelIndex] = pixelColor;
            }
        }
    }
    texture->UnlockRect(0);
}

}