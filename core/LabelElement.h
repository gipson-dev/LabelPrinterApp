#pragma once

#include <string>

enum class LabelElementType
{
    Text,
    Code128Barcode,
    Code39Barcode,
    QrCode
};

enum class ElementRotation
{
    Deg0,
    Deg90,
    Deg180,
    Deg270
};

enum class TextAlignment
{
    Left,
    Center,
    Right
};

enum class FieldSource
{
    Fixed,
    Variable,
    PromptAtPrint,
    SerialNumber
};

struct LabelElement
{
    std::string id;
    std::string name;
    LabelElementType type = LabelElementType::Text;
    FieldSource source = FieldSource::Fixed;

    std::string text;
    std::string variableName;
    std::string prefix;
    std::string suffix;

    double xInches = 0.05;
    double yInches = 0.05;
    double boxWidthInches = 2.0;

    int fontHeightDots = 32;
    int fontWidthDots = 28;
    std::string fontName = "0";
    bool bold = false;
    bool italic = false;
    bool underline = false;
    bool autoFit = false;
    bool wrap = false;
    bool multiLine = false;
    int maxLines = 1;
    TextAlignment alignment = TextAlignment::Left;
    ElementRotation rotation = ElementRotation::Deg0;

    int barcodeHeightDots = 60;
    int barcodeModuleWidth = 2;
    bool humanReadable = true;

    int qrModel = 2;
    int qrMagnification = 4;

    int serialStart = 1;
    int serialStep = 1;
    int serialWidth = 0;

    bool doNotPrint = false;
    bool locked = false;
};
