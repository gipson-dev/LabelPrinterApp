#include "ui/PreviewWidget.h"

#include <QMouseEvent>
#include <QPainter>
#include <QFontMetrics>
#include <algorithm>
#include <cmath>

#include "core/VariableResolver.h"

PreviewWidget::PreviewWidget(QWidget* parent)
    : QWidget(parent),
      template_(LabelTemplate::defaultTemplate())
{
    setMinimumSize(420, 260);
    setMouseTracking(true);
}

void PreviewWidget::setTemplate(const LabelTemplate& labelTemplate)
{
    template_ = labelTemplate;
    update();
}

void PreviewWidget::setVariables(const VariableContext& context)
{
    variables_ = context;
    update();
}

void PreviewWidget::setSelectedElement(int index)
{
    selectedElement_ = index;
    update();
}

void PreviewWidget::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), QColor(238, 241, 245));

    QRectF label = labelRect();
    painter.setPen(QPen(QColor(90, 90, 90), 1));
    painter.drawText(QRectF(label.left(), label.top() - 24, label.width(), 18), Qt::AlignCenter,
                     QString("%1\" x %2\"").arg(template_.settings.labelWidthInches, 0, 'f', 2).arg(template_.settings.labelHeightInches, 0, 'f', 2));
    painter.setPen(QPen(QColor(140, 140, 140), 1));
    for (double x = 0.0; x <= template_.settings.labelWidthInches + 0.001; x += 0.25)
    {
        const double wx = label.left() + x / template_.settings.labelWidthInches * label.width();
        painter.drawLine(QPointF(wx, label.top() - 12), QPointF(wx, label.top() - (std::abs(std::fmod(x, 1.0)) < 0.001 ? 24 : 16)));
    }
    for (double y = 0.0; y <= template_.settings.labelHeightInches + 0.001; y += 0.25)
    {
        const double wy = label.top() + y / template_.settings.labelHeightInches * label.height();
        painter.drawLine(QPointF(label.left() - 12, wy), QPointF(label.left() - (std::abs(std::fmod(y, 1.0)) < 0.001 ? 24 : 16), wy));
    }

    painter.setPen(QPen(QColor(96, 110, 128), 2));
    painter.setBrush(Qt::white);
    painter.drawRoundedRect(label, 5, 5);
    drawGrid(painter, label);

    for (int i = 0; i < static_cast<int>(template_.elements.size()); ++i)
    {
        const LabelElement& element = template_.elements[i];
        bool selected = i == selectedElement_;
        if (element.type == LabelElementType::Text)
        {
            drawTextElement(painter, element, label, selected);
        }
        else if (element.type == LabelElementType::QrCode)
        {
            drawQrElement(painter, element, label, selected);
        }
        else
        {
            drawBarcodeElement(painter, element, label, selected);
        }
    }
}

void PreviewWidget::mousePressEvent(QMouseEvent* event)
{
    int hit = hitTest(event->position());
    selectedElement_ = hit;
    emit elementSelected(hit);
    if (hit >= 0)
    {
        QRectF label = labelRect();
        QPointF labelPoint = widgetToLabel(event->position(), label);
        const LabelElement& element = template_.elements[hit];
        dragOffsetInches_ = QPointF(labelPoint.x() - element.xInches, labelPoint.y() - element.yInches);
        draggingElement_ = hit;
    }
    update();
}

void PreviewWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (draggingElement_ < 0 || draggingElement_ >= static_cast<int>(template_.elements.size()))
    {
        return;
    }

    QRectF label = labelRect();
    QPointF labelPoint = widgetToLabel(event->position(), label) - dragOffsetInches_;
    LabelElement& element = template_.elements[draggingElement_];
    element.xInches = std::max(0.0, labelPoint.x());
    element.yInches = std::max(0.0, labelPoint.y());
    emit elementMoved(draggingElement_, element.xInches, element.yInches);
    update();
}

void PreviewWidget::mouseReleaseEvent(QMouseEvent*)
{
    draggingElement_ = -1;
}

QRectF PreviewWidget::labelRect() const
{
    const double aspect = template_.settings.labelWidthInches / template_.settings.labelHeightInches;
    QRectF available = rect().adjusted(28, 28, -28, -28);
    double width = available.width();
    double height = width / aspect;
    if (height > available.height())
    {
        height = available.height();
        width = height * aspect;
    }
    return QRectF(available.center().x() - width / 2.0, available.center().y() - height / 2.0, width, height);
}

QRectF PreviewWidget::elementRect(const LabelElement& element, const QRectF& label) const
{
    QPointF topLeft = labelToWidget(element.xInches, element.yInches, label);
    double scaleX = label.width() / template_.settings.labelWidthInches;
    double scaleY = label.height() / template_.settings.labelHeightInches;

    if (element.type == LabelElementType::Text)
    {
        double width = std::max(36.0, element.boxWidthInches * scaleX);
        double lineHeight = std::max(14.0, element.fontHeightDots / static_cast<double>(template_.settings.dpi) * scaleY);
        int lines = element.wrap || element.multiLine ? element.maxLines : 1;
        return QRectF(topLeft, QSizeF(width, lineHeight * lines + 12));
    }

    if (element.type == LabelElementType::QrCode)
    {
        double size = std::max(28.0, element.qrMagnification * 0.055 * scaleY);
        return QRectF(topLeft, QSizeF(size, size));
    }

    double module = element.barcodeModuleWidth / static_cast<double>(template_.settings.dpi) * scaleX;
    double width = std::max(48.0, static_cast<double>(std::max<std::size_t>(8, VariableResolver::elementValue(element, variables_).size())) * module * 9.0);
    double height = element.barcodeHeightDots / static_cast<double>(template_.settings.dpi) * scaleY;
    double humanText = element.humanReadable ? std::max(14.0, height * 0.22) : 0.0;
    return QRectF(topLeft, QSizeF(width, std::max(22.0, height) + humanText));
}

QPointF PreviewWidget::labelToWidget(double xInches, double yInches, const QRectF& label) const
{
    return QPointF(
        label.left() + xInches / template_.settings.labelWidthInches * label.width(),
        label.top() + yInches / template_.settings.labelHeightInches * label.height());
}

QPointF PreviewWidget::widgetToLabel(const QPointF& point, const QRectF& label) const
{
    return QPointF(
        (point.x() - label.left()) / label.width() * template_.settings.labelWidthInches,
        (point.y() - label.top()) / label.height() * template_.settings.labelHeightInches);
}

int PreviewWidget::hitTest(const QPointF& point) const
{
    QRectF label = labelRect();
    for (int i = static_cast<int>(template_.elements.size()) - 1; i >= 0; --i)
    {
        if (elementRect(template_.elements[i], label).contains(point))
        {
            return i;
        }
    }
    return -1;
}

void PreviewWidget::drawGrid(QPainter& painter, const QRectF& label) const
{
    painter.save();
    painter.setPen(QPen(QColor(226, 232, 240), 1, Qt::DotLine));
    for (double x = 0.25; x < template_.settings.labelWidthInches; x += 0.25)
    {
        double wx = label.left() + x / template_.settings.labelWidthInches * label.width();
        painter.drawLine(QPointF(wx, label.top()), QPointF(wx, label.bottom()));
    }
    for (double y = 0.25; y < template_.settings.labelHeightInches; y += 0.25)
    {
        double wy = label.top() + y / template_.settings.labelHeightInches * label.height();
        painter.drawLine(QPointF(label.left(), wy), QPointF(label.right(), wy));
    }
    painter.restore();
}

void PreviewWidget::drawTextElement(QPainter& painter, const LabelElement& element, const QRectF& label, bool selected) const
{
    QRectF box = elementRect(element, label);
    VariableContext context = variables_;
    if (context.serialNumber == 0)
    {
        context.serialNumber = 1;
    }
    for (const auto& placeholder : VariableResolver::findPlaceholders(element.text))
    {
        if (context.values.find(placeholder.first) == context.values.end())
        {
            context.values[placeholder.first] = placeholder.first == "ItemNumber" ? "A100-001" :
                                                placeholder.first == "Description" ? "Sample Item" :
                                                placeholder.first == "Lot" ? "LOT-001" :
                                                placeholder.first == "Bin" ? "A-01" :
                                                placeholder.first == "Quantity" ? "1" :
                                                "Sample";
        }
    }
    QString value = QString::fromStdString(VariableResolver::elementValue(element, context));
    double scaleY = label.height() / template_.settings.labelHeightInches;
    int pixelSize = std::max(18, static_cast<int>(element.fontHeightDots / static_cast<double>(template_.settings.dpi) * scaleY));

    QFont font("Arial");
    font.setPixelSize(pixelSize);
    font.setBold(element.bold);
    font.setItalic(element.italic);
    font.setUnderline(element.underline);

    painter.save();
    painter.setFont(font);
    painter.setPen(Qt::black);
    QFontMetrics metrics(font);
    box.setHeight(std::max(box.height(), static_cast<double>(metrics.height() + 8)));

    int flags = Qt::AlignTop;
    if (element.alignment == TextAlignment::Center) flags |= Qt::AlignHCenter;
    else if (element.alignment == TextAlignment::Right) flags |= Qt::AlignRight;
    else flags |= Qt::AlignLeft;
    if (element.wrap || element.multiLine) flags |= Qt::TextWordWrap;

    QRectF textBox = box.adjusted(4, 2, -4, -2);
    painter.drawText(textBox, flags, value);
    painter.setPen(QPen(selected ? QColor(0, 120, 215) : QColor(120, 148, 180), selected ? 2 : 1, selected ? Qt::SolidLine : Qt::DashLine));
    painter.drawRect(box);
    painter.restore();
}

void PreviewWidget::drawBarcodeElement(QPainter& painter, const LabelElement& element, const QRectF& label, bool selected) const
{
    QRectF box = elementRect(element, label);
    VariableContext context = variables_;
    for (const auto& placeholder : VariableResolver::findPlaceholders(element.text))
    {
        if (context.values.find(placeholder.first) == context.values.end())
        {
            context.values[placeholder.first] = placeholder.first == "ItemNumber" ? "A100-001" : "123456";
        }
    }
    QString value = QString::fromStdString(VariableResolver::elementValue(element, context));
    double scaleX = label.width() / template_.settings.labelWidthInches;
    double barHeight = element.barcodeHeightDots / static_cast<double>(template_.settings.dpi) * (label.height() / template_.settings.labelHeightInches);
    QRectF bars = QRectF(box.left(), box.top(), box.width(), std::min(box.height(), std::max(18.0, barHeight)));
    painter.save();
    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::black);
    int module = std::max(2, static_cast<int>(element.barcodeModuleWidth / static_cast<double>(template_.settings.dpi) * scaleX));
    for (int x = static_cast<int>(bars.left()); x < static_cast<int>(bars.right()); x += module * 3)
    {
        painter.drawRect(QRectF(x, bars.top(), module, bars.height()));
    }
    painter.setBrush(Qt::NoBrush);
    painter.setPen(QPen(selected ? QColor(0, 120, 215) : QColor(70, 130, 180), selected ? 2 : 1));
    painter.drawRect(box);
    if (element.humanReadable)
    {
        QFont font("Arial");
        font.setPixelSize(std::max(10, static_cast<int>(bars.height() * 0.22)));
        painter.setFont(font);
        painter.setPen(Qt::black);
        painter.drawText(QRectF(box.left(), bars.bottom() + 2, box.width(), box.bottom() - bars.bottom()), Qt::AlignCenter, value);
    }
    painter.restore();
}

void PreviewWidget::drawQrElement(QPainter& painter, const LabelElement& element, const QRectF& label, bool selected) const
{
    QRectF box = elementRect(element, label);
    VariableContext context = variables_;
    for (const auto& placeholder : VariableResolver::findPlaceholders(element.text))
    {
        if (context.values.find(placeholder.first) == context.values.end())
        {
            context.values[placeholder.first] = placeholder.first == "ItemNumber" ? "A100-001" : "Sample";
        }
    }
    painter.save();
    painter.setPen(QPen(selected ? QColor(0, 120, 215) : Qt::black, selected ? 2 : 1));
    painter.setBrush(Qt::white);
    painter.drawRect(box);
    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::black);
    const int cells = 9;
    const double cell = box.width() / cells;
    for (int y = 0; y < cells; ++y)
    {
        for (int x = 0; x < cells; ++x)
        {
            if ((x < 3 && y < 3) || (x > 5 && y < 3) || (x < 3 && y > 5) || ((x * 3 + y * 5) % 7 < 3))
            {
                painter.drawRect(QRectF(box.left() + x * cell, box.top() + y * cell, cell * 0.82, cell * 0.82));
            }
        }
    }
    painter.restore();
}
