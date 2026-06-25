#pragma once

#include <QWidget>

#include "core/ExcelRecordSet.h"

class ExcelTableModel;
class QLabel;
class QComboBox;
class QLineEdit;
class QPushButton;
class QSortFilterProxyModel;
class QTableView;

class ExcelRecordsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ExcelRecordsWidget(QWidget* parent = nullptr);

    ExcelRecordSet records() const;
    QStringList headers() const;
    QVector<int> printableSourceRows() const;
    QVector<int> selectedSourceRows() const;
    int copiesForSourceRow(int sourceRow) const;

signals:
    void recordsChanged();
    void previewRowRequested(int sourceRow);
    void printSelectedRequested();

public slots:
    void loadFile();
    void save();
    void saveAs();

private:
    void rebuildColumnChoices();
    void applySearch();
    void applyRange();
    void updateStatus();

    QLabel* titleLabel_ = nullptr;
    QLineEdit* searchEdit_ = nullptr;
    QComboBox* searchColumnCombo_ = nullptr;
    QTableView* table_ = nullptr;
    QLineEdit* rangeEdit_ = nullptr;
    QLabel* statusLabel_ = nullptr;
    QPushButton* saveButton_ = nullptr;
    ExcelTableModel* model_ = nullptr;
    QSortFilterProxyModel* proxy_ = nullptr;
};
