#pragma once

#include <string>
#include <vector>

class ZebraPrinter
{
public:
    ZebraPrinter();

    static std::vector<std::wstring> enumerateInstalledPrinters(std::string& errorMessage);

    void setPrinterName(const std::wstring& printerName);
    std::wstring getPrinterName() const;

    bool printZpl(const std::string& zpl, std::string& errorMessage);

private:
    std::wstring printerName;
};
