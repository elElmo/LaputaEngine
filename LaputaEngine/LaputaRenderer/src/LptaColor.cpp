#include "LptaColor.h"

namespace lpta
{

const float LptaColor::DEFAULT_CHANNEL_VALUE = 1.0f;
const float LptaColor::DEFAULT_ALPHA = 1.0f;

LptaColor::LptaColor(void) : LptaColor(DEFAULT_CHANNEL_VALUE, DEFAULT_CHANNEL_VALUE, 
    DEFAULT_CHANNEL_VALUE, DEFAULT_ALPHA)
{}

LptaColor::LptaColor(float red, float green, float blue) : LptaColor(red, green, blue, DEFAULT_ALPHA)
{}

LptaColor::LptaColor(float red, float green, float blue, float alpha)
{
    channels[RED] = red;
    channels[GREEN] = green;
    channels[BLUE] = blue;
    channels[ALPHA] = alpha;
}

LptaColor::~LptaColor(void)
{}

bool LptaColor::operator ==(const LptaColor &other) const
{
    return GetAlpha() == other.GetAlpha() &&
        GetRed() == other.GetRed() &&
        GetGreen() == other.GetGreen() &&
        GetBlue() == other.GetBlue();
}

}