#include "core/ZplGenerator.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>

#include "core/BarcodeMetrics.h"

namespace
{
constexpr double kZebraFontVisibleRatio = 0.72;

int zebraFontCellDots(int visualDots)
{
    return std::max(8, static_cast<int>(std::round(visualDots / kZebraFontVisibleRatio)));
}

int alignedBarcodeX(const LabelElement& element, const LabelTemplate& labelTemplate, const std::string& value)
{
    int x = labelTemplate.settings.inchesToDots(element.xInches);
    const int boxWidth = labelTemplate.settings.inchesToDots(element.boxWidthInches);
    const int barcodeWidth = BarcodeMetrics::barcodeWidthDots(element, value);
    if (element.alignment == TextAlignment::Center && boxWidth > barcodeWidth)
    {
        x += (boxWidth - barcodeWidth) / 2;
    }
    else if (element.alignment == TextAlignment::Right && boxWidth > barcodeWidth)
    {
        x += boxWidth - barcodeWidth;
    }
    return x;
}

std::string code128FieldData(const std::string& escapedValue)
{
    std::string data = ">:";
    for (char ch : escapedValue)
    {
        if (ch == '>')
        {
            data += ">>";
        }
        else
        {
            data += ch;
        }
    }
    return data;
}
}

std::string ZplGenerator::generate(const LabelTemplate& labelTemplate, const VariableContext& context)
{
    const PrinterSettings& settings = labelTemplate.settings;
    std::ostringstream zpl;
    zpl << "^XA\n";
    zpl << "^CI28\n";
    zpl << "^PW" << settings.labelWidthDots() << "\n";
    zpl << "^LL" << settings.labelHeightDots() << "\n";
    zpl << "^LH" << settings.marginLeftDots() << "," << settings.marginTopDots() << "\n";
    zpl << "^MN" << mediaCode(settings.mediaSensing) << "\n";
    // Label width/height already describe the stock orientation. Keep fields unrotated so
    // printed output matches the designer canvas for landscape 4 x 2 labels.
    zpl << "^FWN\n";
    zpl << "^MD" << settings.darkness << "\n";
    zpl << "^PR" << settings.speedIps << "\n";

    for (const LabelElement& element : labelTemplate.elements)
    {
        if (!element.doNotPrint)
        {
            zpl << elementZpl(element, labelTemplate, context);
        }
    }

    zpl << "^XZ\n";
    return zpl.str();
}

std::vector<std::string> ZplGenerator::generateSerialRange(const LabelTemplate& labelTemplate, int start, int end, int step, const VariableContext& baseContext)
{
    std::vector<std::string> jobs;
    if (step == 0)
    {
        step = 1;
    }

    if (start <= end)
    {
        for (int serial = start; serial <= end; serial += std::abs(step))
        {
            VariableContext context = baseContext;
            context.serialNumber = serial;
            jobs.push_back(generate(labelTemplate, context));
        }
    }
    else
    {
        for (int serial = start; serial >= end; serial -= std::abs(step))
        {
            VariableContext context = baseContext;
            context.serialNumber = serial;
            jobs.push_back(generate(labelTemplate, context));
        }
    }

    return jobs;
}

std::string ZplGenerator::elementZpl(const LabelElement& element, const LabelTemplate& labelTemplate, const VariableContext& context)
{
    std::string value = VariableResolver::elementValue(element, context);
    switch (element.type)
    {
    case LabelElementType::Code128Barcode:
    case LabelElementType::Code39Barcode:
        return barcodeZpl(element, labelTemplate, value);
    case LabelElementType::QrCode:
        return qrZpl(element, labelTemplate, value);
    case LabelElementType::Line:
    case LabelElementType::Box:
        return shapeZpl(element, labelTemplate);
    default:
        return textZpl(element, labelTemplate, value);
    }
}

std::string ZplGenerator::textZpl(const LabelElement& element, const LabelTemplate& labelTemplate, const std::string& value)
{
    std::ostringstream zpl;
    int x = dots(labelTemplate, element.xInches);
    int boxWidth = dots(labelTemplate, element.boxWidthInches);
    int visualFontHeight = effectiveFontHeight(element, labelTemplate, value);
    int fontHeight = zebraFontCellDots(visualFontHeight);
    int y = dots(labelTemplate, element.yInches);
    if (element.rotation == ElementRotation::Deg0)
    {
        y += visualFontHeight / 4;
    }
    int visualFontWidth = element.autoFit ? std::max(8, element.fontWidthDots * visualFontHeight / std::max(1, element.fontHeightDots)) : element.fontWidthDots;
    int fontWidth = zebraFontCellDots(visualFontWidth);
    char font = element.fontName.empty() ? '0' : element.fontName[0];

    zpl << "^FO" << x << "," << y << "\n";
    zpl << "^A" << font << orientationCode(element.rotation) << "," << fontHeight << "," << fontWidth << "\n";
    if (element.wrap || element.multiLine || element.alignment != TextAlignment::Left)
    {
        zpl << "^FB" << boxWidth << "," << element.maxLines << ",0," << alignmentCode(element.alignment) << ",0\n";
    }
    zpl << "^FH\\^FD" << escapeFieldData(value) << "^FS\n";

    if (element.bold)
    {
        zpl << "^FO" << (x + 2) << "," << y << "\n";
        zpl << "^A" << font << orientationCode(element.rotation) << "," << fontHeight << "," << fontWidth << "\n";
        if (element.wrap || element.multiLine || element.alignment != TextAlignment::Left)
        {
            zpl << "^FB" << boxWidth << "," << element.maxLines << ",0," << alignmentCode(element.alignment) << ",0\n";
        }
        zpl << "^FH\\^FD" << escapeFieldData(value) << "^FS\n";
    }

    if (element.underline)
    {
        int underlineWidth = element.wrap || element.multiLine ? boxWidth : std::max(20, static_cast<int>(value.size()) * visualFontWidth / 2);
        zpl << "^FO" << x << "," << (y + visualFontHeight + 4) << "\n";
        zpl << "^GB" << underlineWidth << ",2,2^FS\n";
    }

    return zpl.str();
}

std::string ZplGenerator::barcodeZpl(const LabelElement& element, const LabelTemplate& labelTemplate, const std::string& value)
{
    std::ostringstream zpl;
    int x = alignedBarcodeX(element, labelTemplate, value);
    int y = dots(labelTemplate, element.yInches);
    zpl << "^FO" << x << "," << y << "\n";
    zpl << "^BY" << element.barcodeModuleWidth << "\n";
    if (element.type == LabelElementType::Code39Barcode)
    {
        zpl << "^B3" << orientationCode(element.rotation) << ",N," << element.barcodeHeightDots << ","
            << (element.humanReadable ? "Y" : "N") << ",N\n";
    }
    else
    {
        zpl << "^BC" << orientationCode(element.rotation) << "," << element.barcodeHeightDots << ","
            << (element.humanReadable ? "Y" : "N") << ",N,N\n";
    }
    const std::string escapedValue = escapeFieldData(value);
    zpl << "^FH\\^FD" << (element.type == LabelElementType::Code128Barcode ? code128FieldData(escapedValue) : escapedValue) << "^FS\n";
    return zpl.str();
}

std::string ZplGenerator::qrZpl(const LabelElement& element, const LabelTemplate& labelTemplate, const std::string& value)
{
    std::ostringstream zpl;
    int x = dots(labelTemplate, element.xInches);
    int y = dots(labelTemplate, element.yInches);
    zpl << "^FO" << x << "," << y << "\n";
    zpl << "^BQN," << element.qrModel << "," << element.qrMagnification << "\n";
    zpl << "^FH\\^FDLA," << escapeFieldData(value) << "^FS\n";
    return zpl.str();
}

std::string ZplGenerator::shapeZpl(const LabelElement& element, const LabelTemplate& labelTemplate)
{
    std::ostringstream zpl;
    const int x = dots(labelTemplate, element.xInches);
    const int y = dots(labelTemplate, element.yInches);
    const int width = std::max(1, dots(labelTemplate, element.boxWidthInches));
    const int height = element.type == LabelElementType::Line
        ? std::max(1, element.fontWidthDots)
        : std::max(1, element.fontHeightDots);
    const int thickness = element.type == LabelElementType::Line
        ? std::max(1, element.fontWidthDots)
        : std::max(1, std::min(element.fontWidthDots, std::min(width, height) / 2));

    zpl << "^FO" << x << "," << y << "\n";
    zpl << "^GB" << width << "," << height << "," << thickness << "^FS\n";
    return zpl.str();
}

std::string ZplGenerator::escapeFieldData(const std::string& input)
{
    std::string output;
    for (unsigned char c : input)
    {
        if (c == '\\' || c == '^' || c == '~' || c < 32)
        {
            output += '\\';
            output += hexByte(c);
        }
        else
        {
            output += static_cast<char>(c);
        }
    }
    return output;
}

std::string ZplGenerator::hexByte(unsigned char value)
{
    std::ostringstream hex;
    hex << std::uppercase << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(value);
    return hex.str();
}

char ZplGenerator::orientationCode(ElementRotation rotation)
{
    switch (rotation)
    {
    case ElementRotation::Deg90: return 'R';
    case ElementRotation::Deg180: return 'I';
    case ElementRotation::Deg270: return 'B';
    default: return 'N';
    }
}

char ZplGenerator::alignmentCode(TextAlignment alignment)
{
    switch (alignment)
    {
    case TextAlignment::Center: return 'C';
    case TextAlignment::Right: return 'R';
    default: return 'L';
    }
}

char ZplGenerator::mediaCode(MediaSensingMode mode)
{
    switch (mode)
    {
    case MediaSensingMode::BlackMark: return 'M';
    case MediaSensingMode::Continuous: return 'N';
    default: return 'Y';
    }
}

int ZplGenerator::dots(const LabelTemplate& labelTemplate, double inches)
{
    return labelTemplate.settings.inchesToDots(inches);
}

int ZplGenerator::effectiveFontHeight(const LabelElement& element, const LabelTemplate& labelTemplate, const std::string& value)
{
    if (!element.autoFit || value.empty() || element.boxWidthInches <= 0)
    {
        return element.fontHeightDots;
    }

    int estimated = static_cast<int>(value.size()) * element.fontWidthDots;
    int boxWidth = labelTemplate.settings.inchesToDots(element.boxWidthInches);
    if (estimated <= boxWidth)
    {
        return element.fontHeightDots;
    }
    return std::max(8, element.fontHeightDots * boxWidth / std::max(1, estimated));
}
