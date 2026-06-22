#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include "LabelPrinterApp/LabelTemplate.h"

class PreviewWidget
{
public:
    static constexpr UINT PositionChangedMessage = WM_APP + 42;

    bool RegisterWindowClass(HINSTANCE instance);
    HWND Create(HWND parent, const LabelTemplate& labelTemplate);
    void SetTemplate(const LabelTemplate& labelTemplate);

private:
    static LRESULT CALLBACK WindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam);

    void Paint(HWND window);
    void BeginDrag(HWND window, int x, int y);
    void DragTo(HWND window, int x, int y);
    void EndDrag(HWND window);
    int HitTest(int x, int y, const RECT& labelRect) const;
    RECT GetLabelRect(const RECT& clientRect) const;
    int ScaleX(int value, const RECT& labelRect) const;
    int ScaleY(int value, const RECT& labelRect) const;
    int UnscaleX(int value, const RECT& labelRect) const;
    int UnscaleY(int value, const RECT& labelRect) const;
    RECT GetElementRect(const LabelElement& element, const RECT& labelRect) const;

    HWND windowHandle = nullptr;
    LabelTemplate currentTemplate;
    int draggingElement = -1;
    int dragOffsetX = 0;
    int dragOffsetY = 0;
};
