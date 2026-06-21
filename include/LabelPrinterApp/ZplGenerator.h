#pragma once

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
                if (element.barcodeSymbology == BarcodeSymbology::Code39)
                {
                    zpl << "^B3N,N," << element.barcodeHeight << ","
                        << (element.barcodeHumanReadable ? "Y" : "N") << ",N\n";
                }
                else
                {
                    zpl << "^BCN," << element.barcodeHeight << ","
                        << (element.barcodeHumanReadable ? "Y" : "N") << ",N,N\n";
                }
                zpl << "^FH\\^FD" << escapeFieldData(element.text) << "^FS\n";
            }
            else if (element.bold)
            {
                // Fake bold by printing the same text twice with slight offset
                zpl << "^A0N," << element.fontHeight << "," << element.fontWidth << "\n";
                zpl << "^FH\\^FD" << escapeFieldData(element.text) << "^FS\n";

                zpl << "^FO" << element.x + 2 << "," << element.y << "\n";
                zpl << "^A0N," << element.fontHeight << "," << element.fontWidth << "\n";
                zpl << "^FH\\^FD" << escapeFieldData(element.text) << "^FS\n";
            }
            else
            {
                zpl << "^A0N," << element.fontHeight << "," << element.fontWidth << "\n";
                zpl << "^FH\\^FD" << escapeFieldData(element.text) << "^FS\n";
            }
        }

        zpl << "^XZ\n";

        return zpl.str();
    }

private:
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
