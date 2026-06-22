#include "PreviewWidget.h"

#include <algorithm>
#include <string>
#include <windowsx.h>

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

    if (message == WM_LBUTTONDOWN && preview)
    {
        preview->BeginDrag(window, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        return 0;
    }

    if (message == WM_MOUSEMOVE && preview)
    {
        if (wParam & MK_LBUTTON)
        {
            preview->DragTo(window, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        }
        return 0;
    }

    if (message == WM_LBUTTONUP && preview)
    {
        preview->EndDrag(window);
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
            RECT barcodeRect = GetElementRect(element, labelRect);

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
            RECT textRect = GetElementRect(element, labelRect);
            int format = DT_LEFT | DT_NOPREFIX | DT_TOP;
            if (element.alignment == TextAlignment::Center)
            {
                format = DT_CENTER | DT_NOPREFIX | DT_TOP;
            }
            else if (element.alignment == TextAlignment::Right)
            {
                format = DT_RIGHT | DT_NOPREFIX | DT_TOP;
            }
            if (!element.wrap && !element.multiline)
            {
                format |= DT_SINGLELINE;
            }
            DrawTextW(dc, text.c_str(), -1, &textRect, format);
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

void PreviewWidget::BeginDrag(HWND window, int x, int y)
{
    RECT clientRect = {};
    GetClientRect(window, &clientRect);
    RECT labelRect = GetLabelRect(clientRect);
    draggingElement = HitTest(x, y, labelRect);
    if (draggingElement < 0)
    {
        return;
    }

    const LabelElement& element = currentTemplate.elements[draggingElement];
    dragOffsetX = UnscaleX(x, labelRect) - element.x;
    dragOffsetY = UnscaleY(y, labelRect) - element.y;
    SetCapture(window);
}

void PreviewWidget::DragTo(HWND window, int x, int y)
{
    if (draggingElement < 0 || static_cast<std::size_t>(draggingElement) >= currentTemplate.elements.size())
    {
        return;
    }

    RECT clientRect = {};
    GetClientRect(window, &clientRect);
    RECT labelRect = GetLabelRect(clientRect);

    LabelElement& element = currentTemplate.elements[draggingElement];
    element.x = std::max(0, UnscaleX(x, labelRect) - dragOffsetX);
    element.y = std::max(0, UnscaleY(y, labelRect) - dragOffsetY);
    InvalidateRect(window, nullptr, TRUE);

    HWND parent = GetParent(window);
    if (parent)
    {
        LPARAM packedPosition = MAKELPARAM(static_cast<WORD>(element.x), static_cast<WORD>(element.y));
        SendMessageW(parent, PositionChangedMessage, static_cast<WPARAM>(draggingElement), packedPosition);
    }
}

void PreviewWidget::EndDrag(HWND window)
{
    if (draggingElement >= 0)
    {
        ReleaseCapture();
    }
    draggingElement = -1;
}

int PreviewWidget::HitTest(int x, int y, const RECT& labelRect) const
{
    for (int i = static_cast<int>(currentTemplate.elements.size()) - 1; i >= 0; --i)
    {
        RECT rect = GetElementRect(currentTemplate.elements[i], labelRect);
        if (x >= rect.left && x <= rect.right && y >= rect.top && y <= rect.bottom)
        {
            return i;
        }
    }

    return -1;
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

int PreviewWidget::UnscaleX(int value, const RECT& labelRect) const
{
    int width = labelRect.right - labelRect.left;
    return (value - labelRect.left) * std::max(1, currentTemplate.labelWidthDots) / std::max(1, width);
}

int PreviewWidget::UnscaleY(int value, const RECT& labelRect) const
{
    int height = labelRect.bottom - labelRect.top;
    return (value - labelRect.top) * std::max(1, currentTemplate.labelHeightDots) / std::max(1, height);
}

RECT PreviewWidget::GetElementRect(const LabelElement& element, const RECT& labelRect) const
{
    int x = ScaleX(element.x, labelRect);
    int y = ScaleY(element.y, labelRect);

    if (element.type == LabelElementType::Barcode)
    {
        int barcodeWidth = std::max(96, static_cast<int>(element.text.length()) * element.barcodeModuleWidth * 10);
        int scaledWidth = ScaleX(element.x + barcodeWidth, labelRect) - x;
        int scaledHeight = ScaleY(element.y + element.barcodeHeight, labelRect) - y;
        return { x, y, x + std::max(24, scaledWidth), y + std::max(24, scaledHeight) };
    }

    int fontSize = std::max(12, static_cast<int>(ScaleY(element.fontHeight, labelRect) - labelRect.top));
    int width = ScaleX(element.x + element.boxWidth, labelRect) - x;
    int lines = element.multiline || element.wrap ? element.maxLines : 1;
    int height = std::max(fontSize + 8, lines * (fontSize + 4));
    int previewTop = y - std::max(0, fontSize / 6);
    return { x, previewTop, x + std::max(40, width), previewTop + height };
}
