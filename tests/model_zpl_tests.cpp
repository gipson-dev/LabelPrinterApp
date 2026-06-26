#include <cassert>
#include <filesystem>
#include <fstream>
#include <string>

#include "core/CsvImporter.h"
#include "core/LabelTemplate.h"
#include "core/TemplateStorage.h"
#include "core/VariableResolver.h"
#include "core/ZplGenerator.h"

namespace
{
    void GeneratedZplIncludesSettingsAndVersionFiveElements()
    {
        LabelTemplate label = LabelTemplate::defaultTemplate();
        label.settings.dpi = 203;
        label.settings.darkness = 12;
        label.settings.speedIps = 5;
        label.settings.mediaSensing = MediaSensingMode::BlackMark;
        label.settings.orientation = LabelOrientation::Landscape;

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

        LabelElement code128;
        code128.id = "item_barcode";
        code128.name = "Code 128";
        code128.type = LabelElementType::Code128Barcode;
        code128.source = FieldSource::Variable;
        code128.text = "{ItemNumber}";
        code128.variableName = "ItemNumber";
        code128.xInches = 0.12;
        code128.yInches = 0.34;
        code128.barcodeHeightDots = 48;
        code128.barcodeModuleWidth = 2;
        code128.humanReadable = true;
        label.elements.push_back(code128);

        LabelElement code39;
        code39.id = "code39";
        code39.name = "Code 39";
        code39.type = LabelElementType::Code39Barcode;
        code39.text = "{Serial}";
        code39.xInches = 0.1;
        code39.yInches = 0.55;
        code39.barcodeHeightDots = 35;
        code39.barcodeModuleWidth = 3;
        code39.humanReadable = false;
        label.elements.push_back(code39);

        LabelElement qr;
        qr.id = "qr";
        qr.name = "QR";
        qr.type = LabelElementType::QrCode;
        qr.text = "{ItemNumber}";
        qr.xInches = 1.8;
        qr.yInches = 0.08;
        qr.qrMagnification = 3;
        label.elements.push_back(qr);

        VariableContext context;
        context.values["ItemNumber"] = "PART^A~100";
        context.serialNumber = 42;

        const std::string zpl = ZplGenerator::generate(label, context);
        assert(zpl.find("^XA") != std::string::npos);
        assert(zpl.find("^CI28") != std::string::npos);
        assert(zpl.find("^PW457") != std::string::npos);
        assert(zpl.find("^LL152") != std::string::npos);
        assert(zpl.find("^MD12") != std::string::npos);
        assert(zpl.find("^PR5") != std::string::npos);
        assert(zpl.find("^MNM") != std::string::npos);
        assert(zpl.find("^FWN") != std::string::npos);
        assert(zpl.find("^FO14,15\n^A0N,28,24") != std::string::npos);
        assert(zpl.find("^BCN,48,Y,N,N") != std::string::npos);
        assert(zpl.find("^B3N,N,35,N,N") != std::string::npos);
        assert(zpl.find("^BQN,2,3") != std::string::npos);
        assert(zpl.find("PART\\5EA\\7E100") != std::string::npos);
        assert(zpl.rfind("^XZ\n") == zpl.size() - 4);
    }

    void VariableResolverHandlesBuiltInsAndSerialFormatting()
    {
        LabelElement element;
        element.source = FieldSource::SerialNumber;
        element.serialWidth = 5;
        element.prefix = "SN-";
        element.suffix = "-A";

        VariableContext context;
        context.serialNumber = 17;
        context.recordIndex = 3;
        context.values["ItemNumber"] = "A100";
        context.values["Number"] = "A100";
        context.values["Description"] = "Bracket";

        assert(VariableResolver::elementValue(element, context) == "SN-00017-A");
        assert(VariableResolver::resolveText("{ItemNumber}-{Serial}-{RecordIndex}", context) == "A100-17-3");
        assert(VariableResolver::resolveText("{Number}-{Description}", context) == "A100-Bracket");
        assert(!VariableResolver::resolveText("{Date}", context).empty());
        assert(VariableResolver::findPlaceholders("A {ItemNumber} {Lot}").size() == 2);

        context.values["Order id"] = "1001";
        assert(VariableResolver::resolveText("{Order id}-{ItemNumber}", context) == "1001-A100");
        assert(VariableResolver::findPlaceholders("A {Order id} {ItemNumber}").size() == 2);
    }

    void SerialRangeGeneratesAscendingAndDescendingJobs()
    {
        LabelTemplate label = LabelTemplate::defaultTemplate();
        const auto ascending = ZplGenerator::generateSerialRange(label, 1, 3, 1);
        const auto descending = ZplGenerator::generateSerialRange(label, 3, 1, 1);

        assert(ascending.size() == 3);
        assert(descending.size() == 3);
        assert(ascending[0].find("^XA") != std::string::npos);
        assert(descending[0].find("^XA") != std::string::npos);
    }

    void CsvImporterDetectsHeadersAndMapsRows()
    {
        const std::string content =
            "Number,Description\n"
            "A100,\"Bracket, Left\"\n"
            "B200,Spacer\n";

        CsvData data = CsvImporter::parse(content, true);
        assert(data.hasHeader);
        assert(data.headers.size() == 2);
        assert(data.rows.size() == 2);
        assert(data.rows[0][1] == "Bracket, Left");

        const auto variables = data.rowAsVariables(0, {{"Number", "Number"}, {"Description", "Description"}});
        assert(variables.at("Number") == "A100");
        assert(variables.at("Description") == "Bracket, Left");
    }

    void TemplateStorageRoundTripsVersionFiveTemplate()
    {
        LabelTemplate label = LabelTemplate::defaultTemplate();
        label.name = "Round Trip";
        label.settings.dpi = 300;
        label.settings.mediaSensing = MediaSensingMode::Continuous;
        label.settings.orientation = LabelOrientation::Landscape;
        LabelElement element;
        element.id = "round_trip_text";
        element.name = "Round Trip Text";
        element.text = "Round Trip";
        label.elements.push_back(element);
        label.elements[0].alignment = TextAlignment::Center;
        label.elements[0].wrap = true;
        label.elements[0].maxLines = 2;

        const std::filesystem::path path = std::filesystem::temp_directory_path() / "label_printer_v5_template_test.json";
        std::string error;
        assert(TemplateStorage::save(label, path.string(), error));

        const LabelTemplate loaded = TemplateStorage::load(path.string());
        std::filesystem::remove(path);

        assert(loaded.name == "Round Trip");
        assert(loaded.settings.dpi == 300);
        assert(loaded.settings.mediaSensing == MediaSensingMode::Continuous);
        assert(loaded.settings.orientation == LabelOrientation::Landscape);
        assert(loaded.elements.size() == label.elements.size());
        assert(loaded.elements[0].alignment == TextAlignment::Center);
        assert(loaded.elements[0].wrap);
        assert(loaded.elements[0].maxLines == 2);
    }

    void MissingOrInvalidTemplatesFallBackToDefault()
    {
        const LabelTemplate missing = TemplateStorage::load("missing-template-for-test.json");
        assert(missing.elements.empty());

        const std::filesystem::path path = std::filesystem::temp_directory_path() / "bad_label_printer_template_test.json";
        {
            std::ofstream file(path);
            file << "{bad json";
        }

        const LabelTemplate invalid = TemplateStorage::load(path.string());
        std::filesystem::remove(path);
        assert(invalid.elements.empty());
    }
}

int main()
{
    GeneratedZplIncludesSettingsAndVersionFiveElements();
    VariableResolverHandlesBuiltInsAndSerialFormatting();
    SerialRangeGeneratesAscendingAndDescendingJobs();
    CsvImporterDetectsHeadersAndMapsRows();
    TemplateStorageRoundTripsVersionFiveTemplate();
    MissingOrInvalidTemplatesFallBackToDefault();
    return 0;
}
