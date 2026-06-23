#pragma once

#include <QWidget>

#include <string>

#include "core/LabelTemplate.h"

class QString;
class QCheckBox;
class QComboBox;
class QDoubleSpinBox;
class QLineEdit;
class QSpinBox;
class QWidget;

class ElementEditorWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ElementEditorWidget(QWidget* parent = nullptr);

    void setElement(const LabelElement* element);
    LabelElement element() const;
    QWidget* sectionWidget(const QString& sectionName) const;
    void focusSection(const QString& sectionName);

signals:
    void elementChanged(const LabelElement& element);

private:
    void emitChanged();
    void updateVisibility();

    QLineEdit* nameEdit_ = nullptr;
    QComboBox* typeCombo_ = nullptr;
    QComboBox* sourceCombo_ = nullptr;
    QLineEdit* textEdit_ = nullptr;
    QLineEdit* variableEdit_ = nullptr;
    QLineEdit* prefixEdit_ = nullptr;
    QLineEdit* suffixEdit_ = nullptr;
    QDoubleSpinBox* xSpin_ = nullptr;
    QDoubleSpinBox* ySpin_ = nullptr;
    QDoubleSpinBox* boxWidthSpin_ = nullptr;
    QComboBox* fontSizeCombo_ = nullptr;
    QSpinBox* fontWidthSpin_ = nullptr;
    QWidget* formattingChecksRow_ = nullptr;
    QCheckBox* boldCheck_ = nullptr;
    QCheckBox* italicCheck_ = nullptr;
    QCheckBox* underlineCheck_ = nullptr;
    QCheckBox* wrapCheck_ = nullptr;
    QCheckBox* autoFitCheck_ = nullptr;
    QSpinBox* maxLinesSpin_ = nullptr;
    QComboBox* alignmentCombo_ = nullptr;
    QComboBox* rotationCombo_ = nullptr;
    QSpinBox* barcodeHeightSpin_ = nullptr;
    QSpinBox* moduleWidthSpin_ = nullptr;
    QCheckBox* humanReadableCheck_ = nullptr;
    QSpinBox* qrMagnificationSpin_ = nullptr;
    std::string currentId_;
    LabelElement currentElement_;
};
