#ifndef _LPTAD3DUTILS_H_
#define _LPTAD3DUTILS_H_

#include <vector>
#include <string>
#include <d3d9.h>
#include "LptaMaterial.h"

namespace lpta_d3d_utils 
{

inline DWORD FloatToDWORD(float f)
{
    return *reinterpret_cast<DWORD *>(&f);
}

unsigned int GetBitsFor(D3DFORMAT format);
std::string GetTitleFor(D3DDEVTYPE deviceType);

std::wstring ToWChar(const std::string &str);

struct DisplayFormatComparator
{
    bool operator()(const D3DDISPLAYMODE &first, const D3DDISPLAYMODE &second)
    {
        if (first.Width != second.Width) {
            return first.Width < second.Width;
        }
        else if (first.Height != second.Height) {
            return first.Height < second.Height;
        }
        else {
            return first.RefreshRate < second.RefreshRate;
        }
    }
};

inline D3DMATERIAL9 ToDXMaterial(const lpta::LptaMaterial &material)
{
    D3DMATERIAL9 dxMat = {
        material.GetDiffuse().GetRed(), material.GetDiffuse().GetGreen(),
        material.GetDiffuse().GetBlue(), material.GetDiffuse().GetAlpha(),

        material.GetAmbient().GetRed(), material.GetAmbient().GetGreen(),
        material.GetAmbient().GetBlue(), material.GetAmbient().GetAlpha(),

        material.GetSpecular().GetRed(), material.GetSpecular().GetGreen(),
        material.GetSpecular().GetBlue(), material.GetSpecular().GetAlpha(),

        material.GetEmissive().GetRed(), material.GetEmissive().GetGreen(),
        material.GetEmissive().GetBlue(), material.GetEmissive().GetAlpha(),

        material.GetSpecularPower()
    };
    return dxMat;
}

}

#endif