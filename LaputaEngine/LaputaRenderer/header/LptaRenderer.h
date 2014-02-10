#ifndef _LPTARENDERER_H_
#define _LPTARENDERET_H_

#include <memory>
#include <string>
#include <Windows.h>
#include "LptaStatusCodes.h"
#include "LptaDeviceBuilder.h"

class LptaRenderer
{
public:
    LptaRenderer(HINSTANCE hInst);
    ~LptaRenderer(void);

    LPTA_RESULT CreateDeviceBuilder(std::string api, LPTA_DEVICE_BUILDER *builder);

private:
    HINSTANCE hInst;
    HMODULE dllHandle;
};

#endif