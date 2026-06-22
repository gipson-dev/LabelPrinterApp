#pragma once

#include <QString>
#include <QStringList>
#include <QVector>

struct ExcelRecord
{
    bool checked = true;
    int copies = 1;
    QStringList values;
};

struct ExcelRecordSet
{
    QString filePath;
    QString sheetName;
    QStringList headers;
    QVector<ExcelRecord> records;
};
