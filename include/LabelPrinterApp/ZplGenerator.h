#pragma once

#include <algorithm>
#include <iomanip>
#include <sstream>
#include <string>

#include "LabelTemplate.h"
#include "LabelValidation.h"
#include "PrinterSettings.h"

struct ZplGenerationResult
{
    bool success = false;
    std::string zpl;
    std::string errorMessage;
};

class ZplGenerator
{
public:
    static ZplGenerationResult tryGenerate(
        const LabelTemplate& labelTemplate,
        const PrinterSettings& settings
    )
    {
        ZplGenerationResult result;
        LabelValidationResult validation = LabelValidation::ValidateTemplate(labelTemplate);
        if (!validation.isValid())
        {
            result.errorMessage = validation.issues.front().message;
            return result;
        }

        result.zpl = generate(labelTemplate, settings);
        result.success = true;
        return result;
    }

    static std::string generate(
        const LabelTemplate& labelTemplate,
        const PrinterSettings& settings
    )
    {
        std::ostringstream zpl;

        zpl << "^XA\n";
        zpl << "^CI28\n";
        zpl << "^LH" << labelTemplate.marginLeftDots << "," << labelTemplate.marginTopDots << "\n";
        zpl << "^MN" << mediaTrackingCode(labelTemplate.mediaTrackingMode) << "\n";
        zpl << "^FW" << (labelTemplate.orientation == LabelOrientation::Landscape ? "R" : "N") << "\n";

        const int widthDots = labelTemplate.labelWidthDots > 0
            ? labelTemplate.labelWidthDots
            : settings.labelWidthDots;
        const int heightDots = labelTemplate.labelHeightDots > 0
            ? labelTemplate.labelHeightDots
            : settings.labelHeightDots;

        zpl << "^PW" << widthDots << "\n";
        zpl << "^LL" << heightDots << "\n";

        zpl << "^MD" << settings.darkness << "\n";
        zpl << "^PR" << settings.printSpeed << "\n";

        for (const auto& element : labelTemplate.elements)
        {
            zpl << "^FO" << element.x << "," << element.y << "\n";

            if (element.type == LabelElementType::Barcode)
            {
                zpl << "^BY" << element.barcodeModuleWidth << "\n";
                char orientation = ToZplOrientation(element.rotation);
                if (element.barcodeSymbology == BarcodeSymbology::Code39)
                {
                    zpl << "^B3" << orientation << ",N," << element.barcodeHeight << ","
                        << (element.barcodeHumanReadable ? "Y" : "N") << ",N\n";
                }
                else
                {
                    zpl << "^BC" << orientation << "," << element.barcodeHeight << ","
                        << (element.barcodeHumanReadable ? "Y" : "N") << ",N,N\n";
                }
                zpl << "^FH\\^FD" << escapeFieldData(element.text) << "^FS\n";
            }
            else
            {
                int fontHeight = effectiveFontHeight(element);
                int fontWidth = effectiveFontWidth(element, fontHeight);
                char orientation = ToZplOrientation(element.rotation);
                char font = element.fontName.empty() ? '0' : element.fontName[0];

                zpl << "^A" << font << orientation << "," << fontHeight << "," << fontWidth << "\n";
                if (element.wrap || element.multiline || element.alignment != TextAlignment::Left)
                {
                    zpl << "^FB" << element.boxWidth << "," << element.maxLines << ",0,"
                        << ToZplAlignment(element.alignment) << ",0\n";
                }
                zpl << "^FH\\^FD" << escapeFieldData(element.text) << "^FS\n";

                if (element.bold)
                {
                    zpl << "^FO" << element.x + 2 << "," << element.y << "\n";
                    zpl << "^A" << font << orientation << "," << fontHeight << "," << fontWidth << "\n";
                    if (element.wrap || element.multiline || element.alignment != TextAlignment::Left)
                    {
                        zpl << "^FB" << element.boxWidth << "," << element.maxLines << ",0,"
                            << ToZplAlignment(element.alignment) << ",0\n";
                    }
                    zpl << "^FH\\^FD" << escapeFieldData(element.text) << "^FS\n";
                }

                if (element.underline)
                {
                    zpl << "^FO" << element.x << "," << (element.y + fontHeight + 4) << "\n";
                    zpl << "^GB" << estimatedTextWidth(element, fontWidth) << ",2,2^FS\n";
                }
            }
        }

        zpl << "^XZ\n";

        return zpl.str();
    }

private:
    static char mediaTrackingCode(MediaTrackingMode mode)
    {
        switch (mode)
        {
        case MediaTrackingMode::BlackMark:
            return 'M';
        case MediaTrackingMode::Continuous:
            return 'N';
        default:
            return 'Y';
        }
    }

    static int effectiveFontHeight(const LabelElement& element)
    {
        if (!element.autoFit || element.text.empty() || element.boxWidth <= 0)
        {
            return element.fontHeight;
        }

        int estimatedWidth = static_cast<int>(element.text.size()) * element.fontWidth;
        if (estimatedWidth <= element.boxWidth)
        {
            return element.fontHeight;
        }

        int fitted = element.fontHeight * element.boxWidth / std::max(1, estimatedWidth);
        return std::max(8, fitted);
    }

    static int effectiveFontWidth(const LabelElement& element, int fontHeight)
    {
        if (!element.autoFit || element.fontHeight <= 0)
        {
            return element.fontWidth;
        }

        return std::max(8, element.fontWidth * fontHeight / element.fontHeight);
    }

    static int estimatedTextWidth(const LabelElement& element, int fontWidth)
    {
        if (element.wrap || element.multiline)
        {
            return element.boxWidth;
        }

        return std::max(20, static_cast<int>(element.text.size()) * fontWidth / 2);
    }

    static std::string escapeFieldData(const std::string& input)
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

    static std::string hexByte(unsigned char value)
    {
        std::ostringstream hex;
        hex << std::uppercase << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(value);
        return hex.str();
    }
}; 
