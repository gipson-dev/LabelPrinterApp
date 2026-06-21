#pragma once

#include <ctime>
#include <string>
#include <vector>

struct PrintHistoryEntry
{
    std::string templateName;
    std::wstring printerName;
    int copyCount = 1;
    bool success = false;
    std::string message;
    std::time_t printedAt = 0;
};

class PrintHistory
{
public:
    void add(const PrintHistoryEntry& entry)
    {
        entries.push_back(entry);
        if (entries.size() > MaxEntries)
        {
            entries.erase(entries.begin());
        }
    }

    const std::vector<PrintHistoryEntry>& all() const
    {
        return entries;
    }

private:
    static constexpr std::size_t MaxEntries = 50;
    std::vector<PrintHistoryEntry> entries;
};
