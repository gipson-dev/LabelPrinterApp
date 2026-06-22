#include <cassert>
#include <filesystem>
#include <fstream>
#include <string>

#include "LabelPrinterApp/LabelValidation.h"
#include "LabelPrinterApp/TemplateStorage.h"
#include "LabelPrinterApp/ZplGenerator.h"

namespace
{
    void GeneratedZplIncludesLabelSettingsAndElements()
    {
        LabelTemplate label;
        label.name = "Unit Test";
        label.labelWidthDots = 400;
        label.labelHeightDots = 240;
        label.marginLeftDots = 12;
        label.marginTopDots = 8;
        label.mediaTrackingMode = MediaTrackingMode::BlackMark;
        label.orientation = LabelOrientation::Landscape;

        LabelElement part = LabelElement::Text("Part", "PART^A~100", 30, 40, 28, 28, true);
        part.boxWidth = 180;
        part.maxLines = 2;
        part.wrap = true;
        part.alignment = TextAlignment::Center;
        part.underline = true;
        part.autoFit = true;
        label.addElement(part);

        LabelElement barcode = LabelElement::Barcode("Barcode", "123456", 30, 100, 60);
        barcode.barcodeModuleWidth = 3;
        barcode.barcodeSymbology = BarcodeSymbology::Code39;
        barcode.barcodeHumanReadable = false;
        barcode.rotation = FieldRotation::Rotate90;
        label.addElement(barcode);

        PrinterSettings settings;
        settings.darkness = 12;
        settings.printSpeed = 5;

        const std::string zpl = ZplGenerator::generate(label, settings);

        assert(zpl.find("^XA") != std::string::npos);
        assert(zpl.find("^PW400") != std::string::npos);
        assert(zpl.find("^LL240") != std::string::npos);
        assert(zpl.find("^MD12") != std::string::npos);
        assert(zpl.find("^PR5") != std::string::npos);
        assert(zpl.find("^CI28") != std::string::npos);
        assert(zpl.find("^LH12,8") != std::string::npos);
        assert(zpl.find("^MNM") != std::string::npos);
        assert(zpl.find("^FWR") != std::string::npos);
        assert(zpl.find("^FO30,40") != std::string::npos);
        assert(zpl.find("^FB180,2,0,C,0") != std::string::npos);
        assert(zpl.find("^FDPART\\5EA\\7E100^FS") != std::string::npos);
        assert(zpl.find("^GB180,2,2^FS") != std::string::npos);
        assert(zpl.find("^BY3") != std::string::npos);
        assert(zpl.find("^B3R,N,60,N,N") != std::string::npos);
        assert(zpl.find("^FD123456^FS") != std::string::npos);
        assert(zpl.rfind("^XZ\n") == zpl.size() - 4);
    }

    void InvalidTemplateDoesNotGenerateZpl()
    {
        LabelTemplate label;
        label.name = "";
        label.elements.clear();

        PrinterSettings settings;
        const ZplGenerationResult result = ZplGenerator::tryGenerate(label, settings);

        assert(!result.success);
        assert(result.zpl.empty());
        assert(!result.errorMessage.empty());
    }

    void TemplateStorageRoundTripsTextAndBarcodeElements()
    {
        LabelTemplate label;
        label.name = "Escaped \"Label\"";
        label.labelWidthDots = 500;
        label.labelHeightDots = 300;
        label.marginLeftDots = 5;
        label.marginTopDots = 6;
        label.gapDots = 30;
        label.mediaTrackingMode = MediaTrackingMode::Continuous;
        label.orientation = LabelOrientation::Landscape;

        LabelElement partName = LabelElement::Text("Part Name", "A-100 \"Left\"", 10, 20, 30, 40, true);
        partName.fieldKey = "part_name";
        partName.fontName = "A";
        partName.boxWidth = 220;
        partName.maxLines = 3;
        partName.rotation = FieldRotation::Inverted;
        partName.alignment = TextAlignment::Right;
        partName.italic = true;
        partName.underline = true;
        partName.multiline = true;
        partName.variable = false;
        partName.autoFit = true;
        partName.wrap = true;
        partName.editable = false;
        label.addElement(partName);

        LabelElement serial = LabelElement::Barcode("Serial", "SN-001", 50, 90, 70);
        serial.fieldKey = "serial_number";
        serial.barcodeModuleWidth = 4;
        serial.barcodeSymbology = BarcodeSymbology::Code39;
        serial.barcodeHumanReadable = false;
        serial.editable = false;
        label.addElement(serial);

        const std::filesystem::path path = std::filesystem::temp_directory_path() / "label_printer_template_test.json";
        assert(TemplateStorage::SaveToFile(label, path.string()));

        const LabelTemplate loaded = TemplateStorage::LoadFromFile(path.string());
        std::filesystem::remove(path);

        assert(loaded.name == label.name);
        assert(loaded.labelWidthDots == 500);
        assert(loaded.labelHeightDots == 300);
        assert(loaded.marginLeftDots == 5);
        assert(loaded.marginTopDots == 6);
        assert(loaded.gapDots == 30);
        assert(loaded.mediaTrackingMode == MediaTrackingMode::Continuous);
        assert(loaded.orientation == LabelOrientation::Landscape);
        assert(loaded.elements.size() == 2);
        assert(loaded.elements[0].type == LabelElementType::Text);
        assert(loaded.elements[0].fieldKey == "part_name");
        assert(loaded.elements[0].fontName == "A");
        assert(loaded.elements[0].boxWidth == 220);
        assert(loaded.elements[0].maxLines == 3);
        assert(loaded.elements[0].rotation == FieldRotation::Inverted);
        assert(loaded.elements[0].alignment == TextAlignment::Right);
        assert(loaded.elements[0].text == "A-100 \"Left\"");
        assert(loaded.elements[0].bold);
        assert(loaded.elements[0].italic);
        assert(loaded.elements[0].underline);
        assert(loaded.elements[0].multiline);
        assert(!loaded.elements[0].variable);
        assert(loaded.elements[0].autoFit);
        assert(loaded.elements[0].wrap);
        assert(!loaded.elements[0].editable);
        assert(loaded.elements[1].type == LabelElementType::Barcode);
        assert(loaded.elements[1].fieldKey == "serial_number");
        assert(loaded.elements[1].barcodeHeight == 70);
        assert(loaded.elements[1].barcodeModuleWidth == 4);
        assert(loaded.elements[1].barcodeSymbology == BarcodeSymbology::Code39);
        assert(!loaded.elements[1].barcodeHumanReadable);
        assert(!loaded.elements[1].editable);
    }

    void TemplateStorageParsesStructuredEscapedJson()
    {
        const std::filesystem::path path = std::filesystem::temp_directory_path() / "structured_label_printer_template_test.json";
        {
            std::ofstream file(path);
            file << "{\n";
            file << "  \"name\": \"Structured Template\",\n";
            file << "  \"labelWidthDots\": 320,\n";
            file << "  \"labelHeightDots\": 160,\n";
            file << "  \"marginLeftDots\": 4,\n";
            file << "  \"marginTopDots\": 5,\n";
            file << "  \"gapDots\": 22,\n";
            file << "  \"mediaTrackingMode\": \"BlackMark\",\n";
            file << "  \"orientation\": \"Landscape\",\n";
            file << "  \"elements\": [\n";
            file << "    {\"name\":\"Line\", \"type\":\"Text\", \"fieldKey\":\"line\", \"text\":\"One\\nTwo\", \"x\":5, \"y\":6, \"fontHeight\":20, \"fontWidth\":21, \"fontName\":\"B\", \"boxWidth\":100, \"maxLines\":2, \"rotation\":\"Rotate270\", \"alignment\":\"Center\", \"bold\":false, \"italic\":true, \"underline\":true, \"multiline\":true, \"variable\":false, \"autoFit\":true, \"wrap\":true, \"editable\":true},\n";
            file << "    {\"name\":\"Code\", \"type\":\"Barcode\", \"fieldKey\":\"code\", \"text\":\"A\\u002d100\", \"x\":10, \"y\":70, \"barcodeHeight\":40, \"barcodeModuleWidth\":3, \"barcodeSymbology\":\"Code39\", \"barcodeHumanReadable\":false, \"editable\":false}\n";
            file << "  ]\n";
            file << "}\n";
        }

        const LabelTemplate loaded = TemplateStorage::LoadFromFile(path.string());
        std::filesystem::remove(path);

        assert(loaded.name == "Structured Template");
        assert(loaded.labelWidthDots == 320);
        assert(loaded.labelHeightDots == 160);
        assert(loaded.marginLeftDots == 4);
        assert(loaded.marginTopDots == 5);
        assert(loaded.gapDots == 22);
        assert(loaded.mediaTrackingMode == MediaTrackingMode::BlackMark);
        assert(loaded.orientation == LabelOrientation::Landscape);
        assert(loaded.elements.size() == 2);
        assert(loaded.elements[0].text == "One\nTwo");
        assert(loaded.elements[0].fontName == "B");
        assert(loaded.elements[0].boxWidth == 100);
        assert(loaded.elements[0].maxLines == 2);
        assert(loaded.elements[0].rotation == FieldRotation::Rotate270);
        assert(loaded.elements[0].alignment == TextAlignment::Center);
        assert(loaded.elements[0].italic);
        assert(loaded.elements[0].underline);
        assert(loaded.elements[0].multiline);
        assert(!loaded.elements[0].variable);
        assert(loaded.elements[0].autoFit);
        assert(loaded.elements[0].wrap);
        assert(loaded.elements[1].text == "A-100");
        assert(loaded.elements[1].barcodeModuleWidth == 3);
        assert(loaded.elements[1].barcodeSymbology == BarcodeSymbology::Code39);
        assert(!loaded.elements[1].barcodeHumanReadable);
        assert(!loaded.elements[1].editable);
    }

    void MissingTemplateFallsBackToDefault()
    {
        const LabelTemplate label = TemplateStorage::LoadFromFile("missing-template-for-test.json");

        assert(label.name == "Default Label");
        assert(!label.elements.empty());
    }

    void InvalidTemplateFileFallsBackToDefault()
    {
        const std::filesystem::path path = std::filesystem::temp_directory_path() / "invalid_label_printer_template_test.json";
        {
            std::ofstream file(path);
            file << "{ \"name\": \"Bad\", \"labelWidthDots\": 0, \"labelHeightDots\": 100, \"elements\": [] }";
        }

        const LabelTemplate label = TemplateStorage::LoadFromFile(path.string());
        std::filesystem::remove(path);

        assert(label.name == "Default Label");
        assert(!label.elements.empty());
    }

    void InvalidTemplateCannotBeSaved()
    {
        LabelTemplate label;
        label.name = "";
        label.labelWidthDots = 0;

        const std::filesystem::path path = std::filesystem::temp_directory_path() / "invalid_label_printer_save_test.json";
        std::filesystem::remove(path);

        assert(!TemplateStorage::SaveToFile(label, path.string()));
        assert(!std::filesystem::exists(path));
    }

    void LabelValidationAcceptsDefaultTemplate()
    {
        const LabelTemplate label = TemplateStorage::LoadDefaultTemplate();
        const LabelValidationResult result = LabelValidation::ValidateTemplate(label);

        assert(result.isValid());
    }

    void LabelValidationRejectsInvalidDimensionsAndElements()
    {
        LabelTemplate label;
        label.name = "";
        label.labelWidthDots = 0;
        label.labelHeightDots = -1;

        LabelElement element = LabelElement::Text("", "Bad", -10, 20, 0, 30);
        element.fieldKey = "";
        label.addElement(element);

        const LabelValidationResult result = LabelValidation::ValidateTemplate(label);

        assert(!result.isValid());
        assert(result.issues.size() >= 5);
    }

    void LabelElementTypeConvertsToAndFromStorageText()
    {
        assert(std::string(ToString(LabelElementType::Text)) == "Text");
        assert(std::string(ToString(LabelElementType::Barcode)) == "Barcode");
        assert(LabelElementTypeFromString("Text") == LabelElementType::Text);
        assert(LabelElementTypeFromString("Barcode") == LabelElementType::Barcode);
        assert(LabelElementTypeFromString("Unexpected") == LabelElementType::Text);
        assert(std::string(ToString(BarcodeSymbology::Code128)) == "Code128");
        assert(std::string(ToString(BarcodeSymbology::Code39)) == "Code39");
        assert(BarcodeSymbologyFromString("Code128") == BarcodeSymbology::Code128);
        assert(BarcodeSymbologyFromString("Code39") == BarcodeSymbology::Code39);
        assert(BarcodeSymbologyFromString("Unexpected") == BarcodeSymbology::Code128);
        assert(std::string(ToString(FieldRotation::Rotate90)) == "Rotate90");
        assert(FieldRotationFromString("Rotate270") == FieldRotation::Rotate270);
        assert(ToZplOrientation(FieldRotation::Inverted) == 'I');
        assert(std::string(ToString(TextAlignment::Center)) == "Center");
        assert(TextAlignmentFromString("Right") == TextAlignment::Right);
        assert(ToZplAlignment(TextAlignment::Left) == 'L');
        assert(std::string(ToString(MediaTrackingMode::BlackMark)) == "BlackMark");
        assert(MediaTrackingModeFromString("Continuous") == MediaTrackingMode::Continuous);
        assert(std::string(ToString(LabelOrientation::Landscape)) == "Landscape");
        assert(LabelOrientationFromString("Portrait") == LabelOrientation::Portrait);
    }
}

int main()
{
    GeneratedZplIncludesLabelSettingsAndElements();
    InvalidTemplateDoesNotGenerateZpl();
    TemplateStorageRoundTripsTextAndBarcodeElements();
    TemplateStorageParsesStructuredEscapedJson();
    MissingTemplateFallsBackToDefault();
    InvalidTemplateFileFallsBackToDefault();
    InvalidTemplateCannotBeSaved();
    LabelValidationAcceptsDefaultTemplate();
    LabelValidationRejectsInvalidDimensionsAndElements();
    LabelElementTypeConvertsToAndFromStorageText();

    return 0;
}
