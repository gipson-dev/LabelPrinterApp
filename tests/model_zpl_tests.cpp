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
        label.addElement(LabelElement::Text("Part", "PART^A~100", 30, 40, 28, 28, true));
        LabelElement barcode = LabelElement::Barcode("Barcode", "123456", 30, 100, 60);
        barcode.barcodeModuleWidth = 3;
        barcode.barcodeSymbology = BarcodeSymbology::Code39;
        barcode.barcodeHumanReadable = false;
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
        assert(zpl.find("^FO30,40") != std::string::npos);
        assert(zpl.find("^FDPART\\5EA\\7E100^FS") != std::string::npos);
        assert(zpl.find("^BY3") != std::string::npos);
        assert(zpl.find("^B3N,N,60,N,N") != std::string::npos);
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

        LabelElement partName = LabelElement::Text("Part Name", "A-100 \"Left\"", 10, 20, 30, 40, true);
        partName.fieldKey = "part_name";
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
        assert(loaded.elements.size() == 2);
        assert(loaded.elements[0].type == LabelElementType::Text);
        assert(loaded.elements[0].fieldKey == "part_name");
        assert(loaded.elements[0].text == "A-100 \"Left\"");
        assert(loaded.elements[0].bold);
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
            file << "  \"elements\": [\n";
            file << "    {\"name\":\"Line\", \"type\":\"Text\", \"fieldKey\":\"line\", \"text\":\"One\\nTwo\", \"x\":5, \"y\":6, \"fontHeight\":20, \"fontWidth\":21, \"bold\":false, \"editable\":true},\n";
            file << "    {\"name\":\"Code\", \"type\":\"Barcode\", \"fieldKey\":\"code\", \"text\":\"A\\u002d100\", \"x\":10, \"y\":70, \"barcodeHeight\":40, \"barcodeModuleWidth\":3, \"barcodeSymbology\":\"Code39\", \"barcodeHumanReadable\":false, \"editable\":false}\n";
            file << "  ]\n";
            file << "}\n";
        }

        const LabelTemplate loaded = TemplateStorage::LoadFromFile(path.string());
        std::filesystem::remove(path);

        assert(loaded.name == "Structured Template");
        assert(loaded.labelWidthDots == 320);
        assert(loaded.labelHeightDots == 160);
        assert(loaded.elements.size() == 2);
        assert(loaded.elements[0].text == "One\nTwo");
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
