#pragma once

#include <string>
#include <vector>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include "LabelPrinterApp/LabelTemplate.h"
#include "LabelPrinterApp/LabelValidation.h"
#include "LabelPrinterApp/PrinterSettings.h"
#include "LabelPrinterApp/PrintHistory.h"
#include "PreviewWidget.h"

class MainWindow
{
public:
    MainWindow();

    int Show(HINSTANCE instance, int commandShow);

private:
    struct ElementInput
    {
        std::size_t elementIndex = 0;
        HWND labelHandle = nullptr;
        HWND valueEdit = nullptr;
        HWND xEdit = nullptr;
        HWND yEdit = nullptr;
        HWND sizeEdit = nullptr;
        HWND optionCheck = nullptr;
        HWND symbologyCombo = nullptr;
    };

    static constexpr int PrinterComboId = 1001;
    static constexpr int PrintButtonId = 1002;
    static constexpr int SaveButtonId = 1003;
    static constexpr int NewTemplateButtonId = 1004;
    static constexpr int ReloadTemplateButtonId = 1005;
    static constexpr int RefreshPrintersButtonId = 1006;
    static constexpr int DarknessEditId = 1101;
    static constexpr int SpeedEditId = 1102;
    static constexpr int CopiesEditId = 1103;
    static constexpr int LabelWidthEditId = 1104;
    static constexpr int LabelHeightEditId = 1105;
    static constexpr int FirstInputId = 2000;
    static constexpr int InputsPerElement = 10;

    static LRESULT CALLBACK WindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam);

    bool RegisterWindowClass(HINSTANCE instance);
    bool CreateMainWindow(HINSTANCE instance, int commandShow);

    void CreateControls();
    void LayoutControls(int width, int height);
    void PopulatePrinters();
    void CreateElementInputs();
    void DestroyElementInputs();
    void SyncInputsToTemplate();
    void SyncSettingsFromControls();
    void PrintTemplate();
    void SaveTemplate();
    void NewTemplate();
    void ReloadTemplate();
    void RefreshTemplateControls();
    void AppendHistory(const std::wstring& printerName, int copyCount, bool success, const std::string& message);
    void RefreshHistoryList();
    void UpdateStatus(const std::wstring& message);
    void ShowMessage(const std::wstring& message, const std::wstring& title, UINT icon);
    bool ValidateCurrentTemplate(const std::wstring& title);

    static std::wstring ToWide(const std::string& value);
    static std::wstring ToValidationMessage(const LabelValidationResult& result);
    static std::string ToNarrow(const std::wstring& value);
    static std::wstring GetWindowTextValue(HWND window);
    static int GetWindowIntValue(HWND window, int fallback);
    static void SetWindowIntValue(HWND window, int value);
    static std::string ResolveTemplatePath();

    HWND windowHandle = nullptr;
    HWND printerLabel = nullptr;
    HWND printerCombo = nullptr;
    HWND printButton = nullptr;
    HWND saveButton = nullptr;
    HWND newTemplateButton = nullptr;
    HWND reloadTemplateButton = nullptr;
    HWND refreshPrintersButton = nullptr;
    HWND darknessLabel = nullptr;
    HWND darknessEdit = nullptr;
    HWND speedLabel = nullptr;
    HWND speedEdit = nullptr;
    HWND copiesLabel = nullptr;
    HWND copiesEdit = nullptr;
    HWND labelWidthLabel = nullptr;
    HWND labelWidthEdit = nullptr;
    HWND labelHeightLabel = nullptr;
    HWND labelHeightEdit = nullptr;
    HWND historyLabel = nullptr;
    HWND historyList = nullptr;
    HWND statusLabel = nullptr;

    PreviewWidget previewWidget;
    HWND previewHandle = nullptr;

    LabelTemplate labelTemplate;
    PrinterSettings printerSettings;
    PrintHistory printHistory;
    std::vector<ElementInput> elementInputs;
    std::string templatePath;
    bool printersAvailable = false;
};
