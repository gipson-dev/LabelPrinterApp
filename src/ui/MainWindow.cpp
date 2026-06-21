#include "MainWindow.h"

#include <algorithm>
#include <ctime>
#include <filesystem>
#include <sstream>

#include "LabelPrinterApp/LabelValidation.h"
#include "LabelPrinterApp/TemplateStorage.h"
#include "LabelPrinterApp/ZebraPrinter.h"
#include "LabelPrinterApp/ZplGenerator.h"

MainWindow::MainWindow()
    : templatePath(ResolveTemplatePath()),
      labelTemplate(TemplateStorage::LoadFromFile(templatePath))
{
}

int MainWindow::Show(HINSTANCE instance, int commandShow)
{
    if (!RegisterWindowClass(instance) || !CreateMainWindow(instance, commandShow))
    {
        return 1;
    }

    MSG message = {};
    while (GetMessageW(&message, nullptr, 0, 0) > 0)
    {
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }

    return static_cast<int>(message.wParam);
}

bool MainWindow::RegisterWindowClass(HINSTANCE instance)
{
    previewWidget.RegisterWindowClass(instance);

    WNDCLASSW windowClass = {};
    windowClass.lpfnWndProc = MainWindow::WindowProc;
    windowClass.hInstance = instance;
    windowClass.lpszClassName = L"LabelPrinterMainWindow";
    windowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
    windowClass.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);

    return RegisterClassW(&windowClass) != 0 || GetLastError() == ERROR_CLASS_ALREADY_EXISTS;
}

bool MainWindow::CreateMainWindow(HINSTANCE instance, int commandShow)
{
    windowHandle = CreateWindowExW(
        0,
        L"LabelPrinterMainWindow",
        L"Label Printer",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        940,
        640,
        nullptr,
        nullptr,
        instance,
        this);

    if (!windowHandle)
    {
        return false;
    }

    ShowWindow(windowHandle, commandShow);
    UpdateWindow(windowHandle);
    return true;
}

LRESULT CALLBACK MainWindow::WindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
    MainWindow* mainWindow = nullptr;

    if (message == WM_NCCREATE)
    {
        auto* createStruct = reinterpret_cast<CREATESTRUCTW*>(lParam);
        mainWindow = static_cast<MainWindow*>(createStruct->lpCreateParams);
        SetWindowLongPtrW(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(mainWindow));
        mainWindow->windowHandle = window;
    }
    else
    {
        mainWindow = reinterpret_cast<MainWindow*>(GetWindowLongPtrW(window, GWLP_USERDATA));
    }

    if (!mainWindow)
    {
        return DefWindowProcW(window, message, wParam, lParam);
    }

    switch (message)
    {
    case WM_CREATE:
        mainWindow->CreateControls();
        return 0;

    case WM_SIZE:
        mainWindow->LayoutControls(LOWORD(lParam), HIWORD(lParam));
        return 0;

    case WM_COMMAND:
        if (LOWORD(wParam) == PrintButtonId)
        {
            mainWindow->PrintTemplate();
            return 0;
        }
        if (LOWORD(wParam) == SaveButtonId)
        {
            mainWindow->SaveTemplate();
            return 0;
        }
        if (LOWORD(wParam) == NewTemplateButtonId)
        {
            mainWindow->NewTemplate();
            return 0;
        }
        if (LOWORD(wParam) == ReloadTemplateButtonId)
        {
            mainWindow->ReloadTemplate();
            return 0;
        }
        if (LOWORD(wParam) == RefreshPrintersButtonId)
        {
            mainWindow->PopulatePrinters();
            mainWindow->UpdateStatus(L"Printer list refreshed.");
            return 0;
        }
        if (LOWORD(wParam) >= DarknessEditId && LOWORD(wParam) <= LabelHeightEditId && HIWORD(wParam) == EN_CHANGE)
        {
            mainWindow->SyncInputsToTemplate();
            return 0;
        }
        if (LOWORD(wParam) >= FirstInputId && HIWORD(wParam) == EN_CHANGE)
        {
            mainWindow->SyncInputsToTemplate();
            return 0;
        }
        if (LOWORD(wParam) >= FirstInputId && (HIWORD(wParam) == BN_CLICKED || HIWORD(wParam) == CBN_SELCHANGE))
        {
            mainWindow->SyncInputsToTemplate();
            return 0;
        }
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    default:
        return DefWindowProcW(window, message, wParam, lParam);
    }
}

void MainWindow::CreateControls()
{
    printerLabel = CreateWindowExW(0, L"STATIC", L"Printer", WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, windowHandle, nullptr, nullptr, nullptr);
    printerCombo = CreateWindowExW(0, L"COMBOBOX", nullptr, WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL, 0, 0, 0, 0, windowHandle, reinterpret_cast<HMENU>(static_cast<INT_PTR>(PrinterComboId)), nullptr, nullptr);
    refreshPrintersButton = CreateWindowExW(0, L"BUTTON", L"Refresh", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, 0, 0, 0, windowHandle, reinterpret_cast<HMENU>(static_cast<INT_PTR>(RefreshPrintersButtonId)), nullptr, nullptr);
    newTemplateButton = CreateWindowExW(0, L"BUTTON", L"New", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, 0, 0, 0, windowHandle, reinterpret_cast<HMENU>(static_cast<INT_PTR>(NewTemplateButtonId)), nullptr, nullptr);
    reloadTemplateButton = CreateWindowExW(0, L"BUTTON", L"Reload", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, 0, 0, 0, windowHandle, reinterpret_cast<HMENU>(static_cast<INT_PTR>(ReloadTemplateButtonId)), nullptr, nullptr);
    printButton = CreateWindowExW(0, L"BUTTON", L"Print", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, 0, 0, 0, windowHandle, reinterpret_cast<HMENU>(static_cast<INT_PTR>(PrintButtonId)), nullptr, nullptr);
    saveButton = CreateWindowExW(0, L"BUTTON", L"Save Template", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, 0, 0, 0, windowHandle, reinterpret_cast<HMENU>(static_cast<INT_PTR>(SaveButtonId)), nullptr, nullptr);

    darknessLabel = CreateWindowExW(0, L"STATIC", L"Dark", WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, windowHandle, nullptr, nullptr, nullptr);
    darknessEdit = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", nullptr, WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_NUMBER, 0, 0, 0, 0, windowHandle, reinterpret_cast<HMENU>(static_cast<INT_PTR>(DarknessEditId)), nullptr, nullptr);
    speedLabel = CreateWindowExW(0, L"STATIC", L"Speed", WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, windowHandle, nullptr, nullptr, nullptr);
    speedEdit = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", nullptr, WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_NUMBER, 0, 0, 0, 0, windowHandle, reinterpret_cast<HMENU>(static_cast<INT_PTR>(SpeedEditId)), nullptr, nullptr);
    copiesLabel = CreateWindowExW(0, L"STATIC", L"Copies", WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, windowHandle, nullptr, nullptr, nullptr);
    copiesEdit = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"1", WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_NUMBER, 0, 0, 0, 0, windowHandle, reinterpret_cast<HMENU>(static_cast<INT_PTR>(CopiesEditId)), nullptr, nullptr);
    labelWidthLabel = CreateWindowExW(0, L"STATIC", L"Width", WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, windowHandle, nullptr, nullptr, nullptr);
    labelWidthEdit = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", nullptr, WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_NUMBER, 0, 0, 0, 0, windowHandle, reinterpret_cast<HMENU>(static_cast<INT_PTR>(LabelWidthEditId)), nullptr, nullptr);
    labelHeightLabel = CreateWindowExW(0, L"STATIC", L"Height", WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, windowHandle, nullptr, nullptr, nullptr);
    labelHeightEdit = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", nullptr, WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_NUMBER, 0, 0, 0, 0, windowHandle, reinterpret_cast<HMENU>(static_cast<INT_PTR>(LabelHeightEditId)), nullptr, nullptr);
    historyLabel = CreateWindowExW(0, L"STATIC", L"Print History", WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, windowHandle, nullptr, nullptr, nullptr);
    historyList = CreateWindowExW(WS_EX_CLIENTEDGE, L"LISTBOX", nullptr, WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOINTEGRALHEIGHT, 0, 0, 0, 0, windowHandle, nullptr, nullptr, nullptr);
    statusLabel = CreateWindowExW(0, L"STATIC", L"Ready", WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, windowHandle, nullptr, nullptr, nullptr);

    SetWindowIntValue(darknessEdit, printerSettings.darkness);
    SetWindowIntValue(speedEdit, printerSettings.printSpeed);
    SetWindowIntValue(labelWidthEdit, labelTemplate.labelWidthDots);
    SetWindowIntValue(labelHeightEdit, labelTemplate.labelHeightDots);

    previewHandle = previewWidget.Create(windowHandle, labelTemplate);

    PopulatePrinters();
    CreateElementInputs();

    RECT rect = {};
    GetClientRect(windowHandle, &rect);
    LayoutControls(rect.right - rect.left, rect.bottom - rect.top);
}

void MainWindow::LayoutControls(int width, int height)
{
    const int margin = 18;
    const int labelWidth = 100;
    const int leftWidth = 740;
    const int rowHeight = 28;
    const int rowGap = 10;
    const int previewX = leftWidth + margin;
    const int previewWidth = std::max(260, width - previewX - margin);
    const int historyHeight = 112;
    const int statusHeight = 24;
    const int previewHeight = std::max(220, height - (margin * 2) - statusHeight);

    MoveWindow(printerLabel, margin, margin + 4, labelWidth, rowHeight, TRUE);
    MoveWindow(printerCombo, margin + labelWidth, margin, 300, 200, TRUE);
    MoveWindow(refreshPrintersButton, margin + labelWidth + 310, margin, 86, rowHeight, TRUE);
    MoveWindow(newTemplateButton, margin + labelWidth + 406, margin, 72, rowHeight, TRUE);
    MoveWindow(reloadTemplateButton, margin + labelWidth + 486, margin, 82, rowHeight, TRUE);

    int settingsY = margin + rowHeight + 12;
    MoveWindow(darknessLabel, margin, settingsY + 4, 44, rowHeight, TRUE);
    MoveWindow(darknessEdit, margin + 48, settingsY, 46, rowHeight, TRUE);
    MoveWindow(speedLabel, margin + 104, settingsY + 4, 48, rowHeight, TRUE);
    MoveWindow(speedEdit, margin + 156, settingsY, 46, rowHeight, TRUE);
    MoveWindow(copiesLabel, margin + 212, settingsY + 4, 54, rowHeight, TRUE);
    MoveWindow(copiesEdit, margin + 270, settingsY, 48, rowHeight, TRUE);
    MoveWindow(labelWidthLabel, margin + 330, settingsY + 4, 54, rowHeight, TRUE);
    MoveWindow(labelWidthEdit, margin + 388, settingsY, 62, rowHeight, TRUE);
    MoveWindow(labelHeightLabel, margin + 462, settingsY + 4, 58, rowHeight, TRUE);
    MoveWindow(labelHeightEdit, margin + 524, settingsY, 62, rowHeight, TRUE);

    int y = settingsY + rowHeight + 18;
    for (const ElementInput& input : elementInputs)
    {
        MoveWindow(input.labelHandle, margin, y + 4, labelWidth, rowHeight, TRUE);
        MoveWindow(input.valueEdit, margin + labelWidth, y, 190, rowHeight, TRUE);
        MoveWindow(input.xEdit, margin + labelWidth + 198, y, 48, rowHeight, TRUE);
        MoveWindow(input.yEdit, margin + labelWidth + 252, y, 48, rowHeight, TRUE);
        MoveWindow(input.sizeEdit, margin + labelWidth + 306, y, 54, rowHeight, TRUE);
        MoveWindow(input.optionCheck, margin + labelWidth + 368, y + 2, 80, rowHeight, TRUE);
        if (input.symbologyCombo)
        {
            MoveWindow(input.symbologyCombo, margin + labelWidth + 452, y, 96, 120, TRUE);
        }
        y += rowHeight + rowGap;
    }

    int bottomY = std::max(y + 12, height - margin - historyHeight - 46);
    MoveWindow(historyLabel, margin, bottomY, 140, rowHeight, TRUE);
    MoveWindow(historyList, margin, bottomY + rowHeight, leftWidth - margin * 2, historyHeight, TRUE);
    MoveWindow(printButton, margin, height - margin - 34, 120, 34, TRUE);
    MoveWindow(saveButton, margin + 132, height - margin - 34, 150, 34, TRUE);
    MoveWindow(statusLabel, previewX, height - margin - statusHeight, previewWidth, statusHeight, TRUE);
    MoveWindow(previewHandle, previewX, margin, previewWidth, previewHeight, TRUE);
}

void MainWindow::PopulatePrinters()
{
    SendMessageW(printerCombo, CB_RESETCONTENT, 0, 0);
    printersAvailable = false;

    std::string error;
    std::vector<std::wstring> printers = ZebraPrinter::enumerateInstalledPrinters(error);
    if (printers.empty())
    {
        const wchar_t* message = error.empty() ? L"No printers found" : L"Unable to load printers";
        SendMessageW(printerCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(message));
        SendMessageW(printerCombo, CB_SETCURSEL, 0, 0);
        return;
    }

    for (const std::wstring& printerName : printers)
    {
        SendMessageW(printerCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(printerName.c_str()));
    }

    printersAvailable = true;
    SendMessageW(printerCombo, CB_SETCURSEL, 0, 0);
}

void MainWindow::CreateElementInputs()
{
    DestroyElementInputs();
    elementInputs.clear();

    for (std::size_t i = 0; i < labelTemplate.elements.size(); ++i)
    {
        const LabelElement& element = labelTemplate.elements[i];
        std::wstring label = ToWide(element.name.empty() ? "Element" : element.name);
        std::wstring value = ToWide(element.text);

        ElementInput input;
        input.elementIndex = i;
        input.labelHandle = CreateWindowExW(0, L"STATIC", label.c_str(), WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, windowHandle, nullptr, nullptr, nullptr);
        DWORD editStyle = WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL;
        if (!element.editable)
        {
            editStyle |= ES_READONLY;
        }

        input.valueEdit = CreateWindowExW(
            WS_EX_CLIENTEDGE,
            L"EDIT",
            value.c_str(),
            editStyle,
            0,
            0,
            0,
            0,
            windowHandle,
            reinterpret_cast<HMENU>(static_cast<INT_PTR>(FirstInputId + i * InputsPerElement)),
            nullptr,
            nullptr);

        input.xEdit = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", nullptr, WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_NUMBER, 0, 0, 0, 0, windowHandle, reinterpret_cast<HMENU>(static_cast<INT_PTR>(FirstInputId + i * InputsPerElement + 1)), nullptr, nullptr);
        input.yEdit = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", nullptr, WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_NUMBER, 0, 0, 0, 0, windowHandle, reinterpret_cast<HMENU>(static_cast<INT_PTR>(FirstInputId + i * InputsPerElement + 2)), nullptr, nullptr);
        input.sizeEdit = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", nullptr, WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_NUMBER, 0, 0, 0, 0, windowHandle, reinterpret_cast<HMENU>(static_cast<INT_PTR>(FirstInputId + i * InputsPerElement + 3)), nullptr, nullptr);
        input.optionCheck = CreateWindowExW(0, L"BUTTON", element.type == LabelElementType::Barcode ? L"Human" : L"Bold", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX, 0, 0, 0, 0, windowHandle, reinterpret_cast<HMENU>(static_cast<INT_PTR>(FirstInputId + i * InputsPerElement + 4)), nullptr, nullptr);

        SetWindowIntValue(input.xEdit, element.x);
        SetWindowIntValue(input.yEdit, element.y);
        SetWindowIntValue(input.sizeEdit, element.type == LabelElementType::Barcode ? element.barcodeHeight : element.fontHeight);
        SendMessageW(input.optionCheck, BM_SETCHECK, element.type == LabelElementType::Barcode ? (element.barcodeHumanReadable ? BST_CHECKED : BST_UNCHECKED) : (element.bold ? BST_CHECKED : BST_UNCHECKED), 0);

        if (element.type == LabelElementType::Barcode)
        {
            input.symbologyCombo = CreateWindowExW(0, L"COMBOBOX", nullptr, WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL, 0, 0, 0, 0, windowHandle, reinterpret_cast<HMENU>(static_cast<INT_PTR>(FirstInputId + i * InputsPerElement + 5)), nullptr, nullptr);
            SendMessageW(input.symbologyCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Code128"));
            SendMessageW(input.symbologyCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Code39"));
            SendMessageW(input.symbologyCombo, CB_SETCURSEL, element.barcodeSymbology == BarcodeSymbology::Code39 ? 1 : 0, 0);
        }

        elementInputs.push_back(input);
    }
}

void MainWindow::DestroyElementInputs()
{
    for (const ElementInput& input : elementInputs)
    {
        if (input.labelHandle) DestroyWindow(input.labelHandle);
        if (input.valueEdit) DestroyWindow(input.valueEdit);
        if (input.xEdit) DestroyWindow(input.xEdit);
        if (input.yEdit) DestroyWindow(input.yEdit);
        if (input.sizeEdit) DestroyWindow(input.sizeEdit);
        if (input.optionCheck) DestroyWindow(input.optionCheck);
        if (input.symbologyCombo) DestroyWindow(input.symbologyCombo);
    }
}

void MainWindow::SyncInputsToTemplate()
{
    SyncSettingsFromControls();

    for (const ElementInput& input : elementInputs)
    {
        if (input.elementIndex < labelTemplate.elements.size())
        {
            LabelElement& element = labelTemplate.elements[input.elementIndex];
            if (element.editable)
            {
                element.text = ToNarrow(GetWindowTextValue(input.valueEdit));
            }

            element.x = GetWindowIntValue(input.xEdit, element.x);
            element.y = GetWindowIntValue(input.yEdit, element.y);
            if (element.type == LabelElementType::Barcode)
            {
                element.barcodeHeight = GetWindowIntValue(input.sizeEdit, element.barcodeHeight);
                element.barcodeHumanReadable = SendMessageW(input.optionCheck, BM_GETCHECK, 0, 0) == BST_CHECKED;
                if (input.symbologyCombo)
                {
                    int selected = static_cast<int>(SendMessageW(input.symbologyCombo, CB_GETCURSEL, 0, 0));
                    element.barcodeSymbology = selected == 1 ? BarcodeSymbology::Code39 : BarcodeSymbology::Code128;
                }
            }
            else
            {
                int fontHeight = GetWindowIntValue(input.sizeEdit, element.fontHeight);
                element.fontHeight = fontHeight;
                element.fontWidth = fontHeight;
                element.bold = SendMessageW(input.optionCheck, BM_GETCHECK, 0, 0) == BST_CHECKED;
            }
        }
    }

    previewWidget.SetTemplate(labelTemplate);
}

void MainWindow::SyncSettingsFromControls()
{
    printerSettings.darkness = GetWindowIntValue(darknessEdit, printerSettings.darkness);
    printerSettings.printSpeed = GetWindowIntValue(speedEdit, printerSettings.printSpeed);
    labelTemplate.labelWidthDots = GetWindowIntValue(labelWidthEdit, labelTemplate.labelWidthDots);
    labelTemplate.labelHeightDots = GetWindowIntValue(labelHeightEdit, labelTemplate.labelHeightDots);
}

void MainWindow::PrintTemplate()
{
    SyncInputsToTemplate();

    if (!ValidateCurrentTemplate(L"Cannot Print"))
    {
        return;
    }

    if (!printersAvailable)
    {
        ShowMessage(L"No installed printer is available for printing.", L"Printer Required", MB_ICONWARNING);
        return;
    }

    int selected = static_cast<int>(SendMessageW(printerCombo, CB_GETCURSEL, 0, 0));
    if (selected == CB_ERR)
    {
        ShowMessage(L"Select a printer before printing.", L"Printer Required", MB_ICONWARNING);
        return;
    }

    wchar_t printerName[512] = {};
    SendMessageW(printerCombo, CB_GETLBTEXT, selected, reinterpret_cast<LPARAM>(printerName));

    ZebraPrinter printer;
    printer.setPrinterName(printerName);

    ZplGenerationResult generated = ZplGenerator::tryGenerate(labelTemplate, printerSettings);
    if (!generated.success)
    {
        ShowMessage(ToWide(generated.errorMessage), L"ZPL Failed", MB_ICONERROR);
        return;
    }

    const int copyCount = std::max(1, GetWindowIntValue(copiesEdit, 1));
    std::string printError;
    bool allPrinted = true;
    for (int copy = 0; copy < copyCount; ++copy)
    {
        if (!printer.printZpl(generated.zpl, printError))
        {
            allPrinted = false;
            break;
        }
    }

    if (allPrinted)
    {
        std::ostringstream message;
        message << "Printed " << copyCount << " label" << (copyCount == 1 ? "" : "s") << ".";
        AppendHistory(printerName, copyCount, true, message.str());
        UpdateStatus(ToWide(message.str()));
        ShowMessage(ToWide(message.str()), L"Print Complete", MB_ICONINFORMATION);
    }
    else
    {
        AppendHistory(printerName, copyCount, false, printError);
        UpdateStatus(ToWide(printError));
        ShowMessage(ToWide(printError), L"Print Failed", MB_ICONERROR);
    }
}

void MainWindow::SaveTemplate()
{
    SyncInputsToTemplate();
    if (!ValidateCurrentTemplate(L"Cannot Save"))
    {
        return;
    }

    if (TemplateStorage::SaveToFile(labelTemplate, templatePath))
    {
        UpdateStatus(L"Template saved.");
        ShowMessage(L"Template saved.", L"Save Complete", MB_ICONINFORMATION);
    }
    else
    {
        UpdateStatus(L"Template could not be saved.");
        ShowMessage(L"Template could not be saved.", L"Save Failed", MB_ICONERROR);
    }
}

void MainWindow::NewTemplate()
{
    LabelTemplate fresh;
    fresh.name = "Untitled Label";
    fresh.labelWidthDots = 400;
    fresh.labelHeightDots = 240;
    fresh.addElement(LabelElement::Text("Text", "New Label", 30, 30, 35, 35, true));
    fresh.addElement(LabelElement::Barcode("Barcode", "123456", 30, 110, 60));

    labelTemplate = fresh;
    RefreshTemplateControls();
    UpdateStatus(L"New template created.");
}

void MainWindow::ReloadTemplate()
{
    labelTemplate = TemplateStorage::LoadFromFile(templatePath);
    RefreshTemplateControls();
    UpdateStatus(L"Template reloaded.");
}

void MainWindow::RefreshTemplateControls()
{
    SetWindowIntValue(labelWidthEdit, labelTemplate.labelWidthDots);
    SetWindowIntValue(labelHeightEdit, labelTemplate.labelHeightDots);
    CreateElementInputs();

    RECT rect = {};
    GetClientRect(windowHandle, &rect);
    LayoutControls(rect.right - rect.left, rect.bottom - rect.top);
    previewWidget.SetTemplate(labelTemplate);
}

void MainWindow::AppendHistory(const std::wstring& printerName, int copyCount, bool success, const std::string& message)
{
    PrintHistoryEntry entry;
    entry.templateName = labelTemplate.name;
    entry.printerName = printerName;
    entry.copyCount = copyCount;
    entry.success = success;
    entry.message = message;
    entry.printedAt = std::time(nullptr);
    printHistory.add(entry);
    RefreshHistoryList();
}

void MainWindow::RefreshHistoryList()
{
    SendMessageW(historyList, LB_RESETCONTENT, 0, 0);
    const auto& entries = printHistory.all();
    for (auto it = entries.rbegin(); it != entries.rend(); ++it)
    {
        std::wostringstream row;
        row << (it->success ? L"OK" : L"FAIL") << L"  x" << it->copyCount << L"  "
            << ToWide(it->templateName) << L"  " << ToWide(it->message);
        SendMessageW(historyList, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(row.str().c_str()));
    }
}

void MainWindow::UpdateStatus(const std::wstring& message)
{
    if (statusLabel)
    {
        SetWindowTextW(statusLabel, message.c_str());
    }
}

void MainWindow::ShowMessage(const std::wstring& message, const std::wstring& title, UINT icon)
{
    MessageBoxW(windowHandle, message.c_str(), title.c_str(), MB_OK | icon);
}

bool MainWindow::ValidateCurrentTemplate(const std::wstring& title)
{
    LabelValidationResult result = LabelValidation::ValidateTemplate(labelTemplate);
    if (result.isValid())
    {
        return true;
    }

    ShowMessage(ToValidationMessage(result), title, MB_ICONWARNING);
    return false;
}

std::wstring MainWindow::ToWide(const std::string& value)
{
    if (value.empty())
    {
        return L"";
    }

    int length = MultiByteToWideChar(CP_UTF8, 0, value.c_str(), -1, nullptr, 0);
    std::wstring output(length, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, value.c_str(), -1, output.data(), length);
    output.resize(length - 1);
    return output;
}

std::wstring MainWindow::ToValidationMessage(const LabelValidationResult& result)
{
    std::wostringstream message;
    message << L"Fix the label template before continuing:";

    const std::size_t issueCount = std::min<std::size_t>(result.issues.size(), 4);
    for (std::size_t i = 0; i < issueCount; ++i)
    {
        const LabelValidationIssue& issue = result.issues[i];
        message << L"\n- ";
        if (issue.hasElementIndex)
        {
            message << L"Element " << (issue.elementIndex + 1) << L": ";
        }
        message << ToWide(issue.message);
    }

    if (result.issues.size() > issueCount)
    {
        message << L"\n- Additional issues were found.";
    }

    return message.str();
}

std::string MainWindow::ToNarrow(const std::wstring& value)
{
    if (value.empty())
    {
        return "";
    }

    int length = WideCharToMultiByte(CP_UTF8, 0, value.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string output(length, '\0');
    WideCharToMultiByte(CP_UTF8, 0, value.c_str(), -1, output.data(), length, nullptr, nullptr);
    output.resize(length - 1);
    return output;
}

std::wstring MainWindow::GetWindowTextValue(HWND window)
{
    int length = GetWindowTextLengthW(window);
    std::wstring value(length + 1, L'\0');
    GetWindowTextW(window, value.data(), length + 1);
    value.resize(length);
    return value;
}

int MainWindow::GetWindowIntValue(HWND window, int fallback)
{
    std::wstring text = GetWindowTextValue(window);
    if (text.empty())
    {
        return fallback;
    }

    try
    {
        return std::stoi(text);
    }
    catch (const std::exception&)
    {
        return fallback;
    }
}

void MainWindow::SetWindowIntValue(HWND window, int value)
{
    std::wstring text = std::to_wstring(value);
    SetWindowTextW(window, text.c_str());
}

std::string MainWindow::ResolveTemplatePath()
{
    namespace fs = std::filesystem;

    std::vector<fs::path> searchRoots;
    searchRoots.push_back(fs::current_path());

    wchar_t modulePath[MAX_PATH] = {};
    if (GetModuleFileNameW(nullptr, modulePath, MAX_PATH) > 0)
    {
        searchRoots.push_back(fs::path(modulePath).parent_path());
    }

    for (fs::path root : searchRoots)
    {
        for (int depth = 0; depth < 5 && !root.empty(); ++depth)
        {
            fs::path candidate = root / "templates" / "default_label.json";
            if (fs::exists(candidate))
            {
                return candidate.string();
            }

            root = root.parent_path();
        }
    }

    return "templates/default_label.json";
}
