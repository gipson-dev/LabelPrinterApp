#include "core/TemplateStorage.h"

#include <fstream>
#include <exception>
#include <nlohmann/json.hpp>

using nlohmann::json;

namespace
{
    std::string toString(LabelElementType type)
    {
        switch (type)
        {
        case LabelElementType::Code128Barcode: return "Code128";
        case LabelElementType::Code39Barcode: return "Code39";
        case LabelElementType::QrCode: return "QRCode";
        default: return "Text";
        }
    }

    LabelElementType elementTypeFromString(const std::string& value)
    {
        if (value == "Code128") return LabelElementType::Code128Barcode;
        if (value == "Code39") return LabelElementType::Code39Barcode;
        if (value == "QRCode") return LabelElementType::QrCode;
        return LabelElementType::Text;
    }

    std::string toString(FieldSource source)
    {
        switch (source)
        {
        case FieldSource::Variable: return "Variable";
        case FieldSource::PromptAtPrint: return "PromptAtPrint";
        case FieldSource::SerialNumber: return "SerialNumber";
        default: return "Fixed";
        }
    }

    FieldSource sourceFromString(const std::string& value)
    {
        if (value == "Variable") return FieldSource::Variable;
        if (value == "PromptAtPrint") return FieldSource::PromptAtPrint;
        if (value == "SerialNumber") return FieldSource::SerialNumber;
        return FieldSource::Fixed;
    }

    std::string toString(TextAlignment alignment)
    {
        switch (alignment)
        {
        case TextAlignment::Center: return "Center";
        case TextAlignment::Right: return "Right";
        default: return "Left";
        }
    }

    TextAlignment alignmentFromString(const std::string& value)
    {
        if (value == "Center") return TextAlignment::Center;
        if (value == "Right") return TextAlignment::Right;
        return TextAlignment::Left;
    }

    std::string toString(ElementRotation rotation)
    {
        switch (rotation)
        {
        case ElementRotation::Deg90: return "90";
        case ElementRotation::Deg180: return "180";
        case ElementRotation::Deg270: return "270";
        default: return "0";
        }
    }

    ElementRotation rotationFromString(const std::string& value)
    {
        if (value == "90") return ElementRotation::Deg90;
        if (value == "180") return ElementRotation::Deg180;
        if (value == "270") return ElementRotation::Deg270;
        return ElementRotation::Deg0;
    }

    std::string toString(MediaSensingMode mode)
    {
        switch (mode)
        {
        case MediaSensingMode::BlackMark: return "BlackMark";
        case MediaSensingMode::Continuous: return "Continuous";
        default: return "Gap";
        }
    }

    MediaSensingMode mediaFromString(const std::string& value)
    {
        if (value == "BlackMark") return MediaSensingMode::BlackMark;
        if (value == "Continuous") return MediaSensingMode::Continuous;
        return MediaSensingMode::Gap;
    }

    json elementToJson(const LabelElement& e)
    {
        return {
            {"id", e.id}, {"name", e.name}, {"type", toString(e.type)}, {"source", toString(e.source)},
            {"text", e.text}, {"variableName", e.variableName}, {"prefix", e.prefix}, {"suffix", e.suffix},
            {"xInches", e.xInches}, {"yInches", e.yInches}, {"boxWidthInches", e.boxWidthInches},
            {"fontHeightDots", e.fontHeightDots}, {"fontWidthDots", e.fontWidthDots}, {"fontName", e.fontName},
            {"bold", e.bold}, {"italic", e.italic}, {"underline", e.underline}, {"autoFit", e.autoFit},
            {"wrap", e.wrap}, {"multiLine", e.multiLine}, {"maxLines", e.maxLines},
            {"alignment", toString(e.alignment)}, {"rotation", toString(e.rotation)},
            {"barcodeHeightDots", e.barcodeHeightDots}, {"barcodeModuleWidth", e.barcodeModuleWidth}, {"humanReadable", e.humanReadable},
            {"qrModel", e.qrModel}, {"qrMagnification", e.qrMagnification},
            {"serialStart", e.serialStart}, {"serialStep", e.serialStep}, {"serialWidth", e.serialWidth},
            {"doNotPrint", e.doNotPrint}, {"locked", e.locked}
        };
    }

    LabelElement elementFromJson(const json& j)
    {
        LabelElement e;
        e.id = j.value("id", "");
        e.name = j.value("name", "Element");
        e.type = elementTypeFromString(j.value("type", "Text"));
        e.source = sourceFromString(j.value("source", "Fixed"));
        e.text = j.value("text", "");
        e.variableName = j.value("variableName", "");
        e.prefix = j.value("prefix", "");
        e.suffix = j.value("suffix", "");
        e.xInches = j.value("xInches", e.xInches);
        e.yInches = j.value("yInches", e.yInches);
        e.boxWidthInches = j.value("boxWidthInches", e.boxWidthInches);
        e.fontHeightDots = j.value("fontHeightDots", e.fontHeightDots);
        e.fontWidthDots = j.value("fontWidthDots", e.fontWidthDots);
        e.fontName = j.value("fontName", e.fontName);
        e.bold = j.value("bold", e.bold);
        e.italic = j.value("italic", e.italic);
        e.underline = j.value("underline", e.underline);
        e.autoFit = j.value("autoFit", e.autoFit);
        e.wrap = j.value("wrap", e.wrap);
        e.multiLine = j.value("multiLine", e.multiLine);
        e.maxLines = j.value("maxLines", e.maxLines);
        e.alignment = alignmentFromString(j.value("alignment", "Left"));
        e.rotation = rotationFromString(j.value("rotation", "0"));
        e.barcodeHeightDots = j.value("barcodeHeightDots", e.barcodeHeightDots);
        e.barcodeModuleWidth = j.value("barcodeModuleWidth", e.barcodeModuleWidth);
        e.humanReadable = j.value("humanReadable", e.humanReadable);
        e.qrModel = j.value("qrModel", e.qrModel);
        e.qrMagnification = j.value("qrMagnification", e.qrMagnification);
        e.serialStart = j.value("serialStart", e.serialStart);
        e.serialStep = j.value("serialStep", e.serialStep);
        e.serialWidth = j.value("serialWidth", e.serialWidth);
        e.doNotPrint = j.value("doNotPrint", e.doNotPrint);
        e.locked = j.value("locked", e.locked);
        return e;
    }
}

LabelTemplate TemplateStorage::load(const std::string& path)
{
    std::ifstream file(path);
    if (!file)
    {
        return LabelTemplate::defaultTemplate();
    }

    json j;
    try
    {
        file >> j;
    }
    catch (const std::exception&)
    {
        return LabelTemplate::defaultTemplate();
    }

    try
    {
        LabelTemplate t;
        t.name = j.value("name", t.name);
        auto settings = j.value("settings", json::object());
        t.settings.printerName = settings.value("printerName", "");
        t.settings.dpi = settings.value("dpi", 203);
        t.settings.darkness = settings.value("darkness", 15);
        t.settings.speedIps = settings.value("speedIps", 4);
        t.settings.copies = settings.value("copies", 1);
        t.settings.labelWidthInches = settings.value("labelWidthInches", 2.25);
        t.settings.labelHeightInches = settings.value("labelHeightInches", 0.75);
        t.settings.marginLeftInches = settings.value("marginLeftInches", 0.0);
        t.settings.marginTopInches = settings.value("marginTopInches", 0.0);
        t.settings.gapInches = settings.value("gapInches", 0.125);
        t.settings.mediaSensing = mediaFromString(settings.value("mediaSensing", "Gap"));
        t.settings.orientation = settings.value("orientation", "Portrait") == "Landscape" ? LabelOrientation::Landscape : LabelOrientation::Portrait;

        for (const auto& item : j.value("elements", json::array()))
        {
            t.elements.push_back(elementFromJson(item));
        }
        return t.elements.empty() ? LabelTemplate::defaultTemplate() : t;
    }
    catch (const std::exception&)
    {
        return LabelTemplate::defaultTemplate();
    }
}

bool TemplateStorage::save(const LabelTemplate& labelTemplate, const std::string& path, std::string& errorMessage)
{
    json settings = {
        {"printerName", labelTemplate.settings.printerName},
        {"dpi", labelTemplate.settings.dpi},
        {"darkness", labelTemplate.settings.darkness},
        {"speedIps", labelTemplate.settings.speedIps},
        {"copies", labelTemplate.settings.copies},
        {"labelWidthInches", labelTemplate.settings.labelWidthInches},
        {"labelHeightInches", labelTemplate.settings.labelHeightInches},
        {"marginLeftInches", labelTemplate.settings.marginLeftInches},
        {"marginTopInches", labelTemplate.settings.marginTopInches},
        {"gapInches", labelTemplate.settings.gapInches},
        {"mediaSensing", toString(labelTemplate.settings.mediaSensing)},
        {"orientation", labelTemplate.settings.orientation == LabelOrientation::Landscape ? "Landscape" : "Portrait"}
    };

    json elements = json::array();
    for (const LabelElement& e : labelTemplate.elements)
    {
        elements.push_back(elementToJson(e));
    }

    json root = {{"name", labelTemplate.name}, {"settings", settings}, {"elements", elements}};
    std::ofstream file(path);
    if (!file)
    {
        errorMessage = "Unable to open template for writing: " + path;
        return false;
    }
    try
    {
        file << root.dump(2);
        return true;
    }
    catch (const std::exception& ex)
    {
        errorMessage = ex.what();
        return false;
    }
}
