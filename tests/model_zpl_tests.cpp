#ifdef NDEBUG
#undef NDEBUG
#endif

#include <cassert>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <string>

#include "core/BarcodeMetrics.h"
#include "core/CsvImporter.h"
#include "core/LabelLayout.h"
#include "core/LabelTemplate.h"
#include "core/LabelUnits.h"
#include "core/SampleData.h"
#include "core/TemplateStorage.h"
#include "core/VariableResolver.h"
#include "core/ZplGenerator.h"

namespace
{
    bool nearlyEqual(double a, double b, double tolerance = 0.001)
    {
        return std::abs(a - b) <= tolerance;
    }

    int occurrenceCount(const std::string& haystack, const std::string& needle)
    {
        int count = 0;
        std::size_t pos = 0;
        while ((pos = haystack.find(needle, pos)) != std::string::npos)
        {
            ++count;
            pos += needle.size();
        }
        return count;
    }

    LabelTemplate twoLineRegressionLabel()
    {
        LabelTemplate label = LabelTemplate::defaultTemplate();
        label.settings.dpi = 203;
        label.settings.labelWidthInches = 2.25;
        label.settings.labelHeightInches = 0.75;
        label.settings.marginLeftInches = 0.0;
        label.settings.marginTopInches = 0.0;

        LabelElement top;
        top.id = "text_align_top";
        top.name = "Align Top";
        top.type = LabelElementType::Text;
        top.text = "Align Top";
        top.xInches = 25.0 / 203.0;
        top.yInches = 20.0 / 203.0;
        top.boxWidthInches = 350.0 / 203.0;
        top.fontHeightDots = 42;
        top.fontWidthDots = 32;
        label.elements.push_back(top);

        LabelElement middle;
        middle.id = "text_align_middle";
        middle.name = "Align Middle";
        middle.type = LabelElementType::Text;
        middle.text = "Align Middle";
        middle.xInches = 25.0 / 203.0;
        middle.yInches = 80.0 / 203.0;
        middle.boxWidthInches = 350.0 / 203.0;
        middle.fontHeightDots = 42;
        middle.fontWidthDots = 32;
        label.elements.push_back(middle);

        return label;
    }

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
        assert(zpl.find("^FO14,8\n^A0N,28,24") != std::string::npos);
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
        assert(VariableResolver::resolveText("{DateTime}", context).find("Sample") == std::string::npos);
        assert(VariableResolver::findPlaceholders("A {ItemNumber} {Lot}").size() == 2);

        context.values["Order id"] = "1001";
        assert(VariableResolver::resolveText("{Order id}-{ItemNumber}", context) == "1001-A100");
        assert(VariableResolver::findPlaceholders("A {Order id} {ItemNumber}").size() == 2);
    }

    void BarcodeMetricsMatchZebraCode128Width()
    {
        LabelElement element;
        element.type = LabelElementType::Code128Barcode;
        element.barcodeModuleWidth = 2;

        assert(BarcodeMetrics::moduleCount(element, "TEST-001") == 123);
        assert(BarcodeMetrics::barcodeWidthDots(element, "TEST-001") == 246);

        VariableContext context;
        element.text = "{Number}";
        SampleData::fillMissingForElement(element, context);
        assert(VariableResolver::elementValue(element, context) == "TEST-001");
    }

    void SampleDataLeavesDateTimeBuiltInsLive()
    {
        LabelElement element;
        element.type = LabelElementType::Text;
        element.text = "{DateTime}";
        element.source = FieldSource::Fixed;

        VariableContext context;
        SampleData::fillMissingForElement(element, context);
        assert(context.values.find("DateTime") == context.values.end());

        const std::string value = VariableResolver::elementValue(element, context);
        assert(value.find("Sample") == std::string::npos);
        assert(value.find("-") != std::string::npos);
        assert(value.find(":") != std::string::npos);
    }

    void CenteredCode128BarcodeUsesActualPrintedValueWidth()
    {
        LabelTemplate label = LabelTemplate::defaultTemplate();
        label.settings.dpi = 203;
        label.settings.labelWidthInches = 2.25;
        label.settings.labelHeightInches = 0.75;

        LabelElement barcode;
        barcode.type = LabelElementType::Code128Barcode;
        barcode.source = FieldSource::Variable;
        barcode.text = "{Number}";
        barcode.variableName = "Number";
        barcode.xInches = 0.0;
        barcode.yInches = 0.1;
        barcode.boxWidthInches = 2.25;
        barcode.alignment = TextAlignment::Center;
        barcode.barcodeModuleWidth = 2;
        barcode.barcodeHeightDots = 48;
        label.elements.push_back(barcode);

        VariableContext context;
        context.values["Number"] = "11111122222";
        const std::string zpl = ZplGenerator::generate(label, context);

        assert(zpl.find("^FO72,20") != std::string::npos);
        assert(zpl.find("^FD>:11111122222") != std::string::npos);
    }

    void ManualTextPrintSizeMatchesDesignerIntent()
    {
        LabelTemplate label = LabelTemplate::defaultTemplate();
        label.settings.dpi = 203;

        LabelElement text;
        text.type = LabelElementType::Text;
        text.text = "Sample Text";
        text.xInches = 0.16;
        text.yInches = 0.12;
        text.boxWidthInches = 1.85;
        text.fontHeightDots = 62;
        text.fontWidthDots = 46;
        label.elements.push_back(text);

        const std::string zpl = ZplGenerator::generate(label);

        assert(zpl.find("^FO32,24\n^A0N,62,46") != std::string::npos);
        assert(zpl.find("^FDSample Text") != std::string::npos);
    }

    void ImportedRecordValuesAreSubstitutedIntoZpl()
    {
        LabelTemplate label = LabelTemplate::defaultTemplate();
        label.settings.labelWidthInches = 2.25;
        label.settings.labelHeightInches = 0.75;

        LabelElement number;
        number.type = LabelElementType::Text;
        number.source = FieldSource::Variable;
        number.text = "{Number}";
        number.variableName = "Number";
        number.xInches = 0.25;
        number.yInches = 0.08;
        number.boxWidthInches = 1.75;
        number.fontHeightDots = 72;
        number.fontWidthDots = 54;
        number.alignment = TextAlignment::Center;
        label.elements.push_back(number);

        LabelElement description;
        description.type = LabelElementType::Text;
        description.source = FieldSource::Variable;
        description.text = "{Description}";
        description.variableName = "Description";
        description.xInches = 0.1;
        description.yInches = 0.48;
        description.boxWidthInches = 2.05;
        description.fontHeightDots = 26;
        description.fontWidthDots = 20;
        description.alignment = TextAlignment::Center;
        label.elements.push_back(description);

        VariableContext context;
        context.values["Number"] = "164";
        context.values["Description"] = "Basic Short -60/64";

        const std::string zpl = ZplGenerator::generate(label, context);
        assert(zpl.find("^FD164^FS") != std::string::npos);
        assert(zpl.find("^FDBasic Short -60/64^FS") != std::string::npos);
        assert(zpl.find("^FO51,16\n^A0N,72,54") != std::string::npos);
        assert(zpl.find("^FO20,97\n^A0N,26,20") != std::string::npos);
    }

    void CoordinateConversionsAreStable()
    {
        assert(nearlyEqual(LabelUnits::inchesToDots(4.0, 203), 812.0));
        assert(nearlyEqual(LabelUnits::dotsToInches(812.0, 203), 4.0));
        assert(nearlyEqual(LabelUnits::mmToDots(25.4, 300), 300.0));
        assert(nearlyEqual(LabelUnits::dotsToMm(300.0, 300), 25.4));
    }

    void LayoutUsesCanonicalDotBounds()
    {
        LabelTemplate label = LabelTemplate::defaultTemplate();
        label.settings.dpi = 203;
        label.settings.labelWidthInches = 4.0;
        label.settings.labelHeightInches = 2.0;

        LabelElement text;
        text.type = LabelElementType::Text;
        text.text = "A";
        text.xInches = 100.0 / 203.0;
        text.yInches = 50.0 / 203.0;
        text.boxWidthInches = 203.0 / 203.0;
        text.fontHeightDots = 42;
        text.fontWidthDots = 32;
        label.elements.push_back(text);

        const ResolvedLabelLayout layout = LabelLayoutEngine::resolve(label);
        assert(nearlyEqual(layout.labelWidthDots, 812.0));
        assert(nearlyEqual(layout.labelHeightDots, 406.0));
        assert(layout.elements.size() == 1);
        assert(nearlyEqual(layout.elements[0].contentBoundsDots.x, 100.0));
        assert(nearlyEqual(layout.elements[0].contentBoundsDots.y, 50.0));
        assert(nearlyEqual(layout.elements[0].contentBoundsDots.width, 203.0));
        assert(nearlyEqual(layout.elements[0].contentBoundsDots.height, 42.0));
    }

    void ZplOutputUsesCanonicalCoordinates()
    {
        LabelTemplate label = LabelTemplate::defaultTemplate();
        label.settings.dpi = 203;

        LabelElement text;
        text.type = LabelElementType::Text;
        text.text = "Dot Position";
        text.xInches = 100.0 / 203.0;
        text.yInches = 50.0 / 203.0;
        text.boxWidthInches = 1.5;
        text.fontHeightDots = 42;
        text.fontWidthDots = 32;
        label.elements.push_back(text);

        const std::string zpl = ZplGenerator::generate(label);
        assert(zpl.find("^FO100,50\n^A0N,42,32") != std::string::npos);
    }

    void SavedTemplatePreservesCanonicalGeometry()
    {
        LabelTemplate label = LabelTemplate::defaultTemplate();
        label.settings.dpi = 203;
        LabelElement text;
        text.type = LabelElementType::Text;
        text.text = "Saved";
        text.xInches = 100.0 / 203.0;
        text.yInches = 50.0 / 203.0;
        text.boxWidthInches = 203.0 / 203.0;
        text.fontHeightDots = 42;
        text.fontWidthDots = 32;
        label.elements.push_back(text);

        const std::filesystem::path path = std::filesystem::temp_directory_path() / "label_printer_geometry_roundtrip.json";
        std::string error;
        assert(TemplateStorage::save(label, path.string(), error));
        const LabelTemplate loaded = TemplateStorage::load(path.string());
        std::filesystem::remove(path);

        const ResolvedLabelLayout layout = LabelLayoutEngine::resolve(loaded);
        assert(layout.elements.size() == 1);
        assert(nearlyEqual(layout.elements[0].contentBoundsDots.x, 100.0));
        assert(nearlyEqual(layout.elements[0].contentBoundsDots.y, 50.0));
        assert(nearlyEqual(layout.elements[0].contentBoundsDots.width, 203.0));
    }

    void TextPrintParityIndependentElementsKeepIndependentY()
    {
        const LabelTemplate label = twoLineRegressionLabel();
        const ResolvedLabelLayout layout = LabelLayoutEngine::resolve(label);
        assert(layout.textLayouts.size() == 2);
        assert(nearlyEqual(layout.textLayouts[0].yDots, 20.0));
        assert(nearlyEqual(layout.textLayouts[1].yDots, 80.0));
        assert(!nearlyEqual(layout.textLayouts[0].yDots, layout.textLayouts[1].yDots));

        const std::string zpl = ZplGenerator::generate(label);
        assert(zpl.find("^PW457") != std::string::npos);
        assert(zpl.find("^LL152") != std::string::npos);
        assert(zpl.find("^LH0,0") != std::string::npos);
        assert(zpl.find("^FWN") != std::string::npos);
        assert(zpl.find("^FO25,20\n^A0N,42,32\n^FH\\^FDAlign Top^FS") != std::string::npos);
        assert(zpl.find("^FO25,80\n^A0N,42,32\n^FH\\^FDAlign Middle^FS") != std::string::npos);
        assert(occurrenceCount(zpl, "^FDAlign Top") == 1);
        assert(occurrenceCount(zpl, "^FDAlign Middle") == 1);
    }

    void TextPrintParityCanvasZoomCannotAffectZpl()
    {
        const LabelTemplate label = twoLineRegressionLabel();
        const std::string zplBeforeAnyViewProjection = ZplGenerator::generate(label);
        const std::string zplAfterAnyViewProjection = ZplGenerator::generate(label);
        assert(zplBeforeAnyViewProjection == zplAfterAnyViewProjection);
    }

    void TextPrintParityFontHeightAndWidthUseDots()
    {
        const LabelTemplate label = twoLineRegressionLabel();
        const ResolvedLabelLayout layout = LabelLayoutEngine::resolve(label);
        assert(layout.textLayouts.size() == 2);
        for (const ResolvedTextLayout& text : layout.textLayouts)
        {
            assert(nearlyEqual(text.fontHeightDots, 42.0));
            assert(nearlyEqual(text.fontWidthDots, 32.0));
        }

        const std::string zpl = ZplGenerator::generate(label);
        assert(occurrenceCount(zpl, "^A0N,42,32") == 2);
    }

    void TextPrintParityZplUsesResolvedLayout()
    {
        const LabelTemplate label = twoLineRegressionLabel();
        const ResolvedLabelLayout layout = LabelLayoutEngine::resolve(label);
        const std::string zpl = ZplGenerator::generate(label);
        for (const ResolvedTextLayout& text : layout.textLayouts)
        {
            const std::string expectedFo = "^FO" +
                std::to_string(LabelUnits::roundDots(text.glyphOriginXDots)) + "," +
                std::to_string(LabelUnits::roundDots(text.glyphOriginYDots));
            assert(zpl.find(expectedFo) != std::string::npos);
        }
    }

    void TextPrintParityDebugReportShowsPerElementZpl()
    {
        const LabelTemplate label = twoLineRegressionLabel();
        const std::string report = ZplGenerator::generateDebugReport(label);
        assert(report.find("Element: Align Top") != std::string::npos);
        assert(report.find("Element: Align Middle") != std::string::npos);
        assert(report.find("xDots=25") != std::string::npos);
        assert(report.find("yDots=20") != std::string::npos);
        assert(report.find("yDots=80") != std::string::npos);
        assert(report.find("^FO25,20") != std::string::npos);
        assert(report.find("^FO25,80") != std::string::npos);
    }

    void TextPrintParityUsesFoTopLeftTextOrigin()
    {
        const LabelTemplate label = twoLineRegressionLabel();
        const std::string zpl = ZplGenerator::generate(label);
        assert(occurrenceCount(zpl, "^FO") >= 2);
        assert(zpl.find("^FT") == std::string::npos);
        assert(zpl.find("^LS") == std::string::npos);
        assert(zpl.find("^LT") == std::string::npos);
        assert(zpl.find("^LH0,0") != std::string::npos);
    }

    void FullLabelBorderPrintsInsideSafeEdges()
    {
        LabelTemplate label = LabelTemplate::defaultTemplate();
        label.settings.dpi = 203;
        label.settings.labelWidthInches = 2.25;
        label.settings.labelHeightInches = 0.75;

        LabelElement border;
        border.type = LabelElementType::Box;
        border.xInches = 0.0;
        border.yInches = 0.0;
        border.boxWidthInches = 2.25;
        border.fontHeightDots = 152;
        border.fontWidthDots = 3;
        label.elements.push_back(border);

        const std::string zpl = ZplGenerator::generate(label);
        assert(zpl.find("^FO8,8\n^GB441,136,3^FS") != std::string::npos);
        assert(zpl.find("^FO0,0\n^GB457,152,3^FS") == std::string::npos);
    }

    void RegressionTwoLargeTextElementsDoNotOverlapOnPrint()
    {
        const LabelTemplate label = twoLineRegressionLabel();
        const ResolvedLabelLayout layout = LabelLayoutEngine::resolve(label);
        assert(layout.textLayouts.size() == 2);
        const double firstBottom = layout.textLayouts[0].yDots + layout.textLayouts[0].boxHeightDots;
        assert(firstBottom < layout.textLayouts[1].yDots);

        const std::string zpl = ZplGenerator::generate(label);
        assert(zpl.find("^FO25,20") != std::string::npos);
        assert(zpl.find("^FO25,80") != std::string::npos);
        assert(zpl.find("Align Top\nAlign Middle") == std::string::npos);
    }

    void ImportedSingleLineTextDoesNotImplicitlyWrapInPreviewLayout()
    {
        LabelTemplate label = LabelTemplate::defaultTemplate();
        label.settings.dpi = 203;
        label.settings.labelWidthInches = 2.25;
        label.settings.labelHeightInches = 0.75;

        LabelElement text;
        text.type = LabelElementType::Text;
        text.source = FieldSource::Variable;
        text.variableName = "Status";
        text.xInches = 0.05;
        text.yInches = 0.1;
        text.boxWidthInches = 1.35;
        text.fontHeightDots = 64;
        text.fontWidthDots = 48;
        text.wrap = false;
        text.multiLine = false;
        text.maxLines = 2;
        label.elements.push_back(text);

        VariableContext context;
        context.values["Status"] = "Show 5 Defective";

        const ResolvedLabelLayout layout = LabelLayoutEngine::resolve(label, context);
        assert(layout.textLayouts.size() == 1);
        assert(layout.textLayouts[0].lines.size() == 1);
        assert(layout.textLayouts[0].lines[0] == "Show 5 Defective");

        const std::string zpl = ZplGenerator::generate(label, context);
        assert(zpl.find("^FB") == std::string::npos);
        assert(zpl.find("^FDShow 5 Defective^FS") != std::string::npos);
        assert(zpl.find("^FDShow 5^FS") == std::string::npos);
        assert(zpl.find("^FDDefective^FS") == std::string::npos);
    }

    void WrappedTextUsesSharedExplicitLineBreaksForPreviewAndZpl()
    {
        LabelTemplate label = LabelTemplate::defaultTemplate();
        label.settings.dpi = 203;
        label.settings.labelWidthInches = 2.25;
        label.settings.labelHeightInches = 0.75;

        LabelElement text;
        text.type = LabelElementType::Text;
        text.text = "Show 5 Defective";
        text.xInches = 25.0 / 203.0;
        text.yInches = 20.0 / 203.0;
        text.boxWidthInches = 250.0 / 203.0;
        text.fontHeightDots = 42;
        text.fontWidthDots = 60;
        text.wrap = true;
        text.multiLine = true;
        text.maxLines = 2;
        text.alignment = TextAlignment::Center;
        label.elements.push_back(text);

        const ResolvedLabelLayout layout = LabelLayoutEngine::resolve(label);
        assert(layout.textLayouts.size() == 1);
        assert(layout.textLayouts[0].lines.size() == 2);
        assert(layout.textLayouts[0].lines[0] == "Show 5");
        assert(layout.textLayouts[0].lines[1] == "Defective");

        const std::string zpl = ZplGenerator::generate(label);
        assert(zpl.find("^FO25,20\n^A0N,42,60\n^FB250,1,0,C,0\n^FH\\^FDShow 5^FS") != std::string::npos);
        assert(zpl.find("^FO25,62\n^A0N,42,60\n^FB250,1,0,C,0\n^FH\\^FDDefective^FS") != std::string::npos);
        assert(zpl.find("^FB250,2") == std::string::npos);
        assert(zpl.find("^FDShow 5 Defective^FS") == std::string::npos);
    }

    void TextAlignmentUsesFullLabelLaneForImportedValues()
    {
        LabelTemplate label = LabelTemplate::defaultTemplate();
        label.settings.labelWidthInches = 2.25;

        LabelElement text;
        text.type = LabelElementType::Text;
        text.source = FieldSource::Variable;
        text.text = "{Description}";
        text.variableName = "Description";
        text.xInches = 0.0;
        text.yInches = 0.2;
        text.boxWidthInches = 2.25;
        text.fontHeightDots = 26;
        text.fontWidthDots = 20;
        text.alignment = TextAlignment::Center;
        label.elements.push_back(text);

        VariableContext context;
        context.values["Description"] = "Basic Short -60/64";

        const std::string zpl = ZplGenerator::generate(label, context);
        assert(zpl.find("^FB457,1,0,C,0") != std::string::npos);
        assert(zpl.find("^FDBasic Short -60/64^FS") != std::string::npos);
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
    BarcodeMetricsMatchZebraCode128Width();
    SampleDataLeavesDateTimeBuiltInsLive();
    CenteredCode128BarcodeUsesActualPrintedValueWidth();
    ManualTextPrintSizeMatchesDesignerIntent();
    ImportedRecordValuesAreSubstitutedIntoZpl();
    TextAlignmentUsesFullLabelLaneForImportedValues();
    CoordinateConversionsAreStable();
    LayoutUsesCanonicalDotBounds();
    ZplOutputUsesCanonicalCoordinates();
    SavedTemplatePreservesCanonicalGeometry();
    TextPrintParityIndependentElementsKeepIndependentY();
    TextPrintParityCanvasZoomCannotAffectZpl();
    TextPrintParityFontHeightAndWidthUseDots();
    TextPrintParityZplUsesResolvedLayout();
    TextPrintParityDebugReportShowsPerElementZpl();
    TextPrintParityUsesFoTopLeftTextOrigin();
    FullLabelBorderPrintsInsideSafeEdges();
    RegressionTwoLargeTextElementsDoNotOverlapOnPrint();
    ImportedSingleLineTextDoesNotImplicitlyWrapInPreviewLayout();
    WrappedTextUsesSharedExplicitLineBreaksForPreviewAndZpl();
    SerialRangeGeneratesAscendingAndDescendingJobs();
    CsvImporterDetectsHeadersAndMapsRows();
    TemplateStorageRoundTripsVersionFiveTemplate();
    MissingOrInvalidTemplatesFallBackToDefault();
    return 0;
}
