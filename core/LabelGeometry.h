#pragma once

#include <algorithm>

struct LabelPoint
{
    double x = 0.0;
    double y = 0.0;
};

struct LabelSize
{
    double width = 0.0;
    double height = 0.0;
};

struct LabelRect
{
    double x = 0.0;
    double y = 0.0;
    double width = 0.0;
    double height = 0.0;

    double left() const { return x; }
    double top() const { return y; }
    double right() const { return x + width; }
    double bottom() const { return y + height; }
    double centerX() const { return x + width / 2.0; }
    double centerY() const { return y + height / 2.0; }

    bool contains(LabelPoint point) const
    {
        return point.x >= left() && point.x <= right() && point.y >= top() && point.y <= bottom();
    }

    LabelRect normalized() const
    {
        LabelRect rect = *this;
        if (rect.width < 0.0)
        {
            rect.x += rect.width;
            rect.width = -rect.width;
        }
        if (rect.height < 0.0)
        {
            rect.y += rect.height;
            rect.height = -rect.height;
        }
        return rect;
    }
};

inline LabelRect unitedLabelRect(const LabelRect& a, const LabelRect& b)
{
    const double left = std::min(a.left(), b.left());
    const double top = std::min(a.top(), b.top());
    const double right = std::max(a.right(), b.right());
    const double bottom = std::max(a.bottom(), b.bottom());
    return {left, top, right - left, bottom - top};
}

