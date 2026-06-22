#include "ui/MainWindow.h"

#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QInputDialog>
#include <QListWidget>
#include <QMessageBox>
#include <QModelIndex>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QSplitter>
#include <QStatusBar>
#include <QStringList>
#include <QTableWidget>
#include <QTableWidgetItem>
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
    auto* toolbar = addToolBar("Label Tools");
    toolbar->addAction("Add Text", this, [this] { addElement(LabelElementType::Text); });
    toolbar->addAction("Add Code128", this, [this] { addElement(LabelElementType::Code128Barcode); });
    toolbar->addAction("Add Code39", this, [this] { addElement(LabelElementType::Code39Barcode); });
    toolbar->addAction("Add QR", this, [this] { addElement(LabelElementType::QrCode); });
    toolbar->addAction("Delete", this, &MainWindow::deleteSelectedElement);
    toolbar->addSeparator();
    toolbar->addAction("Load", this, &MainWindow::loadTemplate);
    toolbar->addAction("Save", this, &MainWindow::saveTemplate);
    toolbar->addAction("Import CSV", this, &MainWindow::importCsv);
    toolbar->addAction("Map CSV", this, &MainWindow::configureCsvMapping);
    toolbar->addSeparator();
    toolbar->addAction("Print Current", this, &MainWindow::printCurrent);
    toolbar->addAction("Print Selected CSV", this, &MainWindow::printSelectedCsvRows);
    toolbar->addAction("Print All CSV", this, &MainWindow::printAllCsvRows);

    auto* splitter = new QSplitter(this);
    auto* left = new QWidget(splitter);
    auto* leftLayout = new QVBoxLayout(left);

    auto* settingsLayout = new QFormLayout;
    printerCombo_ = new QComboBox(left);
    dpiSpin_ = new QSpinBox(left);
    dpiSpin_->setRange(100, 600);
    widthSpin_ = new QDoubleSpinBox(left);
    heightSpin_ = new QDoubleSpinBox(left);
    marginLeftSpin_ = new QDoubleSpinBox(left);
    marginTopSpin_ = new QDoubleSpinBox(left);
    gapSpin_ = new QDoubleSpinBox(left);
    for (QDoubleSpinBox* spin : {widthSpin_, heightSpin_, marginLeftSpin_, marginTopSpin_, gapSpin_})
    {
        spin->setDecimals(3);
        spin->setRange(0.0, 10.0);
        spin->setSuffix(" in");
        spin->setSingleStep(0.01);
    }
    mediaCombo_ = new QComboBox(left);
    mediaCombo_->addItems({"Gap", "Black mark", "Continuous"});
    orientationCombo_ = new QComboBox(left);
    orientationCombo_->addItems({"Portrait", "Landscape"});
    speedSpin_ = new QSpinBox(left);
    speedSpin_->setRange(1, 14);
    darknessSpin_ = new QSpinBox(left);
    darknessSpin_->setRange(0, 30);
    copiesSpin_ = new QSpinBox(left);
    copiesSpin_->setRange(1, 9999);
    serialStartSpin_ = new QSpinBox(left);
    serialStartSpin_->setRange(0, 999999999);
    serialStartSpin_->setValue(1);
    serialEndSpin_ = new QSpinBox(left);
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
    leftLayout->addLayout(settingsLayout);

    elementList_ = new QListWidget(left);
    editor_ = new ElementEditorWidget(left);
    leftLayout->addWidget(elementList_);
    leftLayout->addWidget(editor_);

    auto* right = new QWidget(splitter);
    auto* rightLayout = new QVBoxLayout(right);
    preview_ = new PreviewWidget(right);
    csvTable_ = new QTableWidget(right);
    rightLayout->addWidget(preview_, 3);
    rightLayout->addWidget(csvTable_, 2);
    splitter->addWidget(left);
    splitter->addWidget(right);
    splitter->setStretchFactor(1, 1);
    setCentralWidget(splitter);

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

void MainWindow::refreshSettingsControls()
{
    const PrinterSettings& s = labelTemplate_.settings;
    dpiSpin_->setValue(s.dpi);
    widthSpin_->setValue(s.labelWidthInches);
    heightSpin_->setValue(s.labelHeightInches);
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

void MainWindow::addElement(LabelElementType type)
{
    LabelElement element;
    element.type = type;
    element.name = type == LabelElementType::Text ? "Text" : (type == LabelElementType::QrCode ? "QR Code" : "Barcode");
    element.id = element.name;
    element.text = type == LabelElementType::Text ? "Text" : "{ItemNumber}";
    element.source = type == LabelElementType::Text ? FieldSource::Fixed : FieldSource::Variable;
    element.variableName = type == LabelElementType::Text ? "" : "ItemNumber";
    labelTemplate_.elements.push_back(element);
    refreshElementList();
    selectElement(static_cast<int>(labelTemplate_.elements.size() - 1));
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
    selectElement(labelTemplate_.elements.empty() ? -1 : 0);
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
}

void MainWindow::loadTemplate()
{
    QString path = QFileDialog::getOpenFileName(this, "Load Template", "templates", "JSON (*.json)");
    if (path.isEmpty()) return;
    labelTemplate_ = TemplateStorage::load(path.toStdString());
    refreshSettingsControls();
    refreshElementList();
    selectElement(labelTemplate_.elements.empty() ? -1 : 0);
    refreshPreview();
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
    s.labelWidthInches = widthSpin_->value();
    s.labelHeightInches = heightSpin_->value();
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
