#pragma once

#include <QMainWindow>
#include <QTableWidget>

#include <optional>
#include <vector>

#include "core/CsvImporter.h"
#include "core/LabelTemplate.h"
#include "core/VariableResolver.h"

class QComboBox;
class QCloseEvent;
class QDoubleSpinBox;
class QAction;
class QLabel;
class QLineEdit;
class QListWidget;
class QPushButton;
class QSpinBox;
class QTabWidget;

class ElementEditorWidget;
class ExcelRecordsWidget;
class PreviewWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    void buildUi();
    void buildMenus();
    void buildToolbar();
    QWidget* buildDesignTab();
    QWidget* buildElementsTab();
    QWidget* buildDataTab();
    QWidget* buildPrintTab();
    QWidget* buildTemplatesTab();
    QWidget* buildSettingsTab();
    QWidget* buildPrinterSettingsPanel(QWidget* parent);
    void applyStockPreset(int index);
    void updateCalculatedSizeLabels();
    void updateStatusSummary();
    void setGridVisible(bool visible);
    void setSnapToGrid(bool enabled);
    void loadAppSettings();
    void saveAppSettings();
    void resetAppSettings();
    void refreshPrinterList();
    void refreshElementList();
    void refreshTemplateLibrary();
    void refreshSettingsControls();
    void refreshPreview();
    void loadDefaultTemplate();
    void newTemplate();
    void addElement(LabelElementType type);
    void addNumberElement();
    void addDescriptionElement();
    void addLineElement();
    void addBoxElement();
    void cutSelectedElement();
    void copySelectedElement();
    void pasteElement();
    void undo();
    void redo();
    void saveUndoState();
    void restoreFromHistory(const LabelTemplate& labelTemplate);
    void showHelp();
    void duplicateSelectedElement();
    void deleteSelectedElement();
    void moveSelectedElement(int offset);
    void moveSelectedElementToIndex(int targetIndex);
    void alignSelectedLeft();
    void alignSelectedCenter();
    void alignSelectedRight();
    void alignSelectedTop();
    void alignSelectedMiddle();
    void alignSelectedBottom();
    void distributeElementsHorizontally();
    void lockSelectedElement(bool locked);
    void saveTemplate();
    void loadTemplate();
    void loadTemplateFromPath(const QString& path);
    void loadSelectedLibraryTemplate();
    void loadSelectedCanvasTemplate();
    void importCsv();
    void configureCsvMapping();
    void previewSelectedCsvRecord();
    void previewZpl();
    void printTestLabel();
    void printCurrent();
    void printSelectedCsvRows();
    void printAllCsvRows();
    void updateTemplateFromSettings();
    void selectElement(int index);
    VariableContext contextForRow(int rowIndex) const;
    VariableContext promptContext() const;
    std::map<std::string, std::string> csvMapping() const;
    int quantityForRow(int rowIndex) const;
    bool printContexts(const std::vector<VariableContext>& contexts, int quantityPerContext, const QString& mode);
    void logPrintHistory(const QString& mode, int rows, int copies, bool success, const QString& message) const;

    LabelTemplate labelTemplate_;
    CsvData csvData_;
    std::map<std::string, std::string> csvMappingOverrides_;
    std::optional<LabelElement> clipboardElement_;
    std::vector<LabelTemplate> undoStack_;
    std::vector<LabelTemplate> redoStack_;
    int selectedElement_ = -1;

    QTabWidget* tabs_ = nullptr;
    PreviewWidget* preview_ = nullptr;
    ElementEditorWidget* editor_ = nullptr;
    QListWidget* elementList_ = nullptr;
    QListWidget* templateList_ = nullptr;
    QComboBox* canvasTemplateCombo_ = nullptr;
    QComboBox* printerCombo_ = nullptr;
    QComboBox* stockPresetCombo_ = nullptr;
    QComboBox* dpiCombo_ = nullptr;
    QDoubleSpinBox* widthSpin_ = nullptr;
    QDoubleSpinBox* heightSpin_ = nullptr;
    QDoubleSpinBox* marginLeftSpin_ = nullptr;
    QDoubleSpinBox* marginTopSpin_ = nullptr;
    QDoubleSpinBox* gapSpin_ = nullptr;
    QComboBox* mediaCombo_ = nullptr;
    QComboBox* printMethodCombo_ = nullptr;
    QComboBox* coreSizeCombo_ = nullptr;
    QLabel* widthDotsLabel_ = nullptr;
    QLabel* heightDotsLabel_ = nullptr;
    QComboBox* orientationCombo_ = nullptr;
    QSpinBox* speedSpin_ = nullptr;
    QSpinBox* darknessSpin_ = nullptr;
    QSpinBox* copiesSpin_ = nullptr;
    QSpinBox* serialStartSpin_ = nullptr;
    QSpinBox* serialEndSpin_ = nullptr;
    ExcelRecordsWidget* excelRecords_ = nullptr;
    QAction* gridAction_ = nullptr;
    QAction* snapAction_ = nullptr;
    QLabel* statusPositionLabel_ = nullptr;
    QLabel* statusStockLabel_ = nullptr;
    QLabel* statusDpiLabel_ = nullptr;
    QLabel* statusZoomLabel_ = nullptr;
};
