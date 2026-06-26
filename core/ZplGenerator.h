#pragma once

#include <map>
#include <string>
#include <vector>

#include "core/LabelTemplate.h"
#include "core/VariableResolver.h"

class ZplGenerator
{
public:
    static std::string generate(const LabelTemplate& labelTemplate, const VariableContext& context = {});
    static std::vector<std::string> generateSerialRange(const LabelTemplate& labelTemplate, int start, int end, int step, const VariableContext& baseContext = {});

private:
    static std::string elementZpl(const LabelElement& element, const LabelTemplate& labelTemplate, const VariableContext& context);
    static std::string textZpl(const LabelElement& element, const LabelTemplate& labelTemplate, const std::string& value);
    static std::string barcodeZpl(const LabelElement& element, const LabelTemplate& labelTemplate, const std::string& value);
    static std::string qrZpl(const LabelElement& element, const LabelTemplate& labelTemplate, const std::string& value);
    static std::string shapeZpl(const LabelElement& element, const LabelTemplate& labelTemplate);
    static std::string escapeFieldData(const std::string& input);
    static std::string hexByte(unsigned char value);
    static char orientationCode(ElementRotation rotation);
    static char alignmentCode(TextAlignment alignment);
    static char mediaCode(MediaSensingMode mode);
    static int dots(const LabelTemplate& labelTemplate, double inches);
    static int effectiveFontHeight(const LabelElement& element, const LabelTemplate& labelTemplate);
};
