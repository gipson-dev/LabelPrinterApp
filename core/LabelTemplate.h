#pragma once

#include <string>
#include <vector>

#include "core/LabelElement.h"
#include "core/PrinterSettings.h"

struct LabelTemplate
{
    std::string name = "Default 2.25 x 0.75 Label";
    PrinterSettings settings;
    std::vector<LabelElement> elements;

    static LabelTemplate defaultTemplate()
    {
        LabelTemplate label;
        label.settings.dpi = 203;
        label.settings.labelWidthInches = 2.25;
        label.settings.labelHeightInches = 0.75;
        label.settings.gapInches = 0.125;

        LabelElement item;
        item.id = "item_text";
        item.name = "Item Number";
        item.type = LabelElementType::Text;
        item.source = FieldSource::Variable;
        item.text = "{ItemNumber}";
        item.variableName = "ItemNumber";
        item.xInches = 0.07;
        item.yInches = 0.04;
        item.boxWidthInches = 2.0;
        item.fontHeightDots = 28;
        item.fontWidthDots = 24;
        item.bold = true;
        label.elements.push_back(item);

        LabelElement code;
        code.id = "item_barcode";
        code.name = "Code 128";
        code.type = LabelElementType::Code128Barcode;
        code.source = FieldSource::Variable;
        code.text = "{ItemNumber}";
        code.variableName = "ItemNumber";
        code.xInches = 0.12;
        code.yInches = 0.34;
        code.barcodeHeightDots = 48;
        code.barcodeModuleWidth = 2;
        code.humanReadable = true;
        label.elements.push_back(code);

        return label;
    }
};
