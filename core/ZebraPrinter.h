#pragma once

#include <optional>
#include <string>
#include <vector>

class ZebraPrinter
{
public:
    static std::vector<std::string> installedPrinters(std::string& errorMessage);
    static std::optional<int> printerDpi(const std::string& printerName, std::string& errorMessage);
    static bool printRawZpl(const std::string& printerName, const std::string& zpl, std::string& errorMessage);
};
