#pragma once

#include "core/ExcelRecordSet.h"

class ExcelImporter
{
public:
    static bool load(const QString& path, ExcelRecordSet& records, QString& errorMessage);
    static bool save(const QString& path, const ExcelRecordSet& records, QString& errorMessage);

private:
    static bool loadCsv(const QString& path, ExcelRecordSet& records, QString& errorMessage);
    static bool saveCsv(const QString& path, const ExcelRecordSet& records, QString& errorMessage);
};
