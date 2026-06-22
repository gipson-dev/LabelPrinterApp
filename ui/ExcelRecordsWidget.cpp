#include "ui/ExcelRecordsWidget.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QComboBox>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QSpinBox>
#include <QStyledItemDelegate>
#include <QTableView>
#include <QVBoxLayout>

#include "core/ExcelImporter.h"
#include "core/RecordRangeParser.h"
#include "ui/ExcelTableModel.h"

class CopiesDelegate : public QStyledItemDelegate
{
public:
    using QStyledItemDelegate::QStyledItemDelegate;

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem&, const QModelIndex&) const override
    {
        auto* spin = new QSpinBox(parent);
        spin->setRange(1, 999);
        return spin;
    }
};

ExcelRecordsWidget::ExcelRecordsWidget(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);

    auto* topRow = new QHBoxLayout;
    titleLabel_ = new QLabel("No database loaded", this);
    QFont titleFont = titleLabel_->font();
    titleFont.setBold(true);
    titleLabel_->setFont(titleFont);
    auto* loadButton = new QPushButton("Load...", this);
    saveButton_ = new QPushButton("Save", this);
    auto* saveAsButton = new QPushButton("Save As...", this);
    topRow->addWidget(titleLabel_, 1);
    topRow->addWidget(loadButton);
    topRow->addWidget(saveButton_);
    topRow->addWidget(saveAsButton);
    layout->addLayout(topRow);

    auto* searchRow = new QHBoxLayout;
    searchEdit_ = new QLineEdit(this);
    searchEdit_->setPlaceholderText("Search records");
    searchColumnCombo_ = new QComboBox(this);
    auto* findButton = new QPushButton("Find", this);
    searchRow->addWidget(searchEdit_, 1);
    searchRow->addWidget(searchColumnCombo_);
    searchRow->addWidget(findButton);
    layout->addLayout(searchRow);

    model_ = new ExcelTableModel(this);
    proxy_ = new QSortFilterProxyModel(this);
    proxy_->setSourceModel(model_);
    proxy_->setFilterCaseSensitivity(Qt::CaseInsensitive);
    proxy_->setFilterKeyColumn(-1);

    table_ = new QTableView(this);
    table_->setModel(proxy_);
    table_->setAlternatingRowColors(false);
    table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_->setSelectionMode(QAbstractItemView::ExtendedSelection);
    table_->setSortingEnabled(true);
    table_->setShowGrid(true);
    table_->verticalHeader()->setVisible(false);
    table_->horizontalHeader()->setStretchLastSection(true);
    table_->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    table_->setItemDelegateForColumn(ExcelTableModel::CopiesColumn, new CopiesDelegate(this));
    table_->setStyleSheet(
        "QTableView { gridline-color: #bfc7d1; selection-background-color: #cfe8ff; selection-color: #111; }"
        "QHeaderView::section { background: #eef2f6; border: 1px solid #bfc7d1; padding: 5px; font-weight: 600; }"
        "QTableView::item { padding: 4px; }");
    table_->setColumnWidth(ExcelTableModel::RowNumberColumn, 52);
    table_->setColumnWidth(ExcelTableModel::PrintColumn, 64);
    table_->setColumnWidth(ExcelTableModel::CopiesColumn, 82);
    layout->addWidget(table_, 1);

    auto* bottomRow = new QHBoxLayout;
    statusLabel_ = new QLabel("Selected records: 0/0", this);
    rangeEdit_ = new QLineEdit("1-*", this);
    rangeEdit_->setMaximumWidth(180);
    auto* applyRangeButton = new QPushButton("Apply Range", this);
    auto* printButton = new QPushButton("Print Selected Records", this);
    bottomRow->addWidget(statusLabel_, 1);
    bottomRow->addWidget(new QLabel("Range:", this));
    bottomRow->addWidget(rangeEdit_);
    bottomRow->addWidget(applyRangeButton);
    bottomRow->addWidget(printButton);
    layout->addLayout(bottomRow);

    connect(loadButton, &QPushButton::clicked, this, &ExcelRecordsWidget::loadFile);
    connect(saveButton_, &QPushButton::clicked, this, &ExcelRecordsWidget::save);
    connect(saveAsButton, &QPushButton::clicked, this, &ExcelRecordsWidget::saveAs);
    connect(findButton, &QPushButton::clicked, this, &ExcelRecordsWidget::applySearch);
    connect(searchEdit_, &QLineEdit::textChanged, this, [this](const QString& text) {
        if (text.isEmpty())
        {
            applySearch();
        }
    });
    connect(applyRangeButton, &QPushButton::clicked, this, &ExcelRecordsWidget::applyRange);
    connect(printButton, &QPushButton::clicked, this, &ExcelRecordsWidget::printSelectedRequested);
    connect(model_, &ExcelTableModel::selectionCountChanged, this, &ExcelRecordsWidget::updateStatus);
    connect(model_, &ExcelTableModel::dataChanged, this, &ExcelRecordsWidget::recordsChanged);
    connect(table_->selectionModel(), &QItemSelectionModel::currentRowChanged, this, [this](const QModelIndex& current) {
        const QModelIndex source = proxy_->mapToSource(current);
        if (source.isValid())
        {
            emit previewRowRequested(source.row());
        }
    });

    rebuildColumnChoices();
}

ExcelRecordSet ExcelRecordsWidget::records() const
{
    return model_->records();
}

QStringList ExcelRecordsWidget::headers() const
{
    return model_->importedHeaders();
}

QVector<int> ExcelRecordsWidget::printableSourceRows() const
{
    QVector<int> rows;
    QString error;
    const QSet<int> range = RecordRangeParser::parse(rangeEdit_->text(), model_->rowCount(), &error);
    const ExcelRecordSet data = model_->records();
    for (int row = 0; row < data.records.size(); ++row)
    {
        if (data.records[row].checked && range.contains(row))
        {
            rows << row;
        }
    }
    return rows;
}

int ExcelRecordsWidget::copiesForSourceRow(int sourceRow) const
{
    const ExcelRecordSet data = model_->records();
    if (sourceRow < 0 || sourceRow >= data.records.size())
    {
        return 1;
    }
    return data.records[sourceRow].copies;
}

void ExcelRecordsWidget::loadFile()
{
    QString path = QFileDialog::getOpenFileName(this, "Load Excel Database", "examples", "Excel or CSV (*.xlsx *.csv);;Excel Workbook (*.xlsx);;CSV (*.csv)");
    if (path.isEmpty())
    {
        return;
    }

    ExcelRecordSet records;
    QString error;
    if (!ExcelImporter::load(path, records, error))
    {
        QMessageBox::warning(this, "Load Failed", error);
        return;
    }

    model_->setRecords(records);
    titleLabel_->setText(records.sheetName.isEmpty() ? QFileInfo(path).fileName() : records.sheetName);
    rebuildColumnChoices();
    updateStatus();
    emit recordsChanged();
    if (model_->rowCount() > 0)
    {
        table_->selectRow(0);
        emit previewRowRequested(0);
    }
}

void ExcelRecordsWidget::save()
{
    const ExcelRecordSet data = model_->records();
    if (data.filePath.isEmpty())
    {
        saveAs();
        return;
    }

    QString error;
    if (!ExcelImporter::save(data.filePath, data, error))
    {
        QMessageBox::warning(this, "Save Failed", error);
    }
}

void ExcelRecordsWidget::saveAs()
{
    const ExcelRecordSet data = model_->records();
    QString path = QFileDialog::getSaveFileName(this, "Save Excel Database", data.filePath.isEmpty() ? "examples/records.xlsx" : data.filePath, "Excel Workbook (*.xlsx);;CSV (*.csv)");
    if (path.isEmpty())
    {
        return;
    }

    QString error;
    if (!ExcelImporter::save(path, data, error))
    {
        QMessageBox::warning(this, "Save Failed", error);
    }
}

void ExcelRecordsWidget::rebuildColumnChoices()
{
    searchColumnCombo_->clear();
    searchColumnCombo_->addItem("All columns", -1);
    const QStringList headers = model_->importedHeaders();
    for (int i = 0; i < headers.size(); ++i)
    {
        searchColumnCombo_->addItem(headers[i], ExcelTableModel::FirstDataColumn + i);
    }
}

void ExcelRecordsWidget::applySearch()
{
    proxy_->setFilterFixedString(searchEdit_->text());
    proxy_->setFilterKeyColumn(searchColumnCombo_->currentData().toInt());
}

void ExcelRecordsWidget::applyRange()
{
    QString error;
    const QSet<int> rows = RecordRangeParser::parse(rangeEdit_->text(), model_->rowCount(), &error);
    if (!error.isEmpty())
    {
        QMessageBox::warning(this, "Invalid Range", error);
        return;
    }
    model_->setCheckedRows(rows);
}

void ExcelRecordsWidget::updateStatus()
{
    statusLabel_->setText(QString("Selected records: %1/%2").arg(model_->checkedCount()).arg(model_->rowCount()));
}
