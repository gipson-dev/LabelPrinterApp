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
    tabs_->addTab(buildPrintTab(), "Print");
    tabs_->addTab(buildTemplatesTab(), "Templates");
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
    for (QSpinBox* spin : {dpiSpin_, speedSpin_, darknessSpin_, copiesSpin_})
    {
        connect(spin, &QSpinBox::valueChanged, this, settingsChanged);
    }
    for (QDoubleSpinBox* spin : {widthSpin_, heightSpin_, marginLeftSpin_, marginTopSpin_, gapSpin_})
    {
        connect(spin, &QDoubleSpinBox::valueChanged, this, settingsChanged);
    }
    connect(mediaCombo_, &QComboBox::currentIndexChanged, this, settingsChanged);
    connect(orientationCombo_, &QComboBox::currentIndexChanged, this, settingsChanged);
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

    auto* sourceGroup = new QGroupBox("Data Source", tab);
    auto* sourceLayout = new QVBoxLayout(sourceGroup);
    sourceLayout->addWidget(new QRadioButton("Manual Entry", sourceGroup));
    sourceLayout->addWidget(new QRadioButton("Prompt Before Print", sourceGroup));
    sourceLayout->addWidget(new QRadioButton("Auto-Increment Serial Numbers", sourceGroup));
    auto* csvRadio = new QRadioButton("CSV File", sourceGroup);
    csvRadio->setChecked(true);
    sourceLayout->addWidget(csvRadio);
    layout->addWidget(sourceGroup);

    csvTable_ = new QTableWidget(tab);
    layout->addWidget(csvTable_, 1);

    auto* buttons = new QHBoxLayout;
    auto* importButton = new QPushButton("Import CSV", tab);
    auto* mapButton = new QPushButton("Map Columns", tab);
    auto* previewRecordButton = new QPushButton("Preview Record", tab);
    buttons->addWidget(importButton);
    buttons->addWidget(mapButton);
    buttons->addWidget(previewRecordButton);
    buttons->addStretch();
    layout->addLayout(buttons);

    connect(importButton, &QPushButton::clicked, this, &MainWindow::importCsv);
    connect(mapButton, &QPushButton::clicked, this, &MainWindow::configureCsvMapping);
    connect(previewRecordButton, &QPushButton::clicked, this, &MainWindow::previewSelectedCsvRecord);
    connect(csvTable_, &QTableWidget::itemSelectionChanged, this, &MainWindow::previewSelectedCsvRecord);
    return tab;
}

QWidget* MainWindow::buildPrintTab()
{
    auto* tab = new QWidget(this);
    auto* layout = new QVBoxLayout(tab);
    layout->addWidget(buildPrinterSettingsPanel(tab));

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
    auto* form = new QFormLayout;
    form->addRow("Default Printer", new QLabel("Uses the selected Windows printer from the Print tab.", tab));
    form->addRow("Default DPI", new QLabel("203", tab));
    form->addRow("Default Width", new QLabel("2.25 inches", tab));
    form->addRow("Default Height", new QLabel("0.75 inches", tab));
    form->addRow("Template Folder", new QLabel("templates/", tab));
    form->addRow("CSV Folder", new QLabel("examples/", tab));
    layout->addLayout(form);

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
    auto* group = new QGroupBox("Printer Settings", parent);
    auto* settingsLayout = new QFormLayout(group);

    printerCombo_ = new QComboBox(group);
    dpiSpin_ = new QSpinBox(group);
    dpiSpin_->setRange(100, 600);
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
    mediaCombo_->addItems({"Gap", "Black mark", "Continuous"});
    orientationCombo_ = new QComboBox(group);
    orientationCombo_->addItems({"Portrait", "Landscape"});
    speedSpin_ = new QSpinBox(group);
    speedSpin_->setRange(1, 14);
    darknessSpin_ = new QSpinBox(group);
    darknessSpin_->setRange(0, 30);
    copiesSpin_ = new QSpinBox(group);
    copiesSpin_->setRange(1, 9999);
    serialStartSpin_ = new QSpinBox(group);
    serialStartSpin_->setRange(0, 999999999);
    serialStartSpin_->setValue(1);
    serialEndSpin_ = new QSpinBox(group);
    serialEndSpin_->setRange(0, 999999999);
    serialEndSpin_->setValue(1);

    settingsLayout->addRow("Printer", printerCombo_);
    settingsLayout->addRow("DPI", dpiSpin_);
    settingsLayout->addRow("Width", widthSpin_);
    settingsLayout->addRow("Height", heightSpin_);
    settingsLayout->addRow("Left Margin", marginLeftSpin_);
    settingsLayout->addRow("Top Margin", marginTopSpin_);
    settingsLayout->addRow("Gap", gapSpin_);
    settingsLayout->addRow("Sensing", mediaCombo_);
    settingsLayout->addRow("Orientation", orientationCombo_);
    settingsLayout->addRow("Speed", speedSpin_);
    settingsLayout->addRow("Darkness", darknessSpin_);
    settingsLayout->addRow("Copies", copiesSpin_);
    settingsLayout->addRow("Serial Start", serialStartSpin_);
    settingsLayout->addRow("Serial End", serialEndSpin_);
    return group;
}

void MainWindow::refreshPrinterList()
{
    std::string error;
    printerCombo_->clear();
    for (const std::string& printer : ZebraPrinter::installedPrinters(error))
    {
        printerCombo_->addItem(QString::fromStdString(printer));
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
    QSignalBlocker dpiBlocker(dpiSpin_);
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

    dpiSpin_->setValue(s.dpi);
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
    QString path = QFileDialog::getOpenFileName(this, "Import CSV", "examples", "CSV (*.csv)");
    if (path.isEmpty()) return;
    csvData_ = CsvImporter::loadFile(path.toStdString(), true);
    csvMappingOverrides_.clear();
    csvTable_->clear();
    csvTable_->setColumnCount(static_cast<int>(csvData_.headers.size()));
    csvTable_->setRowCount(static_cast<int>(csvData_.rows.size()));
    for (int c = 0; c < static_cast<int>(csvData_.headers.size()); ++c)
    {
        csvTable_->setHorizontalHeaderItem(c, new QTableWidgetItem(QString::fromStdString(csvData_.headers[c])));
    }
    for (int r = 0; r < static_cast<int>(csvData_.rows.size()); ++r)
    {
        for (int c = 0; c < static_cast<int>(csvData_.rows[r].size()); ++c)
        {
            csvTable_->setItem(r, c, new QTableWidgetItem(QString::fromStdString(csvData_.rows[r][c])));
        }
    }
    previewSelectedCsvRecord();
}

void MainWindow::configureCsvMapping()
{
    if (csvData_.headers.empty())
    {
        QMessageBox::information(this, "CSV Required", "Import a CSV file before mapping columns.");
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
    for (const std::string& header : csvData_.headers)
    {
        headerChoices << QString::fromStdString(header);
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
    if (csvData_.rows.empty())
    {
        preview_->setVariables({});
        return;
    }

    int row = 0;
    const auto selected = csvTable_->selectionModel() ? csvTable_->selectionModel()->selectedRows() : QModelIndexList{};
    if (!selected.empty())
    {
        row = selected.front().row();
    }
    preview_->setVariables(contextForRow(row));
}

void MainWindow::previewZpl()
{
    VariableContext context;
    if (!csvData_.rows.empty())
    {
        int row = 0;
        const auto selected = csvTable_->selectionModel() ? csvTable_->selectionModel()->selectedRows() : QModelIndexList{};
        if (!selected.empty())
        {
            row = selected.front().row();
        }
        context = contextForRow(row);
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
    for (const QModelIndex& index : csvTable_->selectionModel()->selectedRows())
    {
        printContexts({contextForRow(index.row())}, copiesSpin_->value() * quantityForRow(index.row()));
    }
}

void MainWindow::printAllCsvRows()
{
    for (int row = 0; row < csvTable_->rowCount(); ++row)
    {
        printContexts({contextForRow(row)}, copiesSpin_->value() * quantityForRow(row));
    }
}

void MainWindow::updateTemplateFromSettings()
{
    PrinterSettings& s = labelTemplate_.settings;
    s.dpi = dpiSpin_->value();
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
    for (const LabelElement& element : labelTemplate_.elements)
    {
        if (!element.variableName.empty())
        {
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
