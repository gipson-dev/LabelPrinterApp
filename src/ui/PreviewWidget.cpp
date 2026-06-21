#include "PreviewWidget.h"

#include <algorithm>
#include <string>

namespace
{
    std::wstring ToWide(const std::string& value)
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
}

bool PreviewWidget::RegisterWindowClass(HINSTANCE instance)
{
    WNDCLASSW windowClass = {};
    windowClass.lpfnWndProc = PreviewWidget::WindowProc;
    windowClass.hInstance = instance;
    windowClass.lpszClassName = L"LabelPreviewWidget";
    windowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
    windowClass.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);

    return RegisterClassW(&windowClass) != 0 || GetLastError() == ERROR_CLASS_ALREADY_EXISTS;
}

HWND PreviewWidget::Create(HWND parent, const LabelTemplate& labelTemplate)
{
    currentTemplate = labelTemplate;
    windowHandle = CreateWindowExW(
        WS_EX_CLIENTEDGE,
        L"LabelPreviewWidget",
        nullptr,
        WS_CHILD | WS_VISIBLE,
        0,
        0,
        0,
        0,
        parent,
        nullptr,
        reinterpret_cast<HINSTANCE>(GetWindowLongPtrW(parent, GWLP_HINSTANCE)),
        this);

    return windowHandle;
}

void PreviewWidget::SetTemplate(const LabelTemplate& labelTemplate)
{
    currentTemplate = labelTemplate;
    if (windowHandle)
    {
        InvalidateRect(windowHandle, nullptr, TRUE);
    }
}

LRESULT CALLBACK PreviewWidget::WindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
    PreviewWidget* preview = nullptr;

    if (message == WM_NCCREATE)
    {
        auto* createStruct = reinterpret_cast<CREATESTRUCTW*>(lParam);
        preview = static_cast<PreviewWidget*>(createStruct->lpCreateParams);
        SetWindowLongPtrW(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(preview));
    }
    else
    {
        preview = reinterpret_cast<PreviewWidget*>(GetWindowLongPtrW(window, GWLP_USERDATA));
    }

    if (message == WM_PAINT && preview)
    {
        preview->Paint(window);
        return 0;
    }

    return DefWindowProcW(window, message, wParam, lParam);
}

void PreviewWidget::Paint(HWND window)
{
    PAINTSTRUCT paint = {};
    HDC dc = BeginPaint(window, &paint);

    RECT clientRect = {};
    GetClientRect(window, &clientRect);

    HBRUSH backgroundBrush = CreateSolidBrush(RGB(245, 247, 250));
    FillRect(dc, &clientRect, backgroundBrush);
    DeleteObject(backgroundBrush);

    RECT labelRect = GetLabelRect(clientRect);
    HBRUSH labelBrush = CreateSolidBrush(RGB(255, 255, 255));
    FillRect(dc, &labelRect, labelBrush);
    DeleteObject(labelBrush);
    FrameRect(dc, &labelRect, reinterpret_cast<HBRUSH>(GetStockObject(GRAY_BRUSH)));

    SetBkMode(dc, TRANSPARENT);
    SetTextColor(dc, RGB(25, 28, 33));

    HPEN gridPen = CreatePen(PS_DOT, 1, RGB(218, 224, 232));
    HPEN oldPen = reinterpret_cast<HPEN>(SelectObject(dc, gridPen));
    for (int dotX = 50; dotX < currentTemplate.labelWidthDots; dotX += 50)
    {
        int x = ScaleX(dotX, labelRect);
        MoveToEx(dc, x, labelRect.top, nullptr);
        LineTo(dc, x, labelRect.bottom);
    }
    for (int dotY = 50; dotY < currentTemplate.labelHeightDots; dotY += 50)
    {
        int y = ScaleY(dotY, labelRect);
        MoveToEx(dc, labelRect.left, y, nullptr);
        LineTo(dc, labelRect.right, y);
    }
    SelectObject(dc, oldPen);
    DeleteObject(gridPen);

    std::wstring dimensions = std::to_wstring(currentTemplate.labelWidthDots) + L" x " + std::to_wstring(currentTemplate.labelHeightDots) + L" dots";
    RECT dimensionsRect = { labelRect.left, labelRect.top - 22, labelRect.right, labelRect.top - 4 };
    DrawTextW(dc, dimensions.c_str(), -1, &dimensionsRect, DT_RIGHT | DT_SINGLELINE | DT_NOPREFIX);

    for (const LabelElement& element : currentTemplate.elements)
    {
        int x = ScaleX(element.x, labelRect);
        int y = ScaleY(element.y, labelRect);

        if (element.type == LabelElementType::Barcode)
        {
            int barcodeWidth = std::max(96, static_cast<int>(element.text.length()) * 12);
            int scaledWidth = ScaleX(element.x + barcodeWidth, labelRect) - x;
            int scaledHeight = ScaleY(element.y + element.barcodeHeight, labelRect) - y;
            RECT barcodeRect = { x, y, x + scaledWidth, y + std::max(24, scaledHeight) };

            HBRUSH barcodeBrush = CreateSolidBrush(RGB(30, 34, 40));
            int moduleWidth = std::max(2, element.barcodeModuleWidth * 2);
            for (int barX = barcodeRect.left; barX < barcodeRect.right; barX += moduleWidth * 3)
            {
                RECT bar = { barX, barcodeRect.top, barX + moduleWidth, barcodeRect.bottom };
                FillRect(dc, &bar, barcodeBrush);
            }
            DeleteObject(barcodeBrush);

            HPEN outlinePen = CreatePen(PS_SOLID, 1, RGB(70, 130, 180));
            HPEN previousPen = reinterpret_cast<HPEN>(SelectObject(dc, outlinePen));
            SelectObject(dc, GetStockObject(NULL_BRUSH));
            Rectangle(dc, barcodeRect.left, barcodeRect.top, barcodeRect.right, barcodeRect.bottom);
            SelectObject(dc, previousPen);
            DeleteObject(outlinePen);

            std::wstring text = ToWide(element.text);
            if (element.barcodeSymbology == BarcodeSymbology::Code39)
            {
                text = L"Code39: " + text;
            }
            else
            {
                text = L"Code128: " + text;
            }
            RECT textRect = { barcodeRect.left, barcodeRect.bottom + 4, barcodeRect.right + 80, barcodeRect.bottom + 24 };
            if (element.barcodeHumanReadable)
            {
                DrawTextW(dc, text.c_str(), -1, &textRect, DT_LEFT | DT_SINGLELINE | DT_NOPREFIX);
            }
        }
        else
        {
            int fontSize = std::max(12, static_cast<int>(ScaleY(element.fontHeight, labelRect) - labelRect.top));
            HFONT font = CreateFontW(
                fontSize,
                0,
                0,
                0,
                element.bold ? FW_BOLD : FW_NORMAL,
                FALSE,
                FALSE,
                FALSE,
                DEFAULT_CHARSET,
                OUT_DEFAULT_PRECIS,
                CLIP_DEFAULT_PRECIS,
                DEFAULT_QUALITY,
                DEFAULT_PITCH | FF_SWISS,
                L"Segoe UI");

            HFONT oldFont = reinterpret_cast<HFONT>(SelectObject(dc, font));
            std::wstring text = ToWide(element.text);
            RECT textRect = { x, y, labelRect.right - 8, y + fontSize + 8 };
            DrawTextW(dc, text.c_str(), -1, &textRect, DT_LEFT | DT_SINGLELINE | DT_NOPREFIX);
            SelectObject(dc, oldFont);
            DeleteObject(font);

            HPEN outlinePen = CreatePen(PS_DOT, 1, RGB(120, 148, 180));
            HPEN previousPen = reinterpret_cast<HPEN>(SelectObject(dc, outlinePen));
            SelectObject(dc, GetStockObject(NULL_BRUSH));
            Rectangle(dc, textRect.left, textRect.top, textRect.right, textRect.bottom);
            SelectObject(dc, previousPen);
            DeleteObject(outlinePen);
        }
    }

    EndPaint(window, &paint);
}

RECT PreviewWidget::GetLabelRect(const RECT& clientRect) const
{
    const int margin = 24;
    int availableWidth = std::max(1, static_cast<int>(clientRect.right - clientRect.left - margin * 2));
    int availableHeight = std::max(1, static_cast<int>(clientRect.bottom - clientRect.top - margin * 2));

    double labelAspect = static_cast<double>(currentTemplate.labelWidthDots) / std::max(1, currentTemplate.labelHeightDots);
    int width = availableWidth;
    int height = static_cast<int>(width / labelAspect);

    if (height > availableHeight)
    {
        height = availableHeight;
        width = static_cast<int>(height * labelAspect);
    }

    int left = clientRect.left + margin + (availableWidth - width) / 2;
    int top = clientRect.top + margin + (availableHeight - height) / 2;
    return { left, top, left + width, top + height };
}

int PreviewWidget::ScaleX(int value, const RECT& labelRect) const
{
    int width = labelRect.right - labelRect.left;
    return labelRect.left + (value * width / std::max(1, currentTemplate.labelWidthDots));
}

int PreviewWidget::ScaleY(int value, const RECT& labelRect) const
{
    int height = labelRect.bottom - labelRect.top;
    return labelRect.top + (value * height / std::max(1, currentTemplate.labelHeightDots));
}
