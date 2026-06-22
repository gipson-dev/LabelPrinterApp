#include "core/ExcelImporter.h"

#include <QFile>
#include <QFileInfo>
#include <QTextStream>

#ifdef LABELPRINTERAPP_ENABLE_XLSX
#include "xlsxdocument.h"
#include "xlsxworksheet.h"
#endif

namespace
{
QStringList parseCsvLine(const QString& line)
{
    QStringList fields;
    QString field;
    bool quoted = false;
    for (int i = 0; i < line.size(); ++i)
    {
        const QChar c = line.at(i);
        if (quoted)
        {
            if (c == '"' && i + 1 < line.size() && line.at(i + 1) == '"')
            {
                field += '"';
                ++i;
            }
            else if (c == '"')
            {
                quoted = false;
            }
            else
            {
                field += c;
            }
        }
        else if (c == '"')
        {
            quoted = true;
        }
        else if (c == ',')
        {
            fields << field;
            field.clear();
        }
        else
        {
            field += c;
        }
    }
    fields << field;
    return fields;
}

QString csvEscape(const QString& value)
{
    if (!value.contains(',') && !value.contains('"') && !value.contains('\n'))
    {
        return value;
    }
    QString escaped = value;
    escaped.replace("\"", "\"\"");
    return "\"" + escaped + "\"";
}
}

bool ExcelImporter::load(const QString& path, ExcelRecordSet& records, QString& errorMessage)
{
    const QString suffix = QFileInfo(path).suffix().toLower();
    if (suffix == "csv")
    {
        return loadCsv(path, records, errorMessage);
    }

#ifdef LABELPRINTERAPP_ENABLE_XLSX
    QXlsx::Document document(path);
    QXlsx::Worksheet* sheet = dynamic_cast<QXlsx::Worksheet*>(document.currentWorksheet());
    if (!sheet)
    {
        errorMessage = "Could not open worksheet.";
        return false;
    }

    const auto dimension = sheet->dimension();
    if (!dimension.isValid() || dimension.rowCount() < 1)
    {
        errorMessage = "The workbook does not contain any rows.";
        return false;
    }

    records = {};
    records.filePath = path;
    records.sheetName = QFileInfo(path).completeBaseName();
    const int firstRow = dimension.firstRow();
    const int lastRow = dimension.lastRow();
    const int firstColumn = dimension.firstColumn();
    const int lastColumn = dimension.lastColumn();

    for (int column = firstColumn; column <= lastColumn; ++column)
    {
        const QString header = document.read(firstRow, column).toString().trimmed();
        records.headers << (header.isEmpty() ? QString("Column%1").arg(column - firstColumn + 1) : header);
    }

    for (int row = firstRow + 1; row <= lastRow; ++row)
    {
        ExcelRecord record;
        for (int column = firstColumn; column <= lastColumn; ++column)
        {
            record.values << document.read(row, column).toString();
        }
        records.records << record;
    }
    return true;
#else
    errorMessage = "XLSX support is not enabled in this build. Rebuild with LABELPRINTERAPP_ENABLE_XLSX=ON.";
    return false;
#endif
}

bool ExcelImporter::save(const QString& path, const ExcelRecordSet& records, QString& errorMessage)
{
    const QString suffix = QFileInfo(path).suffix().toLower();
    if (suffix == "csv")
    {
        return saveCsv(path, records, errorMessage);
    }

#ifdef LABELPRINTERAPP_ENABLE_XLSX
    QXlsx::Document document;
    for (int column = 0; column < records.headers.size(); ++column)
    {
        document.write(1, column + 1, records.headers[column]);
    }
    for (int row = 0; row < records.records.size(); ++row)
    {
        const ExcelRecord& record = records.records[row];
        for (int column = 0; column < records.headers.size(); ++column)
        {
            document.write(row + 2, column + 1, column < record.values.size() ? record.values[column] : QString());
        }
    }
    if (!document.saveAs(path))
    {
        errorMessage = "Could not save workbook.";
        return false;
    }
    return true;
#else
    errorMessage = "XLSX support is not enabled in this build. Rebuild with LABELPRINTERAPP_ENABLE_XLSX=ON.";
    return false;
#endif
}

bool ExcelImporter::loadCsv(const QString& path, ExcelRecordSet& records, QString& errorMessage)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        errorMessage = file.errorString();
        return false;
    }

    QTextStream stream(&file);
    records = {};
    records.filePath = path;
    records.sheetName = QFileInfo(path).completeBaseName();
    if (stream.atEnd())
    {
        errorMessage = "The file is empty.";
        return false;
    }

    records.headers = parseCsvLine(stream.readLine());
    while (!stream.atEnd())
    {
        ExcelRecord record;
        record.values = parseCsvLine(stream.readLine());
        while (record.values.size() < records.headers.size())
        {
            record.values << QString();
        }
        records.records << record;
    }
    return true;
}

bool ExcelImporter::saveCsv(const QString& path, const ExcelRecordSet& records, QString& errorMessage)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
    {
        errorMessage = file.errorString();
        return false;
    }

    QTextStream stream(&file);
    QStringList headers;
    for (const QString& header : records.headers)
    {
        headers << csvEscape(header);
    }
    stream << headers.join(',') << '\n';
    for (const ExcelRecord& record : records.records)
    {
        QStringList row;
        for (int column = 0; column < records.headers.size(); ++column)
        {
            row << csvEscape(column < record.values.size() ? record.values[column] : QString());
        }
        stream << row.join(',') << '\n';
    }
    return true;
}
