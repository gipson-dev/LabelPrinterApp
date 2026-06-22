#include "ui/ExcelTableModel.h"

ExcelTableModel::ExcelTableModel(QObject* parent)
    : QAbstractTableModel(parent)
{
}

int ExcelTableModel::rowCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : records_.records.size();
}

int ExcelTableModel::columnCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : FirstDataColumn + records_.headers.size();
}

QVariant ExcelTableModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= records_.records.size())
    {
        return {};
    }

    const ExcelRecord& record = records_.records[index.row()];
    if (index.column() == RowNumberColumn)
    {
        return role == Qt::DisplayRole ? QVariant(index.row() + 1) : QVariant();
    }
    if (index.column() == PrintColumn)
    {
        return role == Qt::CheckStateRole ? QVariant(record.checked ? Qt::Checked : Qt::Unchecked) : QVariant();
    }
    if (index.column() == CopiesColumn)
    {
        return role == Qt::DisplayRole || role == Qt::EditRole ? QVariant(record.copies) : QVariant();
    }

    const int dataColumn = index.column() - FirstDataColumn;
    if ((role == Qt::DisplayRole || role == Qt::EditRole) && dataColumn >= 0 && dataColumn < record.values.size())
    {
        return record.values[dataColumn];
    }
    return {};
}

QVariant ExcelTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
    {
        return {};
    }
    if (section == RowNumberColumn) return "#";
    if (section == PrintColumn) return "Print";
    if (section == CopiesColumn) return "Copies";
    const int dataColumn = section - FirstDataColumn;
    return dataColumn >= 0 && dataColumn < records_.headers.size() ? records_.headers[dataColumn] : QVariant();
}

Qt::ItemFlags ExcelTableModel::flags(const QModelIndex& index) const
{
    Qt::ItemFlags itemFlags = QAbstractTableModel::flags(index);
    if (!index.isValid())
    {
        return itemFlags;
    }
    if (index.column() == PrintColumn)
    {
        return itemFlags | Qt::ItemIsUserCheckable | Qt::ItemIsEditable;
    }
    if (index.column() >= CopiesColumn)
    {
        return itemFlags | Qt::ItemIsEditable;
    }
    return itemFlags;
}

bool ExcelTableModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || index.row() < 0 || index.row() >= records_.records.size())
    {
        return false;
    }

    ExcelRecord& record = records_.records[index.row()];
    if (index.column() == PrintColumn && role == Qt::CheckStateRole)
    {
        record.checked = value.toInt() == Qt::Checked;
        emit dataChanged(index, index, {Qt::CheckStateRole});
        emit selectionCountChanged();
        return true;
    }
    if (index.column() == CopiesColumn && role == Qt::EditRole)
    {
        record.copies = std::max(1, std::min(999, value.toInt()));
        emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});
        return true;
    }
    if (index.column() >= FirstDataColumn && role == Qt::EditRole)
    {
        const int dataColumn = index.column() - FirstDataColumn;
        while (record.values.size() <= dataColumn)
        {
            record.values << QString();
        }
        record.values[dataColumn] = value.toString();
        emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});
        return true;
    }
    return false;
}

void ExcelTableModel::setRecords(const ExcelRecordSet& records)
{
    beginResetModel();
    records_ = records;
    for (ExcelRecord& record : records_.records)
    {
        while (record.values.size() < records_.headers.size())
        {
            record.values << QString();
        }
    }
    endResetModel();
    emit selectionCountChanged();
}

ExcelRecordSet ExcelTableModel::records() const
{
    return records_;
}

QStringList ExcelTableModel::importedHeaders() const
{
    return records_.headers;
}

int ExcelTableModel::checkedCount() const
{
    int count = 0;
    for (const ExcelRecord& record : records_.records)
    {
        if (record.checked)
        {
            ++count;
        }
    }
    return count;
}

void ExcelTableModel::setCheckedRows(const QSet<int>& rows)
{
    for (int i = 0; i < records_.records.size(); ++i)
    {
        records_.records[i].checked = rows.contains(i);
    }
    if (!records_.records.empty())
    {
        emit dataChanged(index(0, PrintColumn), index(records_.records.size() - 1, PrintColumn), {Qt::CheckStateRole});
    }
    emit selectionCountChanged();
}
