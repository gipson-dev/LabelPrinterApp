#include "core/RecordRangeParser.h"

#include <algorithm>

QSet<int> RecordRangeParser::parse(const QString& text, int rowCount, QString* errorMessage)
{
    QSet<int> rows;
    const QString trimmed = text.trimmed();
    if (rowCount <= 0)
    {
        return rows;
    }
    if (trimmed.isEmpty() || trimmed == "*")
    {
        for (int i = 0; i < rowCount; ++i)
        {
            rows.insert(i);
        }
        return rows;
    }

    const QStringList parts = trimmed.split(',', Qt::SkipEmptyParts);
    for (const QString& rawPart : parts)
    {
        const QString part = rawPart.trimmed();
        if (part.isEmpty())
        {
            continue;
        }

        int start = 0;
        int end = 0;
        if (part.contains('-'))
        {
            const QStringList bounds = part.split('-', Qt::KeepEmptyParts);
            if (bounds.size() != 2)
            {
                if (errorMessage) *errorMessage = "Invalid range: " + part;
                return {};
            }

            bool startOk = false;
            start = bounds[0].trimmed().toInt(&startOk);
            if (!startOk || start < 1)
            {
                if (errorMessage) *errorMessage = "Invalid range start: " + part;
                return {};
            }

            const QString endText = bounds[1].trimmed();
            if (endText == "*")
            {
                end = rowCount;
            }
            else
            {
                bool endOk = false;
                end = endText.toInt(&endOk);
                if (!endOk || end < 1)
                {
                    if (errorMessage) *errorMessage = "Invalid range end: " + part;
                    return {};
                }
            }
        }
        else
        {
            bool ok = false;
            start = part.toInt(&ok);
            end = start;
            if (!ok || start < 1)
            {
                if (errorMessage) *errorMessage = "Invalid row: " + part;
                return {};
            }
        }

        if (start > end)
        {
            std::swap(start, end);
        }
        start = std::max(1, start);
        end = std::min(rowCount, end);
        for (int row = start; row <= end; ++row)
        {
            rows.insert(row - 1);
        }
    }

    return rows;
}
