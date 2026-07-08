#include "core/LabelLayout.h"

#include <algorithm>
#include <sstream>

#include "core/BarcodeMetrics.h"
#include "core/LabelUnits.h"
#include "core/SampleData.h"

namespace
{
double inchesToDots(const LabelTemplate& labelTemplate, double inches)
{
    return LabelUnits::inchesToDots(inches, std::max(1, labelTemplate.settings.dpi));
}

std::string resolvedValue(const LabelElement& element, VariableContext context, bool fillMissingSampleData)
{
    if (fillMissingSampleData)
    {
        SampleData::fillMissingForElement(element, context);
    }
    return VariableResolver::elementValue(element, context);
}

LabelRect textBounds(
    const LabelTemplate& labelTemplate,
    const LabelElement& element,
    const std::string& value,
    const PrinterTextCalibration& calibration)
{
    const int fontHeight = LabelLayoutEngine::effectiveTextFontHeightDots(labelTemplate, element, value);
    const double width = std::max(1.0, inchesToDots(labelTemplate, element.boxWidthInches)) * calibration.widthScale;
    const double height = std::max(1.0, LabelLayoutEngine::textBoxHeightDots(element, fontHeight)) * calibration.heightScale;
    return {
        inchesToDots(labelTemplate, element.xInches) + calibration.xOffsetDots,
        inchesToDots(labelTemplate, element.yInches) + calibration.yOffsetDots,
        width,
        height
    };
}

LabelRect barcodeBounds(const LabelTemplate& labelTemplate, const LabelElement& element, const std::string& value)
{
    const double laneWidth = std::max(1.0, inchesToDots(labelTemplate, element.boxWidthInches));
    const double barcodeWidth = std::max(1, BarcodeMetrics::barcodeWidthDots(element, value));
    double x = inchesToDots(labelTemplate, element.xInches);
    if (element.alignment == TextAlignment::Center && laneWidth > barcodeWidth)
    {
        x += (laneWidth - barcodeWidth) / 2.0;
    }
    else if (element.alignment == TextAlignment::Right && laneWidth > barcodeWidth)
    {
        x += laneWidth - barcodeWidth;
    }

    const double humanText = element.humanReadable ? std::max(14.0, element.barcodeHeightDots * 0.22) : 0.0;
    return {
        x,
        inchesToDots(labelTemplate, element.yInches),
        barcodeWidth,
        std::max(1.0, static_cast<double>(element.barcodeHeightDots)) + humanText
    };
}

LabelRect elementBounds(
    const LabelTemplate& labelTemplate,
    const LabelElement& element,
    const std::string& value,
    const PrinterTextCalibration& textCalibration)
{
    if (element.type == LabelElementType::Text)
    {
        return textBounds(labelTemplate, element, value, textCalibration);
    }

    if (element.type == LabelElementType::QrCode)
    {
        const double size = std::max(1.0, element.qrMagnification * LabelUnits::mmToDots(1.4, labelTemplate.settings.dpi));
        return {
            inchesToDots(labelTemplate, element.xInches),
            inchesToDots(labelTemplate, element.yInches),
            size,
            size
        };
    }

    if (element.type == LabelElementType::Line || element.type == LabelElementType::Box)
    {
        const double height = element.type == LabelElementType::Line
            ? std::max(1, element.fontWidthDots)
            : std::max(1, element.fontHeightDots);
        return {
            inchesToDots(labelTemplate, element.xInches),
            inchesToDots(labelTemplate, element.yInches),
            std::max(1.0, inchesToDots(labelTemplate, element.boxWidthInches)),
            height
        };
    }

    return barcodeBounds(labelTemplate, element, value);
}

std::vector<std::string> splitExplicitLines(const std::string& value)
{
    std::vector<std::string> lines;
    std::string line;
    for (char ch : value)
    {
        if (ch == '\r')
        {
            continue;
        }
        if (ch == '\n')
        {
            lines.push_back(line);
            line.clear();
        }
        else
        {
            line += ch;
        }
    }
    lines.push_back(line);
    return lines;
}

std::vector<std::string> splitWords(const std::string& line)
{
    std::vector<std::string> words;
    std::istringstream stream(line);
    std::string word;
    while (stream >> word)
    {
        words.push_back(word);
    }
    if (words.empty())
    {
        words.push_back("");
    }
    return words;
}

void appendLimitedLine(std::vector<std::string>& lines, const std::string& line, int maxLines)
{
    if (static_cast<int>(lines.size()) >= maxLines)
    {
        return;
    }
    lines.push_back(line);
}
}

ResolvedLabelLayout LabelLayoutEngine::resolve(
    const LabelTemplate& labelTemplate,
    const VariableContext& context,
    const LabelResolveOptions& options)
{
    ResolvedLabelLayout layout;
    layout.dpi = std::max(1, labelTemplate.settings.dpi);
    layout.labelWidthDots = LabelUnits::inchesToDots(labelTemplate.settings.labelWidthInches, layout.dpi);
    layout.labelHeightDots = LabelUnits::inchesToDots(labelTemplate.settings.labelHeightInches, layout.dpi);
    layout.elements.reserve(labelTemplate.elements.size());

    for (std::size_t i = 0; i < labelTemplate.elements.size(); ++i)
    {
        const LabelElement& element = labelTemplate.elements[i];
        const std::string value = resolvedValue(element, context, options.fillMissingSampleData);
        const LabelRect bounds = elementBounds(labelTemplate, element, value, options.textCalibration);

        ResolvedLabelElement resolved;
        resolved.sourceIndex = i;
        resolved.zIndex = static_cast<int>(i);
        resolved.layer = CanvasLayer::Elements;
        resolved.element = element;
        resolved.value = value;
        resolved.contentBoundsDots = bounds;
        resolved.selectionBoundsDots = bounds;
        resolved.fontHeightDots = element.type == LabelElementType::Text
            ? effectiveTextFontHeightDots(labelTemplate, element, value)
            : element.fontHeightDots;
        resolved.fontWidthDots = element.type == LabelElementType::Text
            ? effectiveTextFontWidthDots(element, resolved.fontHeightDots)
            : element.fontWidthDots;
        resolved.textLineCount = textLineCount(element);
        if (element.type == LabelElementType::Text)
        {
            resolved.textLines = layoutTextLines(element, value, resolved.fontWidthDots, bounds.width);
        }
        if (element.type == LabelElementType::Text)
        {
            ResolvedTextLayout textLayout;
            textLayout.sourceIndex = i;
            textLayout.id = element.id;
            textLayout.text = value;
            textLayout.xDots = bounds.x;
            textLayout.yDots = bounds.y;
            textLayout.boxWidthDots = bounds.width;
            textLayout.boxHeightDots = bounds.height;
            textLayout.glyphOriginXDots = bounds.x;
            textLayout.glyphOriginYDots = bounds.y;
            textLayout.fontHeightDots = resolved.fontHeightDots;
            textLayout.fontWidthDots = resolved.fontWidthDots;
            textLayout.lines = resolved.textLines;
            textLayout.rotation = element.rotation;
            layout.textLayouts.push_back(textLayout);
        }
        layout.elements.push_back(resolved);
    }

    return layout;
}

LabelRect LabelLayoutEngine::elementModelBoundsDots(
    const LabelTemplate& labelTemplate,
    const LabelElement& element,
    const VariableContext& context,
    const LabelResolveOptions& options)
{
    const std::string value = resolvedValue(element, context, options.fillMissingSampleData);
    return elementBounds(labelTemplate, element, value, options.textCalibration);
}

int LabelLayoutEngine::effectiveTextFontHeightDots(
    const LabelTemplate& labelTemplate,
    const LabelElement& element,
    const std::string& value)
{
    if (!element.autoFit || value.empty() || element.boxWidthInches <= 0.0)
    {
        return std::max(8, element.fontHeightDots);
    }

    const int estimated = static_cast<int>(value.size()) * std::max(1, element.fontWidthDots);
    const int boxWidth = std::max(1, LabelUnits::roundDots(LabelUnits::inchesToDots(element.boxWidthInches, labelTemplate.settings.dpi)));
    if (estimated <= boxWidth)
    {
        return std::max(8, element.fontHeightDots);
    }
    return std::max(8, element.fontHeightDots * boxWidth / std::max(1, estimated));
}

int LabelLayoutEngine::effectiveTextFontWidthDots(const LabelElement& element, int effectiveFontHeightDots)
{
    if (!element.autoFit)
    {
        return std::max(8, element.fontWidthDots);
    }

    return std::max(8, element.fontWidthDots * effectiveFontHeightDots / std::max(1, element.fontHeightDots));
}

int LabelLayoutEngine::textLineCount(const LabelElement& element)
{
    return element.wrap || element.multiLine ? std::max(1, element.maxLines) : 1;
}

double LabelLayoutEngine::textBoxHeightDots(const LabelElement& element, int effectiveFontHeightDots)
{
    return static_cast<double>(std::max(8, effectiveFontHeightDots) * textLineCount(element));
}

double LabelLayoutEngine::estimatedTextWidthDots(const std::string& text, int fontWidthDots)
{
    double width = 0.0;
    const double base = static_cast<double>(std::max(1, fontWidthDots));
    for (char ch : text)
    {
        if (ch == ' ')
        {
            width += base * 0.35;
        }
        else if (ch == 'i' || ch == 'l' || ch == 'I' || ch == '!' || ch == '.' || ch == ',' || ch == '\'')
        {
            width += base * 0.35;
        }
        else if (ch == 'W' || ch == 'M' || ch == 'w' || ch == 'm')
        {
            width += base * 0.82;
        }
        else
        {
            width += base * 0.62;
        }
    }
    return width;
}

std::vector<std::string> LabelLayoutEngine::layoutTextLines(
    const LabelElement& element,
    const std::string& value,
    int effectiveFontWidthDots,
    double boxWidthDots)
{
    const int maxLines = textLineCount(element);
    if (maxLines <= 1 || (!element.wrap && !element.multiLine))
    {
        std::string singleLine;
        for (char ch : value)
        {
            singleLine += (ch == '\r' || ch == '\n') ? ' ' : ch;
        }
        return {singleLine};
    }

    std::vector<std::string> lines;
    const double maxWidth = std::max(1.0, boxWidthDots);
    for (const std::string& explicitLine : splitExplicitLines(value))
    {
        if (!element.wrap)
        {
            appendLimitedLine(lines, explicitLine, maxLines);
            if (static_cast<int>(lines.size()) >= maxLines)
            {
                break;
            }
            continue;
        }

        std::string current;
        for (const std::string& word : splitWords(explicitLine))
        {
            const std::string candidate = current.empty() ? word : current + " " + word;
            if (!current.empty() && estimatedTextWidthDots(candidate, effectiveFontWidthDots) > maxWidth)
            {
                appendLimitedLine(lines, current, maxLines);
                current = word;
                if (static_cast<int>(lines.size()) >= maxLines)
                {
                    break;
                }
            }
            else
            {
                current = candidate;
            }
        }

        if (static_cast<int>(lines.size()) >= maxLines)
        {
            break;
        }
        appendLimitedLine(lines, current, maxLines);
    }

    if (lines.empty())
    {
        lines.push_back("");
    }
    return lines;
}
