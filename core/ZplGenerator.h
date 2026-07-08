#pragma once

#include <map>
#include <string>
#include <vector>

#include "core/LabelTemplate.h"
#include "core/LabelLayout.h"
#include "core/VariableResolver.h"

class ZplGenerator
{
public:
    static std::string generate(const LabelTemplate& labelTemplate, const VariableContext& context = {});
    static std::string generateDebugReport(const LabelTemplate& labelTemplate, const VariableContext& context = {});
    static std::vector<std::string> generateSerialRange(const LabelTemplate& labelTemplate, int start, int end, int step, const VariableContext& baseContext = {});

private:
    static std::string elementZpl(const ResolvedLabelElement& resolved, const LabelTemplate& labelTemplate);
    static std::string textZpl(const ResolvedLabelElement& resolved);
    static std::string barcodeZpl(const ResolvedLabelElement& resolved);
    static std::string qrZpl(const ResolvedLabelElement& resolved);
    static std::string shapeZpl(const ResolvedLabelElement& resolved, const LabelTemplate& labelTemplate);
    static std::string escapeFieldData(const std::string& input);
    static std::string hexByte(unsigned char value);
    static char orientationCode(ElementRotation rotation);
    static char alignmentCode(TextAlignment alignment);
    static char mediaCode(MediaSensingMode mode);
};
