#pragma once

#include <QWidget>

#include "core/LabelTemplate.h"
#include "core/VariableResolver.h"

class QLineEdit;
class QResizeEvent;

class PreviewWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PreviewWidget(QWidget* parent = nullptr);

    void setTemplate(const LabelTemplate& labelTemplate);
    void setVariables(const VariableContext& context);
    void setSelectedElement(int index);
    void setGridVisible(bool visible);
    void setSnapToGrid(bool enabled);
    void zoomIn();
    void zoomOut();
    void zoomFit();

signals:
    void elementSelected(int index);
    void elementMoved(int index, double xInches, double yInches);
    void elementChanged(int index, const LabelElement& element);
    void cursorPositionChanged(double xInches, double yInches);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    QRectF labelRect() const;
    QRectF elementRect(const LabelElement& element, const QRectF& label) const;
    QPointF labelToWidget(double xInches, double yInches, const QRectF& label) const;
    QPointF widgetToLabel(const QPointF& point, const QRectF& label) const;
    int hitTest(const QPointF& point) const;
    int resizeHandleAt(const QPointF& point, int elementIndex) const;
    QRectF elementRectInches(const LabelElement& element, const QRectF& label) const;
    void applyResize(const QPointF& labelPoint);
    void beginInlineTextEdit(int elementIndex);
    void commitInlineTextEdit();
    void cancelInlineTextEdit();
    void updateInlineEditorGeometry();
    void drawGrid(QPainter& painter, const QRectF& label) const;
    void drawTextElement(QPainter& painter, const LabelElement& element, const QRectF& label, bool selected) const;
    void drawBarcodeElement(QPainter& painter, const LabelElement& element, const QRectF& label, bool selected) const;
    void drawQrElement(QPainter& painter, const LabelElement& element, const QRectF& label, bool selected) const;
    void drawShapeElement(QPainter& painter, const LabelElement& element, const QRectF& label, bool selected) const;

    LabelTemplate template_;
    VariableContext variables_;
    int selectedElement_ = -1;
    int draggingElement_ = -1;
    int resizingElement_ = -1;
    int resizeHandle_ = -1;
    bool gridVisible_ = true;
    bool snapToGrid_ = false;
    double zoomFactor_ = 1.0;
    QLineEdit* inlineEditor_ = nullptr;
    int editingElement_ = -1;
    QPointF dragOffsetInches_;
    QPointF resizeStartPointInches_;
    QRectF resizeStartRectInches_;
    LabelElement resizeStartElement_;
};
