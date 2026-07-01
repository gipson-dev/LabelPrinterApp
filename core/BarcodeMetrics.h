#pragma once

#include <algorithm>
#include <cstddef>
#include <string>

#include "core/LabelElement.h"

namespace BarcodeMetrics
{
inline int code128ModuleCount(std::size_t length)
{
    return static_cast<int>(35 + 11 * length);
}

inline int code39ModuleCount(std::size_t length)
{
    return static_cast<int>(std::max<std::size_t>(1, length + 2) * 16);
}

inline int moduleCount(const LabelElement& element, const std::string& value)
{
    const std::size_t length = std::max<std::size_t>(1, value.size());
    if (element.type == LabelElementType::Code39Barcode)
    {
        return code39ModuleCount(length);
    }
    return code128ModuleCount(length);
}

inline int barcodeWidthDots(const LabelElement& element, const std::string& value)
{
    return std::max(1, moduleCount(element, value) * std::max(1, element.barcodeModuleWidth));
}
}
