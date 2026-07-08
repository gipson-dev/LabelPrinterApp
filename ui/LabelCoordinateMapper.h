#pragma once

#include <QPointF>
#include <QRectF>

#include "core/LabelGeometry.h"

class LabelCoordinateMapper
{
public:
    LabelCoordinateMapper() = default;
    LabelCoordinateMapper(double labelWidthDots, double labelHeightDots, const QRectF& labelScreenRect)
        : labelWidthDots_(labelWidthDots),
          labelHeightDots_(labelHeightDots),
          labelScreenRect_(labelScreenRect)
    {
    }

    QRectF labelScreenRect() const { return labelScreenRect_; }

    QPointF labelToScreen(const LabelPoint& point) const
    {
        return {
            labelScreenRect_.left() + point.x * xScale(),
            labelScreenRect_.top() + point.y * yScale()
        };
    }

    QPointF screenToLabel(const QPointF& point) const
    {
        return {
            (point.x() - labelScreenRect_.left()) / xScale(),
            (point.y() - labelScreenRect_.top()) / yScale()
        };
    }

    QRectF labelToScreen(const LabelRect& rect) const
    {
        const QPointF topLeft = labelToScreen(LabelPoint{rect.x, rect.y});
        return {topLeft, QSizeF(labelLengthToScreenX(rect.width), labelLengthToScreenY(rect.height))};
    }

    LabelRect screenToLabel(const QRectF& rect) const
    {
        const QPointF topLeft = screenToLabel(rect.topLeft());
        return {
            topLeft.x(),
            topLeft.y(),
            screenLengthToLabelX(rect.width()),
            screenLengthToLabelY(rect.height())
        };
    }

    double labelLengthToScreenX(double dots) const { return dots * xScale(); }
    double labelLengthToScreenY(double dots) const { return dots * yScale(); }
    double screenLengthToLabelX(double pixels) const { return pixels / xScale(); }
    double screenLengthToLabelY(double pixels) const { return pixels / yScale(); }

private:
    double xScale() const { return labelWidthDots_ > 0.0 ? labelScreenRect_.width() / labelWidthDots_ : 1.0; }
    double yScale() const { return labelHeightDots_ > 0.0 ? labelScreenRect_.height() / labelHeightDots_ : 1.0; }

    double labelWidthDots_ = 1.0;
    double labelHeightDots_ = 1.0;
    QRectF labelScreenRect_;
};
