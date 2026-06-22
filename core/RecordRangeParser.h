#pragma once

#include <QSet>
#include <QString>

class RecordRangeParser
{
public:
    static QSet<int> parse(const QString& text, int rowCount, QString* errorMessage = nullptr);
};
