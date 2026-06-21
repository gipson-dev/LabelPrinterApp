#pragma once

#include <string>

enum class LabelElementType
{
    Text,
    Barcode
};

enum class BarcodeSymbology
{
    Code128,
    Code39
};

inline const char* ToString(LabelElementType type)
{
    return type == LabelElementType::Barcode ? "Barcode" : "Text";
}

inline LabelElementType LabelElementTypeFromString(const std::string& value)
{
    return value == "Barcode" ? LabelElementType::Barcode : LabelElementType::Text;
}

inline const char* ToString(BarcodeSymbology symbology)
{
    return symbology == BarcodeSymbology::Code39 ? "Code39" : "Code128";
}

inline BarcodeSymbology BarcodeSymbologyFromString(const std::string& value)
{
    return value == "Code39" ? BarcodeSymbology::Code39 : BarcodeSymbology::Code128;
}

struct LabelElement
{
    LabelElementType type = LabelElementType::Text;
    std::string name;
    std::string text;
    std::string fieldKey;

    int x = 20;
    int y = 20;

    int fontHeight = 30;
    int fontWidth = 30;
    int barcodeHeight = 60;
    int barcodeModuleWidth = 2;
    BarcodeSymbology barcodeSymbology = BarcodeSymbology::Code128;
    bool barcodeHumanReadable = true;

    bool bold = false;
    bool editable = true;

    LabelElement() = default;

    static LabelElement Text(
        const std::string& nameValue,
        const std::string& textValue,
        int xValue,
        int yValue,
        int heightValue,
        int widthValue,
        bool boldValue = false
    )
    {
        LabelElement element;
        element.type = LabelElementType::Text;
        element.name = nameValue;
        element.fieldKey = nameValue;
        element.text = textValue;
        element.x = xValue;
        element.y = yValue;
        element.fontHeight = heightValue;
        element.fontWidth = widthValue;
        element.bold = boldValue;
        return element;
    }

    static LabelElement Barcode(
        const std::string& nameValue,
        const std::string& value,
        int xValue,
        int yValue,
        int heightValue
    )
    {
        LabelElement element;
        element.type = LabelElementType::Barcode;
        element.name = nameValue;
        element.fieldKey = nameValue;
        element.text = value;
        element.x = xValue;
        element.y = yValue;
        element.barcodeHeight = heightValue;
        return element;
    }
};
