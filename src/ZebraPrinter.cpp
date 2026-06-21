#include "LabelPrinterApp/ZebraPrinter.h"

#include <string>
#include <vector>
#include <windows.h>
#include <winspool.h>

#pragma comment(lib, "Winspool.lib")

ZebraPrinter::ZebraPrinter()
{
}

namespace
{
    std::string formatWindowsError(const std::string& message, DWORD errorCode)
    {
        return message + " Windows error code: " + std::to_string(errorCode) + ".";
    }

    std::string formatLastError(const std::string& message)
    {
        return formatWindowsError(message, GetLastError());
    }

    class PrinterHandle
    {
    public:
        ~PrinterHandle()
        {
            if (handle)
            {
                ClosePrinter(handle);
            }
        }

        HANDLE* address()
        {
            return &handle;
        }

        HANDLE get() const
        {
            return handle;
        }

    private:
        HANDLE handle = nullptr;
    };
}

std::vector<std::wstring> ZebraPrinter::enumerateInstalledPrinters(std::string& errorMessage)
{
    errorMessage.clear();

    DWORD bytesNeeded = 0;
    DWORD printerCount = 0;
    EnumPrintersW(PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS, nullptr, 2, nullptr, 0, &bytesNeeded, &printerCount);

    if (bytesNeeded == 0)
    {
        DWORD errorCode = GetLastError();
        if (errorCode != ERROR_INSUFFICIENT_BUFFER && errorCode != ERROR_SUCCESS)
        {
            errorMessage = formatWindowsError("Failed to query installed printers.", errorCode);
        }
        return {};
    }

    std::vector<BYTE> buffer(bytesNeeded);
    if (!EnumPrintersW(PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS, nullptr, 2, buffer.data(), bytesNeeded, &bytesNeeded, &printerCount))
    {
        errorMessage = formatLastError("Failed to enumerate installed printers.");
        return {};
    }

    std::vector<std::wstring> printerNames;
    auto* printers = reinterpret_cast<PRINTER_INFO_2W*>(buffer.data());
    for (DWORD i = 0; i < printerCount; ++i)
    {
        if (printers[i].pPrinterName)
        {
            printerNames.push_back(printers[i].pPrinterName);
        }
    }

    return printerNames;
}

void ZebraPrinter::setPrinterName(const std::wstring& printerName)
{
    this->printerName = printerName;
}

std::wstring ZebraPrinter::getPrinterName() const
{
    return printerName;
}

bool ZebraPrinter::printZpl(const std::string& zpl, std::string& errorMessage)
{
    if (printerName.empty())
    {
        errorMessage = "Printer name is empty.";
        return false;
    }

    if (zpl.empty())
    {
        errorMessage = "ZPL data is empty.";
        return false;
    }

    PrinterHandle printerHandle;
    if (!OpenPrinterW(
            const_cast<LPWSTR>(printerName.c_str()),
            printerHandle.address(),
            nullptr))
    {
        errorMessage = formatLastError("Failed to open printer.");
        return false;
    }

    DOC_INFO_1W docInfo;
    docInfo.pDocName = const_cast<LPWSTR>(L"ZPL Label Print Job");
    docInfo.pOutputFile = nullptr;
    docInfo.pDatatype = const_cast<LPWSTR>(L"RAW");

    DWORD jobId = StartDocPrinterW(printerHandle.get(), 1, reinterpret_cast<LPBYTE>(&docInfo));

    if (jobId == 0)
    {
        errorMessage = formatLastError("Failed to start print document.");
        return false;
    }

    if (!StartPagePrinter(printerHandle.get()))
    {
        errorMessage = formatLastError("Failed to start printer page.");
        EndDocPrinter(printerHandle.get());
        return false;
    }

    DWORD bytesWritten = 0;
    DWORD bytesToWrite = static_cast<DWORD>(zpl.size());

    BOOL writeResult = WritePrinter(
        printerHandle.get(),
        const_cast<char*>(zpl.c_str()),
        bytesToWrite,
        &bytesWritten);
    DWORD writeError = writeResult ? ERROR_SUCCESS : GetLastError();

    EndPagePrinter(printerHandle.get());
    EndDocPrinter(printerHandle.get());

    if (!writeResult || bytesWritten != bytesToWrite)
    {
        errorMessage = "Failed to write all ZPL data to printer.";
        if (writeError != ERROR_SUCCESS)
        {
            errorMessage += " Windows error code: " + std::to_string(writeError) + ".";
        }
        return false;
    }

    return true;
} 
