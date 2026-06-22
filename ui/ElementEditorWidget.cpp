#include "ui/ElementEditorWidget.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QSignalBlocker>
#include <QSpinBox>
#include <vector>

ElementEditorWidget::ElementEditorWidget(QWidget* parent)
    : QWidget(parent)
{
    auto* form = new QFormLayout(this);
    nameEdit_ = new QLineEdit(this);
    typeCombo_ = new QComboBox(this);
    typeCombo_->addItems({"Text", "Code 128", "Code 39", "QR Code"});
    sourceCombo_ = new QComboBox(this);
    sourceCombo_->addItems({"Fixed", "Variable", "Prompt at Print", "Serial Number"});
    textEdit_ = new QLineEdit(this);
    variableEdit_ = new QLineEdit(this);
    prefixEdit_ = new QLineEdit(this);
    suffixEdit_ = new QLineEdit(this);
    xSpin_ = new QDoubleSpinBox(this);
    ySpin_ = new QDoubleSpinBox(this);
    boxWidthSpin_ = new QDoubleSpinBox(this);
    for (QDoubleSpinBox* spin : {xSpin_, ySpin_, boxWidthSpin_})
    {
        spin->setRange(0.0, 10.0);
        spin->setDecimals(3);
        spin->setSingleStep(0.01);
        spin->setSuffix(" in");
    }
    fontHeightSpin_ = new QSpinBox(this);
    fontWidthSpin_ = new QSpinBox(this);
    fontHeightSpin_->setRange(8, 300);
    fontWidthSpin_->setRange(8, 300);
    boldCheck_ = new QCheckBox("Bold", this);
    italicCheck_ = new QCheckBox("Italic", this);
    underlineCheck_ = new QCheckBox("Underline", this);
    wrapCheck_ = new QCheckBox("Wrap", this);
    autoFitCheck_ = new QCheckBox("Auto-fit", this);
    maxLinesSpin_ = new QSpinBox(this);
    maxLinesSpin_->setRange(1, 12);
    alignmentCombo_ = new QComboBox(this);
    alignmentCombo_->addItems({"Left", "Center", "Right"});
    rotationCombo_ = new QComboBox(this);
    rotationCombo_->addItems({"0", "90", "180", "270"});
    barcodeHeightSpin_ = new QSpinBox(this);
    barcodeHeightSpin_->setRange(10, 500);
    moduleWidthSpin_ = new QSpinBox(this);
    moduleWidthSpin_->setRange(1, 10);
    humanReadableCheck_ = new QCheckBox("Human-readable", this);
    qrMagnificationSpin_ = new QSpinBox(this);
    qrMagnificationSpin_->setRange(1, 10);

    form->addRow("Name", nameEdit_);
    form->addRow("Type", typeCombo_);
    form->addRow("Source", sourceCombo_);
    form->addRow("Text", textEdit_);
    form->addRow("Variable", variableEdit_);
    form->addRow("Prefix", prefixEdit_);
    form->addRow("Suffix", suffixEdit_);
    form->addRow("X", xSpin_);
    form->addRow("Y", ySpin_);
    form->addRow("Box Width", boxWidthSpin_);
    form->addRow("Font Height", fontHeightSpin_);
    form->addRow("Font Width", fontWidthSpin_);
    form->addRow(boldCheck_);
    form->addRow(italicCheck_);
    form->addRow(underlineCheck_);
    form->addRow(wrapCheck_);
    form->addRow(autoFitCheck_);
    form->addRow("Max Lines", maxLinesSpin_);
    form->addRow("Alignment", alignmentCombo_);
    form->addRow("Rotation", rotationCombo_);
    form->addRow("Barcode Height", barcodeHeightSpin_);
    form->addRow("Module Width", moduleWidthSpin_);
    form->addRow(humanReadableCheck_);
    form->addRow("QR Magnification", qrMagnificationSpin_);

    for (QLineEdit* edit : {nameEdit_, textEdit_, variableEdit_, prefixEdit_, suffixEdit_})
    {
        connect(edit, &QLineEdit::textChanged, this, &ElementEditorWidget::emitChanged);
    }
    for (QComboBox* combo : {typeCombo_, sourceCombo_, alignmentCombo_, rotationCombo_})
    {
        connect(combo, &QComboBox::currentIndexChanged, this, [this] { updateVisibility(); emitChanged(); });
    }
    for (QDoubleSpinBox* spin : {xSpin_, ySpin_, boxWidthSpin_})
    {
        connect(spin, &QDoubleSpinBox::valueChanged, this, &ElementEditorWidget::emitChanged);
    }
    for (QSpinBox* spin : {fontHeightSpin_, fontWidthSpin_, maxLinesSpin_, barcodeHeightSpin_, moduleWidthSpin_, qrMagnificationSpin_})
    {
        connect(spin, &QSpinBox::valueChanged, this, &ElementEditorWidget::emitChanged);
    }
    for (QCheckBox* check : {boldCheck_, italicCheck_, underlineCheck_, wrapCheck_, autoFitCheck_, humanReadableCheck_})
    {
        connect(check, &QCheckBox::toggled, this, &ElementEditorWidget::emitChanged);
    }

    updateVisibility();
}

void ElementEditorWidget::setElement(const LabelElement* element)
{
    setEnabled(element != nullptr);
    if (!element)
    {
        return;
    }

    std::vector<QSignalBlocker> blockers;
    for (QObject* object : findChildren<QObject*>())
    {
        blockers.emplace_back(object);
    }

    currentElement_ = *element;
    currentId_ = element->id.empty() ? element->name : element->id;
    nameEdit_->setText(QString::fromStdString(element->name));
    typeCombo_->setCurrentIndex(static_cast<int>(element->type));
    sourceCombo_->setCurrentIndex(static_cast<int>(element->source));
    textEdit_->setText(QString::fromStdString(element->text));
    variableEdit_->setText(QString::fromStdString(element->variableName));
    prefixEdit_->setText(QString::fromStdString(element->prefix));
    suffixEdit_->setText(QString::fromStdString(element->suffix));
    xSpin_->setValue(element->xInches);
    ySpin_->setValue(element->yInches);
    boxWidthSpin_->setValue(element->boxWidthInches);
    fontHeightSpin_->setValue(element->fontHeightDots);
    fontWidthSpin_->setValue(element->fontWidthDots);
    boldCheck_->setChecked(element->bold);
    italicCheck_->setChecked(element->italic);
    underlineCheck_->setChecked(element->underline);
    wrapCheck_->setChecked(element->wrap);
    autoFitCheck_->setChecked(element->autoFit);
    maxLinesSpin_->setValue(element->maxLines);
    alignmentCombo_->setCurrentIndex(static_cast<int>(element->alignment));
    rotationCombo_->setCurrentIndex(static_cast<int>(element->rotation));
    barcodeHeightSpin_->setValue(element->barcodeHeightDots);
    moduleWidthSpin_->setValue(element->barcodeModuleWidth);
    humanReadableCheck_->setChecked(element->humanReadable);
    qrMagnificationSpin_->setValue(element->qrMagnification);
    updateVisibility();
}

LabelElement ElementEditorWidget::element() const
{
    LabelElement e = currentElement_;
    e.name = nameEdit_->text().toStdString();
    e.id = currentId_.empty() ? e.name : currentId_;
    e.type = static_cast<LabelElementType>(typeCombo_->currentIndex());
    e.source = static_cast<FieldSource>(sourceCombo_->currentIndex());
    e.text = textEdit_->text().toStdString();
    e.variableName = variableEdit_->text().toStdString();
    e.prefix = prefixEdit_->text().toStdString();
    e.suffix = suffixEdit_->text().toStdString();
    e.xInches = xSpin_->value();
    e.yInches = ySpin_->value();
    e.boxWidthInches = boxWidthSpin_->value();
    e.fontHeightDots = fontHeightSpin_->value();
    e.fontWidthDots = fontWidthSpin_->value();
    e.bold = boldCheck_->isChecked();
    e.italic = italicCheck_->isChecked();
    e.underline = underlineCheck_->isChecked();
    e.wrap = wrapCheck_->isChecked();
    e.multiLine = wrapCheck_->isChecked();
    e.autoFit = autoFitCheck_->isChecked();
    e.maxLines = maxLinesSpin_->value();
    e.alignment = static_cast<TextAlignment>(alignmentCombo_->currentIndex());
    e.rotation = static_cast<ElementRotation>(rotationCombo_->currentIndex());
    e.barcodeHeightDots = barcodeHeightSpin_->value();
    e.barcodeModuleWidth = moduleWidthSpin_->value();
    e.humanReadable = humanReadableCheck_->isChecked();
    e.qrMagnification = qrMagnificationSpin_->value();
    return e;
}

void ElementEditorWidget::emitChanged()
{
    currentElement_ = element();
    emit elementChanged(currentElement_);
}

void ElementEditorWidget::updateVisibility()
{
    bool text = typeCombo_->currentIndex() == static_cast<int>(LabelElementType::Text);
    bool barcode = typeCombo_->currentIndex() == static_cast<int>(LabelElementType::Code128Barcode) ||
                   typeCombo_->currentIndex() == static_cast<int>(LabelElementType::Code39Barcode);
    bool qr = typeCombo_->currentIndex() == static_cast<int>(LabelElementType::QrCode);

    const std::vector<QWidget*> textWidgets = {
        fontHeightSpin_,
        fontWidthSpin_,
        boldCheck_,
        italicCheck_,
        underlineCheck_,
        wrapCheck_,
        autoFitCheck_,
        maxLinesSpin_,
        alignmentCombo_
    };
    for (QWidget* widget : textWidgets)
    {
        widget->setVisible(text);
    }

    const std::vector<QWidget*> barcodeWidgets = {
        barcodeHeightSpin_,
        moduleWidthSpin_,
        humanReadableCheck_
    };
    for (QWidget* widget : barcodeWidgets)
    {
        widget->setVisible(barcode);
    }
    qrMagnificationSpin_->setVisible(qr);
}
