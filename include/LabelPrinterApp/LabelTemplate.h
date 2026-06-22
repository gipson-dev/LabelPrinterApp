#pragma once

#include <string>
#include <vector>
#include "LabelElement.h"

enum class MediaTrackingMode
{
    Gap,
    BlackMark,
    Continuous
};

enum class LabelOrientation
{
    Portrait,
    Landscape
};

inline const char* ToString(MediaTrackingMode mode)
{
    switch (mode)
    {
    case MediaTrackingMode::BlackMark:
        return "BlackMark";
    case MediaTrackingMode::Continuous:
        return "Continuous";
    default:
        return "Gap";
    }
}

inline MediaTrackingMode MediaTrackingModeFromString(const std::string& value)
{
    if (value == "BlackMark") return MediaTrackingMode::BlackMark;
    if (value == "Continuous") return MediaTrackingMode::Continuous;
    return MediaTrackingMode::Gap;
}

inline const char* ToString(LabelOrientation orientation)
{
    return orientation == LabelOrientation::Landscape ? "Landscape" : "Portrait";
}

inline LabelOrientation LabelOrientationFromString(const std::string& value)
{
    return value == "Landscape" ? LabelOrientation::Landscape : LabelOrientation::Portrait;
}

struct LabelTemplate
{
    std::string name = "Default Label";

    int labelWidthDots = 609;
    int labelHeightDots = 203;
    int marginLeftDots = 0;
    int marginTopDots = 0;
    int gapDots = 24;
    MediaTrackingMode mediaTrackingMode = MediaTrackingMode::Gap;
    LabelOrientation orientation = LabelOrientation::Portrait;

    std::vector<LabelElement> elements;

    void addElement(const LabelElement& element)
    {
        elements.push_back(element);
    }

    void clear()
    {
        elements.clear();
    }
};
