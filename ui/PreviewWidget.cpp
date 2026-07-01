#include "ui/PreviewWidget.h"

#include <QEvent>
#include <QKeyEvent>
#include <QLineEdit>
#include <QMouseEvent>
#include <QPainter>
#include <QFontMetrics>
#include <algorithm>
#include <array>
#include <cmath>
#include <vector>

#include "core/BarcodeMetrics.h"
#include "core/SampleData.h"
#include "core/VariableResolver.h"

namespace
{
double safeLabelWidth(const LabelTemplate& labelTemplate)
{
    return std::max(0.1, labelTemplate.settings.labelWidthInches);
}

double safeLabelHeight(const LabelTemplate& labelTemplate)
{
    return std::max(0.1, labelTemplate.settings.labelHeightInches);
}

int rotationDegrees(ElementRotation rotation)
{
    switch (rotation)
    {
    case ElementRotation::Deg90:
        return 90;
    case ElementRotation::Deg180:
        return 180;
    case ElementRotation::Deg270:
        return 270;
    default:
        return 0;
    }
}

std::vector<int> code128Modules(const std::string& value)
{
    static const std::array<const char*, 107> patterns = {
        "212222", "222122", "222221", "121223", "121322", "131222", "122213", "122312", "132212", "221213",
        "221312", "231212", "112232", "122132", "122231", "113222", "123122", "123221", "223211", "221132",
        "221231", "213212", "223112", "312131", "311222", "321122", "321221", "312212", "322112", "322211",
        "212123", "212321", "232121", "111323", "131123", "131321", "112313", "132113", "132311", "211313",
        "231113", "231311", "112133", "112331", "132131", "113123", "113321", "133121", "313121", "211331",
        "231131", "213113", "213311", "213131", "311123", "311321", "331121", "312113", "312311", "332111",
        "314111", "221411", "431111", "111224", "111422", "121124", "121421", "141122", "141221", "112214",
        "112412", "122114", "122411", "142112", "142211", "241211", "221114", "413111", "241112", "134111",
        "111242", "121142", "121241", "114212", "124112", "124211", "411212", "421112", "421211", "212141",
        "214121", "412121", "111143", "111341", "131141", "114113", "114311", "411113", "411311", "113141",
        "114131", "311141", "411131", "211412", "211214", "211232", "2331112"
    };

    std::vector<int> codes;
    codes.push_back(104);
    for (unsigned char ch : value)
    {
        if (ch < 32 || ch > 127)
        {
            ch = '?';
        }
        codes.push_back(static_cast<int>(ch) - 32);
    }

    int checksum = codes.front();
    for (std::size_t i = 1; i < codes.size(); ++i)
    {
        checksum += codes[i] * static_cast<int>(i);
    }
    codes.push_back(checksum % 103);
    codes.push_back(106);

    std::vector<int> modules;
    for (int code : codes)
    {
        const char* pattern = patterns[static_cast<std::size_t>(code)];
        for (const char* p = pattern; *p; ++p)
        {
            modules.push_back(*p - '0');
        }
    }
    return modules;
}

void drawSelectionFrame(QPainter& painter, const QRectF& box, bool selected)
{
    painter.setPen(QPen(selected ? QColor(0, 84, 190) : QColor(53, 93, 135), selected ? 3 : 2, selected ? Qt::SolidLine : Qt::DashLine));
    painter.drawRect(box);
    if (!selected)
    {
        return;
    }

    const double size = 10.0;
    const QPointF points[] = {
        box.topLeft(), QPointF(box.center().x(), box.top()), box.topRight(),
        QPointF(box.left(), box.center().y()), QPointF(box.right(), box.center().y()),
        box.bottomLeft(), QPointF(box.center().x(), box.bottom()), box.bottomRight()
    };
    for (const QPointF& point : points)
    {
        QRectF handle(point.x() - size / 2.0, point.y() - size / 2.0, size, size);
        painter.setBrush(Qt::white);
        painter.setPen(QPen(QColor(0, 84, 190), 2));
        painter.drawRect(handle);
        painter.setBrush(QColor(0, 84, 190));
        painter.setPen(Qt::NoPen);
        painter.drawRect(handle.adjusted(3, 3, -3, -3));
    }
}
}

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
    for (auto it = selectedElements_.begin(); it != selectedElements_.end();)
    {
        if (*it < 0 || *it >= static_cast<int>(template_.elements.size()))
        {
            it = selectedElements_.erase(it);
        }
        else
        {
            ++it;
        }
    }
    if (selectedElement_ >= static_cast<int>(template_.elements.size()))
    {
        selectedElement_ = selectedElements_.isEmpty() ? -1 : selectedElements_.front();
    }
    updateInlineEditorGeometry();
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
    selectedElements_.clear();
    if (index >= 0)
    {
        selectedElements_.append(index);
    }
    if (inlineEditor_ && editingElement_ != selectedElement_)
    {
        commitInlineTextEdit();
    }
    update();
}

void PreviewWidget::setSelectedElements(const QList<int>& indexes)
{
    selectedElements_.clear();
    for (int index : indexes)
    {
        if (index >= 0 && index < static_cast<int>(template_.elements.size()) && !selectedElements_.contains(index))
        {
            selectedElements_.append(index);
        }
    }
    selectedElement_ = selectedElements_.isEmpty() ? -1 : selectedElements_.front();
    if (inlineEditor_ && editingElement_ != selectedElement_)
    {
        commitInlineTextEdit();
    }
    update();
}

void PreviewWidget::setGridVisible(bool visible)
{
    gridVisible_ = visible;
    update();
}

void PreviewWidget::setSnapToGrid(bool enabled)
{
    snapToGrid_ = enabled;
}

void PreviewWidget::zoomIn()
{
    zoomFactor_ = std::min(4.0, zoomFactor_ * 1.2);
    updateInlineEditorGeometry();
    update();
}

void PreviewWidget::zoomOut()
{
    zoomFactor_ = std::max(0.35, zoomFactor_ / 1.2);
    updateInlineEditorGeometry();
    update();
}

void PreviewWidget::zoomFit()
{
    zoomFactor_ = 1.0;
    updateInlineEditorGeometry();
    update();
}

bool PreviewWidget::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == inlineEditor_)
    {
        if (event->type() == QEvent::KeyPress)
        {
            auto* keyEvent = static_cast<QKeyEvent*>(event);
            if (keyEvent->key() == Qt::Key_Escape)
            {
                cancelInlineTextEdit();
                return true;
            }
            if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter)
            {
                commitInlineTextEdit();
                return true;
            }
        }
        else if (event->type() == QEvent::FocusOut)
        {
            commitInlineTextEdit();
            return false;
        }
    }
    return QWidget::eventFilter(watched, event);
}

void PreviewWidget::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), QColor(214, 214, 214));

    QRectF label = labelRect();
    const double labelWidthInches = safeLabelWidth(template_);
    const double labelHeightInches = safeLabelHeight(template_);

    QRectF topRuler(label.left(), 0, label.width(), label.top() - 4);
    QRectF leftRuler(0, label.top(), label.left() - 4, label.height());
    painter.fillRect(topRuler, QColor(235, 235, 235));
    painter.fillRect(leftRuler, QColor(235, 235, 235));
    painter.setPen(QPen(QColor(120, 120, 120), 1));
    painter.drawLine(QPointF(label.left(), topRuler.bottom()), QPointF(label.right(), topRuler.bottom()));
    painter.drawLine(QPointF(leftRuler.right(), label.top()), QPointF(leftRuler.right(), label.bottom()));

    painter.setPen(QPen(QColor(70, 70, 70), 1));
    painter.drawText(QRectF(label.left(), label.top() - 24, label.width(), 18), Qt::AlignCenter,
                     QString("%1\" x %2\"").arg(labelWidthInches, 0, 'f', 2).arg(labelHeightInches, 0, 'f', 2));
    painter.setPen(QPen(QColor(140, 140, 140), 1));
    for (double x = 0.0; x <= labelWidthInches + 0.001; x += 0.25)
    {
        const double wx = label.left() + x / labelWidthInches * label.width();
        painter.drawLine(QPointF(wx, label.top() - 12), QPointF(wx, label.top() - (std::abs(std::fmod(x, 1.0)) < 0.001 ? 24 : 16)));
    }
    for (double y = 0.0; y <= labelHeightInches + 0.001; y += 0.25)
    {
        const double wy = label.top() + y / labelHeightInches * label.height();
        painter.drawLine(QPointF(label.left() - 12, wy), QPointF(label.left() - (std::abs(std::fmod(y, 1.0)) < 0.001 ? 24 : 16), wy));
    }

    painter.setPen(QPen(QColor(96, 110, 128), 2));
    painter.setBrush(Qt::white);
    painter.drawRoundedRect(label, 5, 5);
    painter.setPen(QPen(QColor(40, 40, 40), 1, Qt::DashLine));
    painter.setBrush(Qt::NoBrush);
    painter.drawRoundedRect(label.adjusted(8, 8, -8, -8), 4, 4);
    if (gridVisible_)
    {
        drawGrid(painter, label);
    }

    if (template_.elements.empty())
    {
        return;
    }

    for (int i = 0; i < static_cast<int>(template_.elements.size()); ++i)
    {
        const LabelElement& element = template_.elements[i];
        bool selected = isSelected(i);
        if (element.type == LabelElementType::Text)
        {
            drawTextElement(painter, element, label, selected);
        }
        else if (element.type == LabelElementType::QrCode)
        {
            drawQrElement(painter, element, label, selected);
        }
        else if (element.type == LabelElementType::Line || element.type == LabelElementType::Box)
        {
            drawShapeElement(painter, element, label, selected);
        }
        else
        {
            drawBarcodeElement(painter, element, label, selected);
        }
    }

    if (marqueeSelecting_)
    {
        painter.setBrush(QColor(0, 84, 190, 35));
        painter.setPen(QPen(QColor(0, 84, 190), 1, Qt::DashLine));
        painter.drawRect(marqueeRect_.normalized());
    }
}

void PreviewWidget::mousePressEvent(QMouseEvent* event)
{
    if (inlineEditor_)
    {
        commitInlineTextEdit();
    }

    if (selectedElement_ >= 0 && selectedElement_ < static_cast<int>(template_.elements.size()))
    {
        const int handle = resizeHandleAt(event->position(), selectedElement_);
        if (handle >= 0)
        {
            emit elementSelected(selectedElement_);
            if (!template_.elements[selectedElement_].locked)
            {
                QRectF label = labelRect();
                resizingElement_ = selectedElement_;
                resizeHandle_ = handle;
                resizeStartPointInches_ = widgetToLabel(event->position(), label);
                resizeStartRectInches_ = elementRectInches(template_.elements[selectedElement_], label);
                resizeStartElement_ = template_.elements[selectedElement_];
            }
            update();
            return;
        }
    }

    const int hit = hitTest(event->position());
    if (hit >= 0)
    {
        if (event->modifiers() & Qt::ControlModifier)
        {
            if (selectedElements_.contains(hit))
            {
                selectedElements_.removeAll(hit);
            }
            else
            {
                selectedElements_.append(hit);
            }
            selectedElement_ = selectedElements_.isEmpty() ? -1 : hit;
            emit elementsSelected(selectedElements_);
            update();
            return;
        }

        if (!isSelected(hit))
        {
            selectedElements_.clear();
            selectedElements_.append(hit);
        }
        selectedElement_ = hit;
        emit elementsSelected(selectedElements_);
        if (template_.elements[hit].locked)
        {
            update();
            return;
        }
        QRectF label = labelRect();
        QPointF labelPoint = widgetToLabel(event->position(), label);
        const LabelElement& element = template_.elements[hit];
        dragOffsetInches_ = QPointF(labelPoint.x() - element.xInches, labelPoint.y() - element.yInches);
        groupDragStartInches_ = labelPoint;
        groupDragIndexes_.clear();
        groupDragStartElements_.clear();
        for (int index : selectedElements_)
        {
            if (index >= 0 && index < static_cast<int>(template_.elements.size()) && !template_.elements[index].locked)
            {
                groupDragIndexes_.append(index);
                groupDragStartElements_.append(template_.elements[index]);
            }
        }
        draggingElement_ = hit;
    }
    else
    {
        selectedElement_ = -1;
        selectedElements_.clear();
        emit elementsSelected(selectedElements_);
        marqueeSelecting_ = labelRect().contains(event->position());
        marqueeStart_ = event->position();
        marqueeRect_ = QRectF(marqueeStart_, marqueeStart_);
    }
    update();
}

void PreviewWidget::mouseDoubleClickEvent(QMouseEvent* event)
{
    const int hit = hitTest(event->position());
    if (hit >= 0 && hit < static_cast<int>(template_.elements.size()))
    {
        selectedElement_ = hit;
        emit elementSelected(hit);
        const LabelElement& element = template_.elements[hit];
        if (element.type == LabelElementType::Text && !element.locked)
        {
            beginInlineTextEdit(hit);
            event->accept();
            return;
        }
    }
    QWidget::mouseDoubleClickEvent(event);
}

void PreviewWidget::mouseMoveEvent(QMouseEvent* event)
{
    QRectF label = labelRect();
    QPointF cursorPoint = widgetToLabel(event->position(), label);
    emit cursorPositionChanged(cursorPoint.x(), cursorPoint.y());

    if (resizingElement_ >= 0 && resizingElement_ < static_cast<int>(template_.elements.size()))
    {
        applyResize(cursorPoint);
        return;
    }

    if (draggingElement_ < 0 || draggingElement_ >= static_cast<int>(template_.elements.size()))
    {
        if (marqueeSelecting_)
        {
            marqueeRect_ = QRectF(marqueeStart_, event->position()).normalized();
            selectedElements_.clear();
            const QRectF label = labelRect();
            for (int i = 0; i < static_cast<int>(template_.elements.size()); ++i)
            {
                if (marqueeRect_.intersects(elementRect(template_.elements[i], label)))
                {
                    selectedElements_.append(i);
                }
            }
            selectedElement_ = selectedElements_.isEmpty() ? -1 : selectedElements_.front();
            update();
            return;
        }
        if (selectedElement_ >= 0 && resizeHandleAt(event->position(), selectedElement_) >= 0)
        {
            setCursor(Qt::SizeAllCursor);
        }
        else
        {
            unsetCursor();
        }
        return;
    }

    const QPointF currentLabelPoint = widgetToLabel(event->position(), label);
    QPointF delta = currentLabelPoint - groupDragStartInches_;
    if (snapToGrid_)
    {
        constexpr double gridStep = 0.25;
        delta.setX(std::round(delta.x() / gridStep) * gridStep);
        delta.setY(std::round(delta.y() / gridStep) * gridStep);
    }
    for (int i = 0; i < groupDragIndexes_.size(); ++i)
    {
        const int index = groupDragIndexes_[i];
        if (index < 0 || index >= static_cast<int>(template_.elements.size()))
        {
            continue;
        }
        LabelElement element = groupDragStartElements_[i];
        element.xInches = std::max(0.0, element.xInches + delta.x());
        element.yInches = std::max(0.0, element.yInches + delta.y());
        template_.elements[index] = element;
    }
    update();
}

void PreviewWidget::mouseReleaseEvent(QMouseEvent*)
{
    if (marqueeSelecting_)
    {
        marqueeSelecting_ = false;
        marqueeRect_ = QRectF();
        emit elementsSelected(selectedElements_);
        update();
    }
    if (draggingElement_ >= 0 && !groupDragIndexes_.isEmpty())
    {
        QList<LabelElement> changed;
        for (int index : groupDragIndexes_)
        {
            if (index >= 0 && index < static_cast<int>(template_.elements.size()))
            {
                changed.append(template_.elements[index]);
            }
        }
        emit elementsChanged(groupDragIndexes_, changed);
    }
    draggingElement_ = -1;
    resizingElement_ = -1;
    resizeHandle_ = -1;
    groupDragIndexes_.clear();
    groupDragStartElements_.clear();
}

void PreviewWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    updateInlineEditorGeometry();
}

QRectF PreviewWidget::labelRect() const
{
    const double aspect = safeLabelWidth(template_) / safeLabelHeight(template_);
    QRectF available = rect().adjusted(28, 28, -28, -28);
    double width = available.width();
    double height = width / aspect;
    if (height > available.height())
    {
        height = available.height();
        width = height * aspect;
    }
    width *= zoomFactor_;
    height *= zoomFactor_;
    return QRectF(available.center().x() - width / 2.0, available.center().y() - height / 2.0, width, height);
}

QRectF PreviewWidget::elementRect(const LabelElement& element, const QRectF& label) const
{
    QPointF topLeft = labelToWidget(element.xInches, element.yInches, label);
    double scaleX = label.width() / safeLabelWidth(template_);
    double scaleY = label.height() / safeLabelHeight(template_);

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

    if (element.type == LabelElementType::Line || element.type == LabelElementType::Box)
    {
        const double width = std::max(12.0, element.boxWidthInches * scaleX);
        const double heightDots = element.type == LabelElementType::Line ? std::max(2, element.fontWidthDots) : std::max(2, element.fontHeightDots);
        const double height = std::max(3.0, heightDots / static_cast<double>(template_.settings.dpi) * scaleY);
        return QRectF(topLeft, QSizeF(width, height));
    }

    VariableContext context = variables_;
    SampleData::fillMissingForElement(element, context);
    const std::string value = VariableResolver::elementValue(element, context);
    double width = std::max(48.0, BarcodeMetrics::barcodeWidthDots(element, value) / static_cast<double>(template_.settings.dpi) * scaleX);
    const double boxWidth = std::max(width, element.boxWidthInches * scaleX);
    if (element.alignment == TextAlignment::Center && boxWidth > width)
    {
        topLeft.setX(topLeft.x() + (boxWidth - width) / 2.0);
    }
    else if (element.alignment == TextAlignment::Right && boxWidth > width)
    {
        topLeft.setX(topLeft.x() + boxWidth - width);
    }
    double height = element.barcodeHeightDots / static_cast<double>(template_.settings.dpi) * scaleY;
    double humanText = element.humanReadable ? std::max(14.0, height * 0.22) : 0.0;
    return QRectF(topLeft, QSizeF(width, std::max(22.0, height) + humanText));
}

QPointF PreviewWidget::labelToWidget(double xInches, double yInches, const QRectF& label) const
{
    return QPointF(
        label.left() + xInches / safeLabelWidth(template_) * label.width(),
        label.top() + yInches / safeLabelHeight(template_) * label.height());
}

QPointF PreviewWidget::widgetToLabel(const QPointF& point, const QRectF& label) const
{
    if (label.width() <= 0.0 || label.height() <= 0.0)
    {
        return QPointF();
    }
    return QPointF(
        (point.x() - label.left()) / label.width() * safeLabelWidth(template_),
        (point.y() - label.top()) / label.height() * safeLabelHeight(template_));
}

bool PreviewWidget::isSelected(int index) const
{
    return selectedElements_.contains(index);
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

int PreviewWidget::resizeHandleAt(const QPointF& point, int elementIndex) const
{
    if (elementIndex < 0 || elementIndex >= static_cast<int>(template_.elements.size()))
    {
        return -1;
    }

    const QRectF box = elementRect(template_.elements[elementIndex], labelRect());
    const double size = 14.0;
    const QPointF points[] = {
        box.topLeft(), QPointF(box.center().x(), box.top()), box.topRight(),
        QPointF(box.left(), box.center().y()), QPointF(box.right(), box.center().y()),
        box.bottomLeft(), QPointF(box.center().x(), box.bottom()), box.bottomRight()
    };
    for (int i = 0; i < 8; ++i)
    {
        QRectF handle(points[i].x() - size / 2.0, points[i].y() - size / 2.0, size, size);
        if (handle.contains(point))
        {
            return i;
        }
    }
    return -1;
}

QRectF PreviewWidget::elementRectInches(const LabelElement& element, const QRectF& label) const
{
    const QRectF widgetRect = elementRect(element, label);
    const QPointF topLeft = widgetToLabel(widgetRect.topLeft(), label);
    const QPointF bottomRight = widgetToLabel(widgetRect.bottomRight(), label);
    return QRectF(topLeft, bottomRight).normalized();
}

void PreviewWidget::beginInlineTextEdit(int elementIndex)
{
    if (elementIndex < 0 || elementIndex >= static_cast<int>(template_.elements.size()))
    {
        return;
    }
    const LabelElement& element = template_.elements[elementIndex];
    if (element.type != LabelElementType::Text || element.locked)
    {
        return;
    }

    if (inlineEditor_)
    {
        commitInlineTextEdit();
    }

    editingElement_ = elementIndex;
    draggingElement_ = -1;
    resizingElement_ = -1;
    resizeHandle_ = -1;
    inlineEditor_ = new QLineEdit(this);
    inlineEditor_->setText(QString::fromStdString(element.text));
    inlineEditor_->setFrame(true);
    inlineEditor_->setAlignment(Qt::AlignVCenter | (element.alignment == TextAlignment::Center
        ? Qt::AlignHCenter
        : element.alignment == TextAlignment::Right ? Qt::AlignRight : Qt::AlignLeft));
    inlineEditor_->setStyleSheet("QLineEdit { background: white; border: 2px solid #0054be; padding: 1px 3px; }");
    inlineEditor_->installEventFilter(this);
    connect(inlineEditor_, &QLineEdit::editingFinished, this, &PreviewWidget::commitInlineTextEdit);
    updateInlineEditorGeometry();
    inlineEditor_->show();
    inlineEditor_->setFocus(Qt::MouseFocusReason);
    inlineEditor_->selectAll();
    update();
}

void PreviewWidget::commitInlineTextEdit()
{
    if (!inlineEditor_)
    {
        return;
    }

    const int index = editingElement_;
    const QString text = inlineEditor_->text();
    QLineEdit* editor = inlineEditor_;
    inlineEditor_ = nullptr;
    editingElement_ = -1;
    editor->removeEventFilter(this);
    editor->deleteLater();

    if (index >= 0 && index < static_cast<int>(template_.elements.size()))
    {
        LabelElement element = template_.elements[index];
        if (element.type == LabelElementType::Text && !element.locked && element.text != text.toStdString())
        {
            element.text = text.toStdString();
            template_.elements[index] = element;
            emit elementChanged(index, element);
        }
    }
    update();
}

void PreviewWidget::cancelInlineTextEdit()
{
    if (!inlineEditor_)
    {
        return;
    }

    QLineEdit* editor = inlineEditor_;
    inlineEditor_ = nullptr;
    editingElement_ = -1;
    editor->removeEventFilter(this);
    editor->deleteLater();
    update();
}

void PreviewWidget::updateInlineEditorGeometry()
{
    if (!inlineEditor_ || editingElement_ < 0 || editingElement_ >= static_cast<int>(template_.elements.size()))
    {
        return;
    }

    const QRectF box = elementRect(template_.elements[editingElement_], labelRect()).adjusted(6, 4, -6, -4);
    inlineEditor_->setGeometry(box.toAlignedRect());

    const double scaleY = labelRect().height() / safeLabelHeight(template_);
    const int pixelSize = std::max(12, static_cast<int>(template_.elements[editingElement_].fontHeightDots / static_cast<double>(template_.settings.dpi) * scaleY));
    QFont font("Arial");
    font.setPixelSize(pixelSize);
    font.setBold(template_.elements[editingElement_].bold);
    font.setItalic(template_.elements[editingElement_].italic);
    font.setUnderline(template_.elements[editingElement_].underline);
    inlineEditor_->setFont(font);
}

void PreviewWidget::applyResize(const QPointF& labelPoint)
{
    if (resizingElement_ < 0 || resizingElement_ >= static_cast<int>(template_.elements.size()))
    {
        return;
    }

    QRectF rect = resizeStartRectInches_;
    const QPointF delta = labelPoint - resizeStartPointInches_;
    const bool left = resizeHandle_ == 0 || resizeHandle_ == 3 || resizeHandle_ == 5;
    const bool right = resizeHandle_ == 2 || resizeHandle_ == 4 || resizeHandle_ == 7;
    const bool top = resizeHandle_ == 0 || resizeHandle_ == 1 || resizeHandle_ == 2;
    const bool bottom = resizeHandle_ == 5 || resizeHandle_ == 6 || resizeHandle_ == 7;

    if (left) rect.setLeft(rect.left() + delta.x());
    if (right) rect.setRight(rect.right() + delta.x());
    if (top) rect.setTop(rect.top() + delta.y());
    if (bottom) rect.setBottom(rect.bottom() + delta.y());
    rect = rect.normalized();

    const double minWidth = 0.08;
    const double minHeight = 0.05;
    if (rect.width() < minWidth)
    {
        if (left) rect.setLeft(rect.right() - minWidth);
        else rect.setRight(rect.left() + minWidth);
    }
    if (rect.height() < minHeight)
    {
        if (top) rect.setTop(rect.bottom() - minHeight);
        else rect.setBottom(rect.top() + minHeight);
    }

    rect.moveLeft(std::max(0.0, rect.left()));
    rect.moveTop(std::max(0.0, rect.top()));
    if (snapToGrid_)
    {
        constexpr double gridStep = 0.25;
        rect.moveLeft(std::round(rect.left() / gridStep) * gridStep);
        rect.moveTop(std::round(rect.top() / gridStep) * gridStep);
        rect.setWidth(std::max(minWidth, std::round(rect.width() / gridStep) * gridStep));
        rect.setHeight(std::max(minHeight, std::round(rect.height() / gridStep) * gridStep));
    }

    LabelElement& element = template_.elements[resizingElement_];
    element = resizeStartElement_;
    element.xInches = rect.left();
    element.yInches = rect.top();
    const int dpi = std::max(1, template_.settings.dpi);

    if (element.type == LabelElementType::Text)
    {
        element.boxWidthInches = std::max(minWidth, rect.width());
        const int lines = element.wrap || element.multiLine ? std::max(1, element.maxLines) : 1;
        element.fontHeightDots = std::max(8, static_cast<int>(std::round(rect.height() * dpi / lines)));
        element.fontWidthDots = std::max(8, static_cast<int>(std::round(element.fontHeightDots * 0.78)));
    }
    else if (element.type == LabelElementType::QrCode)
    {
        const double target = std::max(rect.width(), rect.height());
        element.qrMagnification = std::clamp(static_cast<int>(std::round(target / 0.055)), 1, 10);
    }
    else if (element.type == LabelElementType::Line || element.type == LabelElementType::Box)
    {
        element.boxWidthInches = std::max(minWidth, rect.width());
        element.fontHeightDots = std::max(2, static_cast<int>(std::round(rect.height() * dpi)));
        if (element.type == LabelElementType::Line)
        {
            element.fontWidthDots = std::clamp(static_cast<int>(std::round(rect.height() * dpi)), 1, 30);
        }
        else
        {
            element.fontWidthDots = std::clamp(element.fontWidthDots, 1, 30);
        }
    }
    else
    {
        element.boxWidthInches = std::max(minWidth, rect.width());
        element.barcodeHeightDots = std::max(10, static_cast<int>(std::round(rect.height() * dpi * 0.78)));
        VariableContext context = variables_;
        SampleData::fillMissingForElement(element, context);
        const int modules = std::max(1, BarcodeMetrics::moduleCount(element, VariableResolver::elementValue(element, context)));
        element.barcodeModuleWidth = std::clamp(static_cast<int>(std::round(rect.width() * dpi / modules)), 1, 10);
    }

    emit elementChanged(resizingElement_, element);
    update();
}

void PreviewWidget::drawGrid(QPainter& painter, const QRectF& label) const
{
    painter.save();
    painter.setPen(QPen(QColor(226, 232, 240), 1, Qt::DotLine));
    const double labelWidthInches = safeLabelWidth(template_);
    const double labelHeightInches = safeLabelHeight(template_);
    for (double x = 0.25; x < labelWidthInches; x += 0.25)
    {
        double wx = label.left() + x / labelWidthInches * label.width();
        painter.drawLine(QPointF(wx, label.top()), QPointF(wx, label.bottom()));
    }
    for (double y = 0.25; y < labelHeightInches; y += 0.25)
    {
        double wy = label.top() + y / labelHeightInches * label.height();
        painter.drawLine(QPointF(label.left(), wy), QPointF(label.right(), wy));
    }
    painter.restore();
}

void PreviewWidget::drawTextElement(QPainter& painter, const LabelElement& element, const QRectF& label, bool selected) const
{
    QRectF box = elementRect(element, label);
    VariableContext context = variables_;
    SampleData::fillMissingForElement(element, context);
    QString value = QString::fromStdString(VariableResolver::elementValue(element, context));
    double scaleY = label.height() / safeLabelHeight(template_);
    int pixelSize = std::max(12, static_cast<int>(element.fontHeightDots / static_cast<double>(template_.settings.dpi) * scaleY));

    QFont font("Arial");
    font.setPixelSize(pixelSize);
    font.setBold(element.bold);
    font.setItalic(element.italic);
    font.setUnderline(element.underline);

    painter.save();
    const int degrees = rotationDegrees(element.rotation);
    if (degrees != 0)
    {
        const QPointF center = box.center();
        painter.translate(center);
        painter.rotate(degrees);
        box.moveCenter(QPointF(0, 0));
    }
    painter.setFont(font);
    QFontMetrics metrics(font);
    box.setHeight(std::max(box.height(), static_cast<double>(metrics.height() + 10)));

    int flags = Qt::AlignVCenter;
    if (element.alignment == TextAlignment::Center) flags |= Qt::AlignHCenter;
    else if (element.alignment == TextAlignment::Right) flags |= Qt::AlignRight;
    else flags |= Qt::AlignLeft;
    if (element.wrap || element.multiLine) flags |= Qt::TextWordWrap;

    drawSelectionFrame(painter, box, selected);
    painter.setClipRect(box.adjusted(2, 2, -2, -2));
    painter.setPen(Qt::black);
    QRectF textBox = box.adjusted(6, 4, -6, -4);
    painter.drawText(textBox, flags, value.isEmpty() ? QStringLiteral("Text") : value);
    painter.restore();
}

void PreviewWidget::drawBarcodeElement(QPainter& painter, const LabelElement& element, const QRectF& label, bool selected) const
{
    QRectF box = elementRect(element, label);
    VariableContext context = variables_;
    SampleData::fillMissingForElement(element, context);
    QString value = QString::fromStdString(VariableResolver::elementValue(element, context));
    double scaleX = label.width() / safeLabelWidth(template_);
    double barHeight = element.barcodeHeightDots / static_cast<double>(template_.settings.dpi) * (label.height() / safeLabelHeight(template_));
    QRectF bars = QRectF(box.left(), box.top(), box.width(), std::min(box.height(), std::max(18.0, barHeight)));
    painter.save();
    const int degrees = rotationDegrees(element.rotation);
    if (degrees != 0)
    {
        const QPointF center = box.center();
        painter.translate(center);
        painter.rotate(degrees);
        box.moveCenter(QPointF(0, 0));
        bars = QRectF(box.left(), box.top(), box.width(), std::min(box.height(), std::max(18.0, barHeight)));
    }
    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::black);
    if (element.type == LabelElementType::Code128Barcode)
    {
        const std::vector<int> modules = code128Modules(value.toStdString());
        int totalModules = 0;
        for (int module : modules)
        {
            totalModules += module;
        }

        double x = bars.left();
        const double unit = bars.width() / std::max(1, totalModules);
        bool bar = true;
        for (int module : modules)
        {
            const double width = unit * module;
            if (bar)
            {
                painter.drawRect(QRectF(x, bars.top(), std::max(1.0, width), bars.height()));
            }
            x += width;
            bar = !bar;
        }
    }
    else
    {
        int module = std::max(2, static_cast<int>(element.barcodeModuleWidth / static_cast<double>(template_.settings.dpi) * scaleX));
        for (int x = static_cast<int>(bars.left()); x < static_cast<int>(bars.right()); x += module * 3)
        {
            painter.drawRect(QRectF(x, bars.top(), module, bars.height()));
        }
    }
    painter.setBrush(Qt::NoBrush);
    drawSelectionFrame(painter, box, selected);
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
    SampleData::fillMissingForElement(element, context);
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
    painter.setBrush(Qt::NoBrush);
    drawSelectionFrame(painter, box, selected);
    painter.restore();
}

void PreviewWidget::drawShapeElement(QPainter& painter, const LabelElement& element, const QRectF& label, bool selected) const
{
    const QRectF box = elementRect(element, label);
    painter.save();
    const double scaleY = label.height() / safeLabelHeight(template_);
    const int stroke = element.type == LabelElementType::Line
        ? std::max(1, static_cast<int>(element.fontWidthDots / static_cast<double>(template_.settings.dpi) * scaleY))
        : std::max(1, static_cast<int>(element.fontWidthDots / 2.0));
    painter.setBrush(Qt::NoBrush);
    painter.setPen(QPen(Qt::black, stroke));
    if (element.type == LabelElementType::Line)
    {
        painter.drawLine(box.left(), box.center().y(), box.right(), box.center().y());
    }
    else
    {
        painter.drawRect(box);
    }
    drawSelectionFrame(painter, box, selected);
    painter.restore();
}
