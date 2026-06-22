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

enum class FieldRotation
{
    Normal,
    Rotate90,
    Inverted,
    Rotate270
};

enum class TextAlignment
{
    Left,
    Center,
    Right
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

inline const char* ToString(FieldRotation rotation)
{
    switch (rotation)
    {
    case FieldRotation::Rotate90:
        return "Rotate90";
    case FieldRotation::Inverted:
        return "Inverted";
    case FieldRotation::Rotate270:
        return "Rotate270";
    default:
        return "Normal";
    }
}

inline FieldRotation FieldRotationFromString(const std::string& value)
{
    if (value == "Rotate90") return FieldRotation::Rotate90;
    if (value == "Inverted") return FieldRotation::Inverted;
    if (value == "Rotate270") return FieldRotation::Rotate270;
    return FieldRotation::Normal;
}

inline char ToZplOrientation(FieldRotation rotation)
{
    switch (rotation)
    {
    case FieldRotation::Rotate90:
        return 'R';
    case FieldRotation::Inverted:
        return 'I';
    case FieldRotation::Rotate270:
        return 'B';
    default:
        return 'N';
    }
}

inline const char* ToString(TextAlignment alignment)
{
    switch (alignment)
    {
    case TextAlignment::Center:
        return "Center";
    case TextAlignment::Right:
        return "Right";
    default:
        return "Left";
    }
}

inline TextAlignment TextAlignmentFromString(const std::string& value)
{
    if (value == "Center") return TextAlignment::Center;
    if (value == "Right") return TextAlignment::Right;
    return TextAlignment::Left;
}

inline char ToZplAlignment(TextAlignment alignment)
{
    switch (alignment)
    {
    case TextAlignment::Center:
        return 'C';
    case TextAlignment::Right:
        return 'R';
    default:
        return 'L';
    }
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
    std::string fontName = "0";
    int boxWidth = 260;
    int maxLines = 1;
    int barcodeHeight = 60;
    int barcodeModuleWidth = 2;
    BarcodeSymbology barcodeSymbology = BarcodeSymbology::Code128;
    bool barcodeHumanReadable = true;
    FieldRotation rotation = FieldRotation::Normal;
    TextAlignment alignment = TextAlignment::Left;

    bool bold = false;
    bool italic = false;
    bool underline = false;
    bool multiline = false;
    bool variable = true;
    bool autoFit = false;
    bool wrap = false;
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
