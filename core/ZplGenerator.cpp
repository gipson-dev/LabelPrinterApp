#include "core/ZplGenerator.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>

#include "core/BarcodeMetrics.h"
#include "core/LabelUnits.h"

namespace
{
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

int roundDot(double value)
{
    return LabelUnits::roundDots(value);
}

int printableEdgeInsetDots(int dpi)
{
    return std::max(4, LabelUnits::roundDots(LabelUnits::inchesToDots(0.04, dpi)));
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

    const ResolvedLabelLayout layout = LabelLayoutEngine::resolve(labelTemplate, context);
    for (const ResolvedLabelElement& element : layout.elements)
    {
        if (!element.element.doNotPrint)
        {
            zpl << elementZpl(element, labelTemplate);
        }
    }

    zpl << "^XZ\n";
    return zpl.str();
}

std::string ZplGenerator::generateDebugReport(const LabelTemplate& labelTemplate, const VariableContext& context)
{
    const PrinterSettings& settings = labelTemplate.settings;
    const ResolvedLabelLayout layout = LabelLayoutEngine::resolve(labelTemplate, context);

    std::ostringstream report;
    report << "Label\n";
    report << "dpi=" << layout.dpi << "\n";
    report << "widthDots=" << roundDot(layout.labelWidthDots) << "\n";
    report << "heightDots=" << roundDot(layout.labelHeightDots) << "\n";
    report << "ZPL globals: ^PW" << settings.labelWidthDots()
           << " ^LL" << settings.labelHeightDots()
           << " ^LH" << settings.marginLeftDots() << "," << settings.marginTopDots()
           << " ^FWN\n\n";

    for (const ResolvedLabelElement& resolved : layout.elements)
    {
        if (resolved.element.type != LabelElementType::Text)
        {
            continue;
        }

        report << "Element: " << (resolved.element.name.empty() ? resolved.element.id : resolved.element.name) << "\n";
        report << "id: " << resolved.element.id << "\n";
        report << "Document:\n";
        report << "xDots=" << roundDot(resolved.contentBoundsDots.x) << "\n";
        report << "yDots=" << roundDot(resolved.contentBoundsDots.y) << "\n";
        report << "widthDots=" << roundDot(resolved.contentBoundsDots.width) << "\n";
        report << "heightDots=" << roundDot(resolved.contentBoundsDots.height) << "\n";
        report << "Font:\n";
        report << "heightDots=" << resolved.fontHeightDots << "\n";
        report << "widthDots=" << resolved.fontWidthDots << "\n";
        report << "Lines:\n";
        for (std::size_t i = 0; i < resolved.textLines.size(); ++i)
        {
            report << i << ": " << resolved.textLines[i] << "\n";
        }
        report << "Generated ZPL:\n";
        report << textZpl(resolved) << "\n";
    }

    report << "Full ZPL:\n";
    report << generate(labelTemplate, context);
    return report.str();
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

std::string ZplGenerator::elementZpl(const ResolvedLabelElement& resolved, const LabelTemplate& labelTemplate)
{
    switch (resolved.element.type)
    {
    case LabelElementType::Code128Barcode:
    case LabelElementType::Code39Barcode:
        return barcodeZpl(resolved);
    case LabelElementType::QrCode:
        return qrZpl(resolved);
    case LabelElementType::Line:
    case LabelElementType::Box:
        return shapeZpl(resolved, labelTemplate);
    default:
        return textZpl(resolved);
    }
}

std::string ZplGenerator::textZpl(const ResolvedLabelElement& resolved)
{
    const LabelElement& element = resolved.element;
    std::ostringstream zpl;
    const int x = roundDot(resolved.contentBoundsDots.x);
    const int y = roundDot(resolved.contentBoundsDots.y);
    const int boxWidth = std::max(1, roundDot(resolved.contentBoundsDots.width));
    const int visualFontHeight = std::max(8, resolved.fontHeightDots);
    const int visualFontWidth = std::max(8, resolved.fontWidthDots);
    char font = element.fontName.empty() ? '0' : element.fontName[0];
    const std::vector<std::string> lines = resolved.textLines.empty()
        ? std::vector<std::string>{resolved.value}
        : resolved.textLines;
    const bool useFieldBlock = element.wrap || element.multiLine || element.alignment != TextAlignment::Left;

    auto appendLine = [&](int lineX, int lineY, const std::string& lineValue, int boldOffset)
    {
        zpl << "^FO" << (lineX + boldOffset) << "," << lineY << "\n";
        zpl << "^A" << font << orientationCode(element.rotation) << "," << visualFontHeight << "," << visualFontWidth << "\n";
        if (useFieldBlock)
        {
            zpl << "^FB" << boxWidth << ",1,0," << alignmentCode(element.alignment) << ",0\n";
        }
        zpl << "^FH\\^FD" << escapeFieldData(lineValue) << "^FS\n";
    };

    for (std::size_t i = 0; i < lines.size(); ++i)
    {
        const int lineY = y + static_cast<int>(i) * visualFontHeight;
        appendLine(x, lineY, lines[i], 0);
        if (element.bold)
        {
            appendLine(x, lineY, lines[i], 2);
        }
    }

    if (element.underline)
    {
        int underlineWidth = element.wrap || element.multiLine ? boxWidth : std::max(20, static_cast<int>(resolved.value.size()) * visualFontWidth / 2);
        zpl << "^FO" << x << "," << (y + static_cast<int>(lines.size()) * visualFontHeight + 4) << "\n";
        zpl << "^GB" << underlineWidth << ",2,2^FS\n";
    }

    return zpl.str();
}

std::string ZplGenerator::barcodeZpl(const ResolvedLabelElement& resolved)
{
    const LabelElement& element = resolved.element;
    std::ostringstream zpl;
    const int x = roundDot(resolved.contentBoundsDots.x);
    const int y = roundDot(resolved.contentBoundsDots.y);
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
    const std::string escapedValue = escapeFieldData(resolved.value);
    zpl << "^FH\\^FD" << (element.type == LabelElementType::Code128Barcode ? code128FieldData(escapedValue) : escapedValue) << "^FS\n";
    return zpl.str();
}

std::string ZplGenerator::qrZpl(const ResolvedLabelElement& resolved)
{
    const LabelElement& element = resolved.element;
    std::ostringstream zpl;
    const int x = roundDot(resolved.contentBoundsDots.x);
    const int y = roundDot(resolved.contentBoundsDots.y);
    zpl << "^FO" << x << "," << y << "\n";
    zpl << "^BQN," << element.qrModel << "," << element.qrMagnification << "\n";
    zpl << "^FH\\^FDLA," << escapeFieldData(resolved.value) << "^FS\n";
    return zpl.str();
}

std::string ZplGenerator::shapeZpl(const ResolvedLabelElement& resolved, const LabelTemplate& labelTemplate)
{
    const LabelElement& element = resolved.element;
    std::ostringstream zpl;
    int x = roundDot(resolved.contentBoundsDots.x);
    int y = roundDot(resolved.contentBoundsDots.y);
    int width = std::max(1, roundDot(resolved.contentBoundsDots.width));
    int height = element.type == LabelElementType::Line
        ? std::max(1, element.fontWidthDots)
        : std::max(1, roundDot(resolved.contentBoundsDots.height));
    const int thickness = element.type == LabelElementType::Line
        ? std::max(1, element.fontWidthDots)
        : std::max(1, std::min(element.fontWidthDots, std::min(width, height) / 2));

    if (element.type == LabelElementType::Box)
    {
        const int inset = std::max(thickness, printableEdgeInsetDots(labelTemplate.settings.dpi));
        const int labelWidth = labelTemplate.settings.labelWidthDots();
        const int labelHeight = labelTemplate.settings.labelHeightDots();
        const int right = std::min(x + width, labelWidth - inset);
        const int bottom = std::min(y + height, labelHeight - inset);
        x = std::max(x, inset);
        y = std::max(y, inset);
        width = std::max(1, right - x);
        height = std::max(1, bottom - y);
    }

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
