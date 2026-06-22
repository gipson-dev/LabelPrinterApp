#pragma once

#include <QAbstractTableModel>

#include "core/ExcelRecordSet.h"

class ExcelTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum Columns
    {
        RowNumberColumn = 0,
        PrintColumn,
        CopiesColumn,
        FirstDataColumn
    };

    explicit ExcelTableModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

    void setRecords(const ExcelRecordSet& records);
    ExcelRecordSet records() const;
    QStringList importedHeaders() const;
    int checkedCount() const;
    void setCheckedRows(const QSet<int>& rows);

signals:
    void selectionCountChanged();

private:
    ExcelRecordSet records_;
};
