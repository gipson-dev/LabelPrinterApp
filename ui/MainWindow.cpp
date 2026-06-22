#include "ui/MainWindow.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QListWidget>
#include <QMenuBar>
#include <QMessageBox>
#include <QModelIndex>
#include <QPushButton>
#include <QRadioButton>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QSplitter>
#include <QStatusBar>
#include <QStringList>
#include <QTabWidget>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTextEdit>
#include <QToolBar>
#include <QVBoxLayout>

#include <algorithm>
#include <set>

#include "core/TemplateStorage.h"
#include "core/VariableResolver.h"
#include "core/ZebraPrinter.h"
#include "core/ZplGenerator.h"
#include "ui/ElementEditorWidget.h"
#include "ui/ExcelRecordsWidget.h"
#include "ui/PreviewWidget.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      labelTemplate_(LabelTemplate::defaultTemplate())
{
    buildUi();
    loadDefaultTemplate();
    refreshPrinterList();
}

void MainWindow::buildUi()
{
    setWindowTitle("LabelPrinterApp");
    buildMenus();
    buildToolbar();

    tabs_ = new QTabWidget(this);
    tabs_->addTab(buildDesignTab(), "Design");
    tabs_->addTab(buildElementsTab(), "Elements");
    tabs_->addTab(buildDataTab(), "Data");
    tabs_->addTab(buildTemplatesTab(), "Templates");
    tabs_->addTab(buildPrintTab(), "Print");
    tabs_->addTab(buildSettingsTab(), "Settings");
    setCentralWidget(tabs_);

    connect(elementList_, &QListWidget::currentRowChanged, this, &MainWindow::selectElement);
    connect(editor_, &ElementEditorWidget::elementChanged, this, [this](const LabelElement& element) {
        if (selectedElement_ >= 0 && selectedElement_ < static_cast<int>(labelTemplate_.elements.size()))
        {
            labelTemplate_.elements[selectedElement_] = element;
            if (auto* item = elementList_->item(selectedElement_))
            {
                item->setText(QString::fromStdString(element.name));
            }
            refreshPreview();
        }
    });
    connect(preview_, &PreviewWidget::elementSelected, this, &MainWindow::selectElement);
    connect(preview_, &PreviewWidget::elementMoved, this, [this](int index, double x, double y) {
        if (index >= 0 && index < static_cast<int>(labelTemplate_.elements.size()))
        {
            labelTemplate_.elements[index].xInches = x;
            labelTemplate_.elements[index].yInches = y;
            selectElement(index);
            refreshPreview();
        }
    });

    const auto settingsChanged = [this] { updateTemplateFromSettings(); refreshPreview(); };
    for (QSpinBox* spin : {speedSpin_, darknessSpin_, copiesSpin_})
    {
        connect(spin, &QSpinBox::valueChanged, this, settingsChanged);
    }
    for (QDoubleSpinBox* spin : {widthSpin_, heightSpin_, marginLeftSpin_, marginTopSpin_, gapSpin_})
    {
        connect(spin, &QDoubleSpinBox::valueChanged, this, settingsChanged);
    }
    connect(dpiCombo_, &QComboBox::currentIndexChanged, this, settingsChanged);
    connect(mediaCombo_, &QComboBox::currentIndexChanged, this, settingsChanged);
    connect(orientationCombo_, &QComboBox::currentIndexChanged, this, settingsChanged);
    connect(printerCombo_, &QComboBox::currentIndexChanged, this, settingsChanged);
    connect(stockPresetCombo_, &QComboBox::currentIndexChanged, this, &MainWindow::applyStockPreset);
}

void MainWindow::buildMenus()
{
    auto* file = menuBar()->addMenu("File");
    file->addAction("New", this, &MainWindow::newTemplate);
    file->addAction("Open", this, &MainWindow::loadTemplate);
    file->addAction("Save", this, &MainWindow::saveTemplate);
    file->addSeparator();
    file->addAction("Exit", this, &QWidget::close);

    auto* edit = menuBar()->addMenu("Edit");
    edit->addAction("Duplicate Element", this, &MainWindow::duplicateSelectedElement);
    edit->addAction("Delete Element", this, &MainWindow::deleteSelectedElement);
    edit->addAction("Move Up", this, [this] { moveSelectedElement(-1); });
    edit->addAction("Move Down", this, [this] { moveSelectedElement(1); });

    auto* view = menuBar()->addMenu("View");
    view->addAction("Preview ZPL", this, &MainWindow::previewZpl);

    auto* printer = menuBar()->addMenu("Printer");
    printer->addAction("Refresh Printers", this, &MainWindow::refreshPrinterList);
    printer->addAction("Print Test Label", this, &MainWindow::printTestLabel);
    printer->addAction("Print Labels", this, &MainWindow::printCurrent);

    auto* templates = menuBar()->addMenu("Templates");
    templates->addAction("Load Template", this, &MainWindow::loadTemplate);
    templates->addAction("Save Template", this, &MainWindow::saveTemplate);

    auto* help = menuBar()->addMenu("Help");
    help->addAction("About", this, [this] {
        QMessageBox::about(this, "About LabelPrinterApp", "LabelPrinterApp designs and prints Zebra ZPL labels.");
    });
}

void MainWindow::buildToolbar()
{
    auto* toolbar = addToolBar("Main");
    toolbar->addAction("New", this, &MainWindow::newTemplate);
    toolbar->addAction("Open", this, &MainWindow::loadTemplate);
    toolbar->addAction("Save", this, &MainWindow::saveTemplate);
    toolbar->addSeparator();
    toolbar->addAction("Print", this, &MainWindow::printCurrent);
    toolbar->addAction("Preview", this, &MainWindow::previewZpl);
    toolbar->addAction("Test Label", this, &MainWindow::printTestLabel);
}

QWidget* MainWindow::buildDesignTab()
{
    auto* tab = new QWidget(this);
    auto* layout = new QHBoxLayout(tab);

    auto* toolbox = new QGroupBox("Toolbox", tab);
    auto* toolboxLayout = new QVBoxLayout(toolbox);
    auto addToolButton = [this, toolboxLayout](const QString& text, LabelElementType type) {
        auto* button = new QPushButton(text, this);
        toolboxLayout->addWidget(button);
        connect(button, &QPushButton::clicked, this, [this, type] { addElement(type); });
    };
    addToolButton("Text / Number", LabelElementType::Text);
    addToolButton("Barcode", LabelElementType::Code128Barcode);
    addToolButton("QR Code", LabelElementType::QrCode);
    auto* dateButton = new QPushButton("Date/Time", toolbox);
    toolboxLayout->addWidget(dateButton);
    connect(dateButton, &QPushButton::clicked, this, [this] {
        addElement(LabelElementType::Text);
        if (!labelTemplate_.elements.empty())
        {
            LabelElement& e = labelTemplate_.elements.back();
            e.name = "Date Time";
            e.text = "{Date} {Time}";
            e.source = FieldSource::Fixed;
            e.fontHeightDots = 36;
            e.fontWidthDots = 28;
            e.bold = false;
            refreshElementList();
            selectElement(static_cast<int>(labelTemplate_.elements.size() - 1));
            refreshPreview();
        }
    });
    auto* serialButton = new QPushButton("Serial #", toolbox);
    toolboxLayout->addWidget(serialButton);
    connect(serialButton, &QPushButton::clicked, this, [this] {
        addElement(LabelElementType::Text);
        if (!labelTemplate_.elements.empty())
        {
            LabelElement& e = labelTemplate_.elements.back();
            e.name = "Serial Number";
            e.text = "{Serial}";
            e.source = FieldSource::SerialNumber;
            e.serialWidth = 4;
            e.fontHeightDots = 64;
            e.fontWidthDots = 48;
            e.bold = true;
            refreshElementList();
            selectElement(static_cast<int>(labelTemplate_.elements.size() - 1));
            refreshPreview();
        }
    });
    toolboxLayout->addWidget(new QPushButton("Line", toolbox));
    toolboxLayout->addWidget(new QPushButton("Box", toolbox));
    toolboxLayout->addStretch();

    preview_ = new PreviewWidget(tab);

    auto* properties = new QGroupBox("Properties", tab);
    auto* propertiesLayout = new QVBoxLayout(properties);
    editor_ = new ElementEditorWidget(properties);
    propertiesLayout->addWidget(editor_);

    layout->addWidget(toolbox, 0);
    layout->addWidget(preview_, 1);
    layout->addWidget(properties, 0);
    return tab;
}

QWidget* MainWindow::buildElementsTab()
{
    auto* tab = new QWidget(this);
    auto* layout = new QVBoxLayout(tab);
    auto* content = new QHBoxLayout;

    auto* listGroup = new QGroupBox("Element List", tab);
    auto* listLayout = new QVBoxLayout(listGroup);
    elementList_ = new QListWidget(listGroup);
    listLayout->addWidget(elementList_);

    auto* buttons = new QHBoxLayout;
    auto* addButton = new QPushButton("Add", listGroup);
    auto* duplicateButton = new QPushButton("Duplicate", listGroup);
    auto* deleteButton = new QPushButton("Delete", listGroup);
    auto* upButton = new QPushButton("Move Up", listGroup);
    auto* downButton = new QPushButton("Move Down", listGroup);
    buttons->addWidget(addButton);
    buttons->addWidget(duplicateButton);
    buttons->addWidget(deleteButton);
    buttons->addWidget(upButton);
    buttons->addWidget(downButton);
    listLayout->addLayout(buttons);

    connect(addButton, &QPushButton::clicked, this, [this] { addElement(LabelElementType::Text); });
    connect(duplicateButton, &QPushButton::clicked, this, &MainWindow::duplicateSelectedElement);
    connect(deleteButton, &QPushButton::clicked, this, &MainWindow::deleteSelectedElement);
    connect(upButton, &QPushButton::clicked, this, [this] { moveSelectedElement(-1); });
    connect(downButton, &QPushButton::clicked, this, [this] { moveSelectedElement(1); });

    auto* settingsGroup = new QGroupBox("Element Settings", tab);
    auto* settingsLayout = new QVBoxLayout(settingsGroup);
    settingsLayout->addWidget(new QLabel("Use the Design tab properties panel for exact element values, position, font, rotation, and barcode settings.", settingsGroup));
    settingsLayout->addStretch();

    content->addWidget(listGroup, 1);
    content->addWidget(settingsGroup, 1);
    layout->addLayout(content);
    return tab;
}

QWidget* MainWindow::buildDataTab()
{
    auto* tab = new QWidget(this);
    auto* layout = new QVBoxLayout(tab);
    excelRecords_ = new ExcelRecordsWidget(tab);
    layout->addWidget(excelRecords_);
    connect(excelRecords_, &ExcelRecordsWidget::previewRowRequested, this, [this](int row) {
        preview_->setVariables(contextForRow(row));
    });
    connect(excelRecords_, &ExcelRecordsWidget::recordsChanged, this, [this] {
        csvMappingOverrides_.clear();
    });
    connect(excelRecords_, &ExcelRecordsWidget::printSelectedRequested, this, &MainWindow::printSelectedCsvRows);
    return tab;
}

QWidget* MainWindow::buildPrintTab()
{
    auto* tab = new QWidget(this);
    auto* layout = new QVBoxLayout(tab);

    auto* job = new QGroupBox("Print Job", tab);
    auto* jobLayout = new QFormLayout(job);
    copiesSpin_ = new QSpinBox(job);
    copiesSpin_->setRange(1, 9999);
    serialStartSpin_ = new QSpinBox(job);
    serialStartSpin_->setRange(0, 999999999);
    serialStartSpin_->setValue(1);
    serialEndSpin_ = new QSpinBox(job);
    serialEndSpin_->setRange(0, 999999999);
    serialEndSpin_->setValue(1);
    jobLayout->addRow("Copies", copiesSpin_);
    jobLayout->addRow("Serial Start", serialStartSpin_);
    jobLayout->addRow("Serial End", serialEndSpin_);
    layout->addWidget(job);

    auto* options = new QGroupBox("Options", tab);
    auto* optionsLayout = new QVBoxLayout(options);
    optionsLayout->addWidget(new QCheckBox("Print all CSV rows", options));
    optionsLayout->addWidget(new QCheckBox("Print selected CSV rows only", options));
    auto* quantityCheck = new QCheckBox("Use quantity column", options);
    quantityCheck->setChecked(true);
    optionsLayout->addWidget(quantityCheck);
    optionsLayout->addWidget(new QCheckBox("Show ZPL before printing", options));
    layout->addWidget(options);

    auto* buttons = new QHBoxLayout;
    auto* testButton = new QPushButton("Print Test Label", tab);
    auto* zplButton = new QPushButton("Preview ZPL", tab);
    auto* printButton = new QPushButton("Print Labels", tab);
    auto* selectedButton = new QPushButton("Print Selected CSV", tab);
    auto* allButton = new QPushButton("Print All CSV", tab);
    buttons->addWidget(testButton);
    buttons->addWidget(zplButton);
    buttons->addWidget(printButton);
    buttons->addWidget(selectedButton);
    buttons->addWidget(allButton);
    buttons->addStretch();
    layout->addLayout(buttons);
    layout->addStretch();

    connect(testButton, &QPushButton::clicked, this, &MainWindow::printTestLabel);
    connect(zplButton, &QPushButton::clicked, this, &MainWindow::previewZpl);
    connect(printButton, &QPushButton::clicked, this, &MainWindow::printCurrent);
    connect(selectedButton, &QPushButton::clicked, this, &MainWindow::printSelectedCsvRows);
    connect(allButton, &QPushButton::clicked, this, &MainWindow::printAllCsvRows);
    return tab;
}

QWidget* MainWindow::buildTemplatesTab()
{
    auto* tab = new QWidget(this);
    auto* layout = new QVBoxLayout(tab);
    templateList_ = new QListWidget(tab);
    layout->addWidget(new QLabel("Template Library", tab));
    layout->addWidget(templateList_, 1);

    auto* buttons = new QHBoxLayout;
    auto* newButton = new QPushButton("New Template", tab);
    auto* loadButton = new QPushButton("Load", tab);
    auto* saveButton = new QPushButton("Save As", tab);
    auto* deleteButton = new QPushButton("Delete", tab);
    auto* defaultButton = new QPushButton("Set Default", tab);
    buttons->addWidget(newButton);
    buttons->addWidget(loadButton);
    buttons->addWidget(saveButton);
    buttons->addWidget(deleteButton);
    buttons->addWidget(defaultButton);
    buttons->addStretch();
    layout->addLayout(buttons);

    connect(newButton, &QPushButton::clicked, this, &MainWindow::newTemplate);
    connect(loadButton, &QPushButton::clicked, this, &MainWindow::loadSelectedLibraryTemplate);
    connect(saveButton, &QPushButton::clicked, this, &MainWindow::saveTemplate);
    connect(deleteButton, &QPushButton::clicked, this, [this] {
        QMessageBox::information(this, "Template Library", "Template deletion from the library will be added with persistent template indexing.");
    });
    connect(defaultButton, &QPushButton::clicked, this, [this] {
        QMessageBox::information(this, "Template Library", "The current template can be saved as templates/default_label.json to make it the startup default.");
    });
    connect(templateList_, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem*) { loadSelectedLibraryTemplate(); });
    refreshTemplateLibrary();
    return tab;
}

QWidget* MainWindow::buildSettingsTab()
{
    auto* tab = new QWidget(this);
    auto* layout = new QVBoxLayout(tab);
    layout->addWidget(buildPrinterSettingsPanel(tab));

    auto* buttons = new QHBoxLayout;
    auto* saveButton = new QPushButton("Save Settings", tab);
    auto* resetButton = new QPushButton("Reset Defaults", tab);
    buttons->addWidget(saveButton);
    buttons->addWidget(resetButton);
    buttons->addStretch();
    layout->addLayout(buttons);
    layout->addStretch();

    connect(saveButton, &QPushButton::clicked, this, [this] {
        QMessageBox::information(this, "Settings", "Persistent application settings will be added in the final polish phase.");
    });
    connect(resetButton, &QPushButton::clicked, this, &MainWindow::newTemplate);
    return tab;
}

QWidget* MainWindow::buildPrinterSettingsPanel(QWidget* parent)
{
    auto* group = new QGroupBox("Label Stock", parent);
    auto* settingsLayout = new QFormLayout(group);

    stockPresetCombo_ = new QComboBox(group);
    stockPresetCombo_->addItems({
        "Custom",
        "Uline S-8599 - 2.25\" x 0.75\" Direct Thermal",
        "Uline S-22422 - 2.25\" x 0.75\" Removable Direct Thermal",
        "Zebra 2.25\" x 0.75\" Generic",
        "Zebra ZD620 - 4\" x 2\" Direct Thermal"
    });
    printerCombo_ = new QComboBox(group);
    dpiCombo_ = new QComboBox(group);
    dpiCombo_->addItem("203", 203);
    dpiCombo_->addItem("300", 300);
    widthSpin_ = new QDoubleSpinBox(group);
    heightSpin_ = new QDoubleSpinBox(group);
    marginLeftSpin_ = new QDoubleSpinBox(group);
    marginTopSpin_ = new QDoubleSpinBox(group);
    gapSpin_ = new QDoubleSpinBox(group);
    for (QDoubleSpinBox* spin : {widthSpin_, heightSpin_, marginLeftSpin_, marginTopSpin_, gapSpin_})
    {
        spin->setDecimals(3);
        spin->setSuffix(" in");
        spin->setSingleStep(0.01);
    }
    widthSpin_->setRange(0.1, 10.0);
    heightSpin_->setRange(0.1, 10.0);
    marginLeftSpin_->setRange(0.0, 10.0);
    marginTopSpin_->setRange(0.0, 10.0);
    gapSpin_->setRange(0.0, 10.0);
    mediaCombo_ = new QComboBox(group);
    mediaCombo_->addItems({"Gap Labels", "Black Mark Labels", "Continuous Media"});
    printMethodCombo_ = new QComboBox(group);
    printMethodCombo_->addItems({"Direct Thermal", "Thermal Transfer"});
    coreSizeCombo_ = new QComboBox(group);
    coreSizeCombo_->addItems({"1.0\"", "0.5\"", "3.0\""});
    widthDotsLabel_ = new QLabel(group);
    heightDotsLabel_ = new QLabel(group);
    orientationCombo_ = new QComboBox(group);
    orientationCombo_->addItems({"Portrait", "Landscape"});
    speedSpin_ = new QSpinBox(group);
    speedSpin_->setRange(1, 14);
    darknessSpin_ = new QSpinBox(group);
    darknessSpin_->setRange(0, 30);
    settingsLayout->addRow("Preset", stockPresetCombo_);
    auto* printerRow = new QWidget(group);
    auto* printerRowLayout = new QHBoxLayout(printerRow);
    printerRowLayout->setContentsMargins(0, 0, 0, 0);
    auto* refreshPrintersButton = new QPushButton("Refresh", printerRow);
    printerRowLayout->addWidget(printerCombo_, 1);
    printerRowLayout->addWidget(refreshPrintersButton);
    settingsLayout->addRow("Default Printer", printerRow);
    settingsLayout->addRow("Width", widthSpin_);
    settingsLayout->addRow("Height", heightSpin_);
    settingsLayout->addRow("Media Type", mediaCombo_);
    settingsLayout->addRow("Print Method", printMethodCombo_);
    settingsLayout->addRow("Core Size", coreSizeCombo_);
    settingsLayout->addRow("DPI", dpiCombo_);
    settingsLayout->addRow("Calculated Width", widthDotsLabel_);
    settingsLayout->addRow("Calculated Height", heightDotsLabel_);
    settingsLayout->addRow("Left Margin", marginLeftSpin_);
    settingsLayout->addRow("Top Margin", marginTopSpin_);
    settingsLayout->addRow("Gap", gapSpin_);
    settingsLayout->addRow("Orientation", orientationCombo_);
    settingsLayout->addRow("Speed", speedSpin_);
    settingsLayout->addRow("Darkness", darknessSpin_);
    connect(refreshPrintersButton, &QPushButton::clicked, this, &MainWindow::refreshPrinterList);
    updateCalculatedSizeLabels();
    return group;
}

void MainWindow::applyStockPreset(int index)
{
    if (index <= 0)
    {
        updateCalculatedSizeLabels();
        return;
    }

    QSignalBlocker widthBlocker(widthSpin_);
    QSignalBlocker heightBlocker(heightSpin_);
    QSignalBlocker mediaBlocker(mediaCombo_);
    QSignalBlocker methodBlocker(printMethodCombo_);
    QSignalBlocker coreBlocker(coreSizeCombo_);
    QSignalBlocker dpiBlocker(dpiCombo_);

    if (index == 4)
    {
        widthSpin_->setValue(4.0);
        heightSpin_->setValue(2.0);
    }
    else
    {
        widthSpin_->setValue(2.25);
        heightSpin_->setValue(0.75);
    }

    mediaCombo_->setCurrentIndex(static_cast<int>(MediaSensingMode::Gap));
    printMethodCombo_->setCurrentIndex(0);
    coreSizeCombo_->setCurrentIndex(0);
    int dpiIndex = dpiCombo_->findData(203);
    if (dpiIndex >= 0)
    {
        dpiCombo_->setCurrentIndex(dpiIndex);
    }

    updateTemplateFromSettings();
    refreshPreview();
}

void MainWindow::updateCalculatedSizeLabels()
{
    if (!widthDotsLabel_ || !heightDotsLabel_ || !dpiCombo_)
    {
        return;
    }

    const int dpi = dpiCombo_->currentData().toInt();
    const int widthDots = static_cast<int>(widthSpin_->value() * dpi + 0.5);
    const int heightDots = static_cast<int>(heightSpin_->value() * dpi + 0.5);
    widthDotsLabel_->setText(QString("%1 dots").arg(widthDots));
    heightDotsLabel_->setText(QString("%1 dots").arg(heightDots));
}

void MainWindow::refreshPrinterList()
{
    std::string error;
    const QString previous = printerCombo_->currentText();
    printerCombo_->clear();
    for (const std::string& printer : ZebraPrinter::installedPrinters(error))
    {
        printerCombo_->addItem(QString::fromStdString(printer));
    }
    int previousIndex = printerCombo_->findText(previous);
    if (previousIndex >= 0)
    {
        printerCombo_->setCurrentIndex(previousIndex);
    }
    if (!error.empty())
    {
        statusBar()->showMessage(QString::fromStdString(error));
    }
}

void MainWindow::refreshElementList()
{
    QSignalBlocker blocker(elementList_);
    elementList_->clear();
    for (const LabelElement& element : labelTemplate_.elements)
    {
        elementList_->addItem(QString::fromStdString(element.name));
    }
}

void MainWindow::refreshTemplateLibrary()
{
    if (!templateList_)
    {
        return;
    }

    templateList_->clear();
    QDir dir("templates");
    const QFileInfoList files = dir.entryInfoList({"*.json"}, QDir::Files, QDir::Name);
    for (const QFileInfo& file : files)
    {
        LabelTemplate label = TemplateStorage::load(file.filePath().toStdString());
        auto* item = new QListWidgetItem(QString::fromStdString(label.name), templateList_);
        item->setData(Qt::UserRole, file.filePath());
        item->setToolTip(file.filePath());
    }
}

void MainWindow::refreshSettingsControls()
{
    const PrinterSettings s = labelTemplate_.settings;
    QSignalBlocker presetBlocker(stockPresetCombo_);
    QSignalBlocker dpiBlocker(dpiCombo_);
    QSignalBlocker widthBlocker(widthSpin_);
    QSignalBlocker heightBlocker(heightSpin_);
    QSignalBlocker marginLeftBlocker(marginLeftSpin_);
    QSignalBlocker marginTopBlocker(marginTopSpin_);
    QSignalBlocker gapBlocker(gapSpin_);
    QSignalBlocker mediaBlocker(mediaCombo_);
    QSignalBlocker orientationBlocker(orientationCombo_);
    QSignalBlocker speedBlocker(speedSpin_);
    QSignalBlocker darknessBlocker(darknessSpin_);
    QSignalBlocker copiesBlocker(copiesSpin_);

    int presetIndex = 0;
    if (std::abs(s.labelWidthInches - 4.0) < 0.001 && std::abs(s.labelHeightInches - 2.0) < 0.001)
    {
        presetIndex = 4;
    }
    else if (std::abs(s.labelWidthInches - 2.25) < 0.001 && std::abs(s.labelHeightInches - 0.75) < 0.001)
    {
        presetIndex = 1;
    }
    stockPresetCombo_->setCurrentIndex(presetIndex);

    int dpiIndex = dpiCombo_->findData(s.dpi);
    if (dpiIndex < 0)
    {
        dpiCombo_->addItem(QString::number(s.dpi), s.dpi);
        dpiIndex = dpiCombo_->findData(s.dpi);
    }
    dpiCombo_->setCurrentIndex(dpiIndex);
    widthSpin_->setValue(std::max(0.1, s.labelWidthInches));
    heightSpin_->setValue(std::max(0.1, s.labelHeightInches));
    marginLeftSpin_->setValue(s.marginLeftInches);
    marginTopSpin_->setValue(s.marginTopInches);
    gapSpin_->setValue(s.gapInches);
    mediaCombo_->setCurrentIndex(static_cast<int>(s.mediaSensing));
    orientationCombo_->setCurrentIndex(static_cast<int>(s.orientation));
    speedSpin_->setValue(s.speedIps);
    darknessSpin_->setValue(s.darkness);
    copiesSpin_->setValue(s.copies);
    int printerIndex = printerCombo_->findText(QString::fromStdString(s.printerName));
    if (printerIndex >= 0)
    {
        printerCombo_->setCurrentIndex(printerIndex);
    }
    updateCalculatedSizeLabels();
}

void MainWindow::refreshPreview()
{
    preview_->setTemplate(labelTemplate_);
    preview_->setSelectedElement(selectedElement_);
}

void MainWindow::loadDefaultTemplate()
{
    labelTemplate_ = TemplateStorage::load("templates/default_label.json");
    refreshSettingsControls();
    refreshElementList();
    selectElement(labelTemplate_.elements.empty() ? -1 : 0);
    refreshPreview();
}

void MainWindow::newTemplate()
{
    labelTemplate_ = LabelTemplate::defaultTemplate();
    csvMappingOverrides_.clear();
    refreshSettingsControls();
    refreshElementList();
    selectElement(labelTemplate_.elements.empty() ? -1 : 0);
    refreshPreview();
}

void MainWindow::addElement(LabelElementType type)
{
    LabelElement element;
    element.type = type;
    element.name = type == LabelElementType::Text ? "Text" : (type == LabelElementType::QrCode ? "QR Code" : "Barcode");
    element.id = element.name + "_" + std::to_string(labelTemplate_.elements.size() + 1);
    element.text = type == LabelElementType::Text ? "Text" : "{ItemNumber}";
    element.source = type == LabelElementType::Text ? FieldSource::Fixed : FieldSource::Variable;
    element.variableName = type == LabelElementType::Text ? "" : "ItemNumber";
    if (type == LabelElementType::Text)
    {
        element.name = "Text / Number";
        element.text = "Sample Text";
        element.xInches = 0.16;
        element.yInches = 0.12;
        element.boxWidthInches = 1.85;
        element.fontHeightDots = 62;
        element.fontWidthDots = 46;
        element.bold = true;
    }
    else if (type == LabelElementType::Code128Barcode || type == LabelElementType::Code39Barcode)
    {
        element.xInches = 0.28;
        element.yInches = 0.36;
        element.barcodeHeightDots = 54;
        element.barcodeModuleWidth = 2;
    }
    else if (type == LabelElementType::QrCode)
    {
        element.xInches = 1.78;
        element.yInches = 0.08;
        element.text = "ITEM:{ItemNumber}";
        element.qrMagnification = 4;
    }
    labelTemplate_.elements.push_back(element);
    refreshElementList();
    selectElement(static_cast<int>(labelTemplate_.elements.size() - 1));
    refreshPreview();
}

void MainWindow::duplicateSelectedElement()
{
    if (selectedElement_ < 0 || selectedElement_ >= static_cast<int>(labelTemplate_.elements.size()))
    {
        return;
    }

    LabelElement copy = labelTemplate_.elements[selectedElement_];
    copy.name += " Copy";
    copy.id += "_copy";
    copy.xInches += 0.05;
    copy.yInches += 0.05;
    labelTemplate_.elements.insert(labelTemplate_.elements.begin() + selectedElement_ + 1, copy);
    refreshElementList();
    selectElement(selectedElement_ + 1);
    refreshPreview();
}

void MainWindow::deleteSelectedElement()
{
    if (selectedElement_ < 0 || selectedElement_ >= static_cast<int>(labelTemplate_.elements.size()))
    {
        return;
    }
    labelTemplate_.elements.erase(labelTemplate_.elements.begin() + selectedElement_);
    refreshElementList();
    selectElement(labelTemplate_.elements.empty() ? -1 : std::min(selectedElement_, static_cast<int>(labelTemplate_.elements.size() - 1)));
    refreshPreview();
}

void MainWindow::moveSelectedElement(int offset)
{
    int newIndex = selectedElement_ + offset;
    if (selectedElement_ < 0 || selectedElement_ >= static_cast<int>(labelTemplate_.elements.size()) ||
        newIndex < 0 || newIndex >= static_cast<int>(labelTemplate_.elements.size()))
    {
        return;
    }

    std::swap(labelTemplate_.elements[selectedElement_], labelTemplate_.elements[newIndex]);
    refreshElementList();
    selectElement(newIndex);
    refreshPreview();
}

void MainWindow::saveTemplate()
{
    QString path = QFileDialog::getSaveFileName(this, "Save Template", "templates/default_label.json", "JSON (*.json)");
    if (path.isEmpty()) return;
    std::string error;
    if (!TemplateStorage::save(labelTemplate_, path.toStdString(), error))
    {
        QMessageBox::warning(this, "Save Failed", QString::fromStdString(error));
    }
    refreshTemplateLibrary();
}

void MainWindow::loadTemplate()
{
    QString path = QFileDialog::getOpenFileName(this, "Load Template", "templates", "JSON (*.json)");
    if (path.isEmpty()) return;
    loadTemplateFromPath(path);
}

void MainWindow::loadTemplateFromPath(const QString& path)
{
    labelTemplate_ = TemplateStorage::load(path.toStdString());
    refreshSettingsControls();
    refreshElementList();
    selectElement(labelTemplate_.elements.empty() ? -1 : 0);
    refreshPreview();
}

void MainWindow::loadSelectedLibraryTemplate()
{
    if (!templateList_ || !templateList_->currentItem())
    {
        QMessageBox::information(this, "Template Library", "Select a template first.");
        return;
    }

    loadTemplateFromPath(templateList_->currentItem()->data(Qt::UserRole).toString());
}

void MainWindow::importCsv()
{
    if (excelRecords_)
    {
        excelRecords_->loadFile();
    }
}

void MainWindow::configureCsvMapping()
{
    if (!excelRecords_ || excelRecords_->headers().empty())
    {
        QMessageBox::information(this, "Data Required", "Import an Excel or CSV file before mapping columns.");
        return;
    }

    std::set<std::string> placeholders;
    for (const LabelElement& element : labelTemplate_.elements)
    {
        if (!element.variableName.empty())
        {
            placeholders.insert(element.variableName);
        }
        for (const auto& placeholder : VariableResolver::findPlaceholders(element.text))
        {
            placeholders.insert(placeholder.first);
        }
    }

    QStringList headerChoices;
    for (const QString& header : excelRecords_->headers())
    {
        headerChoices << header;
    }

    for (const std::string& placeholder : placeholders)
    {
        bool ok = false;
        int currentIndex = 0;
        auto existing = csvMappingOverrides_.find(placeholder);
        if (existing != csvMappingOverrides_.end())
        {
            currentIndex = std::max(0, static_cast<int>(headerChoices.indexOf(QString::fromStdString(existing->second))));
        }
        else
        {
            currentIndex = std::max(0, static_cast<int>(headerChoices.indexOf(QString::fromStdString(placeholder))));
        }

        QString selected = QInputDialog::getItem(
            this,
            "Map CSV Column",
            QString::fromStdString(placeholder),
            headerChoices,
            currentIndex,
            false,
            &ok);
        if (ok && !selected.isEmpty())
        {
            csvMappingOverrides_[placeholder] = selected.toStdString();
        }
    }
    previewSelectedCsvRecord();
}

void MainWindow::previewSelectedCsvRecord()
{
    if (!excelRecords_ || excelRecords_->records().records.empty())
    {
        preview_->setVariables({});
        return;
    }
    preview_->setVariables(contextForRow(0));
}

void MainWindow::previewZpl()
{
    VariableContext context;
    if (excelRecords_ && !excelRecords_->records().records.empty())
    {
        const QVector<int> rows = excelRecords_->printableSourceRows();
        context = contextForRow(rows.isEmpty() ? 0 : rows.first());
    }

    auto* dialog = new QDialog(this);
    dialog->setWindowTitle("Generated ZPL");
    dialog->resize(720, 520);
    auto* layout = new QVBoxLayout(dialog);
    auto* text = new QTextEdit(dialog);
    text->setPlainText(QString::fromStdString(ZplGenerator::generate(labelTemplate_, context)));
    text->setReadOnly(true);
    layout->addWidget(text);
    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Close, dialog);
    connect(buttons, &QDialogButtonBox::rejected, dialog, &QDialog::reject);
    layout->addWidget(buttons);
    dialog->open();
}

void MainWindow::printTestLabel()
{
    VariableContext context;
    context.values["ItemNumber"] = "TEST-123";
    context.values["Description"] = "Test Label";
    context.values["Lot"] = "LOT-TEST";
    context.serialNumber = serialStartSpin_->value();
    printContexts({context}, 1);
}

void MainWindow::printCurrent()
{
    if (excelRecords_ && !excelRecords_->records().records.empty())
    {
        printSelectedCsvRows();
        return;
    }

    const int start = serialStartSpin_->value();
    const int end = serialEndSpin_->value();
    if (end != start)
    {
        std::vector<VariableContext> contexts;
        VariableContext baseContext = promptContext();
        const int step = start <= end ? 1 : -1;
        for (int serial = start; start <= end ? serial <= end : serial >= end; serial += step)
        {
            VariableContext context = baseContext;
            context.serialNumber = serial;
            contexts.push_back(context);
        }
        printContexts(contexts, copiesSpin_->value());
        return;
    }

    printContexts({promptContext()}, copiesSpin_->value());
}

void MainWindow::printSelectedCsvRows()
{
    if (!excelRecords_)
    {
        return;
    }
    for (int row : excelRecords_->printableSourceRows())
    {
        printContexts({contextForRow(row)}, copiesSpin_->value() * excelRecords_->copiesForSourceRow(row));
    }
}

void MainWindow::printAllCsvRows()
{
    if (!excelRecords_)
    {
        return;
    }
    const ExcelRecordSet records = excelRecords_->records();
    for (int row = 0; row < records.records.size(); ++row)
    {
        printContexts({contextForRow(row)}, copiesSpin_->value() * records.records[row].copies);
    }
}

void MainWindow::updateTemplateFromSettings()
{
    PrinterSettings& s = labelTemplate_.settings;
    s.printerName = printerCombo_->currentText().toStdString();
    s.dpi = dpiCombo_->currentData().toInt();
    s.labelWidthInches = std::max(0.1, widthSpin_->value());
    s.labelHeightInches = std::max(0.1, heightSpin_->value());
    s.marginLeftInches = marginLeftSpin_->value();
    s.marginTopInches = marginTopSpin_->value();
    s.gapInches = gapSpin_->value();
    s.mediaSensing = static_cast<MediaSensingMode>(mediaCombo_->currentIndex());
    s.orientation = static_cast<LabelOrientation>(orientationCombo_->currentIndex());
    s.speedIps = speedSpin_->value();
    s.darkness = darknessSpin_->value();
    s.copies = copiesSpin_->value();
    updateCalculatedSizeLabels();
}

void MainWindow::selectElement(int index)
{
    selectedElement_ = index;
    elementList_->setCurrentRow(index);
    preview_->setSelectedElement(index);
    editor_->setElement(index >= 0 && index < static_cast<int>(labelTemplate_.elements.size()) ? &labelTemplate_.elements[index] : nullptr);
}

VariableContext MainWindow::contextForRow(int rowIndex) const
{
    VariableContext context;
    context.recordIndex = rowIndex + 1;
    context.serialNumber = serialStartSpin_->value() + rowIndex;
    if (excelRecords_)
    {
        const ExcelRecordSet records = excelRecords_->records();
        if (rowIndex >= 0 && rowIndex < records.records.size())
        {
            const ExcelRecord& record = records.records[rowIndex];
            for (int column = 0; column < records.headers.size(); ++column)
            {
                context.values[records.headers[column].toStdString()] =
                    column < record.values.size() ? record.values[column].toStdString() : std::string();
            }
            for (const auto& mapEntry : csvMappingOverrides_)
            {
                const int column = records.headers.indexOf(QString::fromStdString(mapEntry.second));
                if (column >= 0 && column < record.values.size())
                {
                    context.values[mapEntry.first] = record.values[column].toStdString();
                }
            }
        }
        return context;
    }
    std::map<std::string, std::string> mapping = csvMapping();
    context.values = csvData_.rowAsVariables(static_cast<std::size_t>(rowIndex), mapping);
    return context;
}

VariableContext MainWindow::promptContext() const
{
    VariableContext context;
    context.serialNumber = serialStartSpin_->value();
    for (const LabelElement& element : labelTemplate_.elements)
    {
        if ((element.source == FieldSource::PromptAtPrint || element.source == FieldSource::Variable) && !element.variableName.empty())
        {
            bool ok = false;
            QString value = QInputDialog::getText(nullptr, "Print Value", QString::fromStdString(element.variableName), QLineEdit::Normal, "", &ok);
            if (ok)
            {
                context.values[element.variableName] = value.toStdString();
            }
        }

        for (const auto& placeholder : VariableResolver::findPlaceholders(element.text))
        {
            if (context.values.find(placeholder.first) != context.values.end())
            {
                continue;
            }
            bool ok = false;
            QString value = QInputDialog::getText(nullptr, "Print Value", QString::fromStdString(placeholder.first), QLineEdit::Normal, "", &ok);
            if (ok)
            {
                context.values[placeholder.first] = value.toStdString();
            }
        }
    }
    return context;
}

std::map<std::string, std::string> MainWindow::csvMapping() const
{
    std::map<std::string, std::string> mapping = csvMappingOverrides_;
    QStringList excelHeaders;
    if (excelRecords_)
    {
        excelHeaders = excelRecords_->headers();
    }
    for (const LabelElement& element : labelTemplate_.elements)
    {
        if (!element.variableName.empty())
        {
            for (const QString& header : excelHeaders)
            {
                if (header.toStdString() == element.variableName && mapping.find(element.variableName) == mapping.end())
                {
                    mapping[element.variableName] = header.toStdString();
                }
            }
            for (const std::string& header : csvData_.headers)
            {
                if (header == element.variableName && mapping.find(element.variableName) == mapping.end())
                {
                    mapping[element.variableName] = header;
                }
            }
        }

        for (const auto& placeholder : VariableResolver::findPlaceholders(element.text))
        {
            for (const QString& header : excelHeaders)
            {
                if (header.toStdString() == placeholder.first && mapping.find(placeholder.first) == mapping.end())
                {
                    mapping[placeholder.first] = header.toStdString();
                }
            }
            for (const std::string& header : csvData_.headers)
            {
                if (header == placeholder.first && mapping.find(placeholder.first) == mapping.end())
                {
                    mapping[placeholder.first] = header;
                }
            }
        }
    }
    return mapping;
}

int MainWindow::quantityForRow(int rowIndex) const
{
    if (excelRecords_)
    {
        return excelRecords_->copiesForSourceRow(rowIndex);
    }
    if (rowIndex < 0 || rowIndex >= static_cast<int>(csvData_.rows.size()))
    {
        return 1;
    }

    int quantityColumn = -1;
    for (int i = 0; i < static_cast<int>(csvData_.headers.size()); ++i)
    {
        const std::string& header = csvData_.headers[i];
        if (header == "Quantity" || header == "Qty" || header == "quantity" || header == "qty")
        {
            quantityColumn = i;
            break;
        }
    }

    if (quantityColumn < 0 || quantityColumn >= static_cast<int>(csvData_.rows[rowIndex].size()))
    {
        return 1;
    }

    bool ok = false;
    int quantity = QString::fromStdString(csvData_.rows[rowIndex][quantityColumn]).toInt(&ok);
    return ok ? std::max(1, quantity) : 1;
}

void MainWindow::printContexts(const std::vector<VariableContext>& contexts, int quantityPerContext)
{
    if (printerCombo_->currentText().isEmpty())
    {
        QMessageBox::warning(this, "Printer Required", "Select a Windows printer first.");
        return;
    }

    std::string printer = printerCombo_->currentText().toStdString();
    std::string error;
    for (const VariableContext& context : contexts)
    {
        std::string zpl = ZplGenerator::generate(labelTemplate_, context);
        for (int i = 0; i < quantityPerContext; ++i)
        {
            if (!ZebraPrinter::printRawZpl(printer, zpl, error))
            {
                QMessageBox::critical(this, "Print Failed", QString::fromStdString(error));
                return;
            }
        }
    }
    statusBar()->showMessage("Print job sent.");
}
