#pragma once

#include <cmath>

namespace LabelUnits
{
inline double inchesToDots(double inches, int dpi)
{
    return inches * static_cast<double>(dpi);
}

inline double dotsToInches(double dots, int dpi)
{
    return dpi > 0 ? dots / static_cast<double>(dpi) : 0.0;
}

inline double mmToDots(double mm, int dpi)
{
    return mm * static_cast<double>(dpi) / 25.4;
}

inline double dotsToMm(double dots, int dpi)
{
    return dpi > 0 ? dots * 25.4 / static_cast<double>(dpi) : 0.0;
}

inline int roundDots(double dots)
{
    return static_cast<int>(std::lround(dots));
}
}

