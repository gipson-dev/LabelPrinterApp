# LabelPrinterApp

LabelPrinterApp is a small Windows C++ app for editing Zebra Programming Language (ZPL) label values, previewing the label, saving the template, and sending it to a Zebra printer through the WinSpool RAW print path.

The current UI loads the default JSON template, creates editable fields for label values and layout, draws a live preview, lists installed Windows printers, and prints one or more generated ZPL labels to the selected printer.

## Project Layout

```text
docs/                     Roadmap and planning notes
include/LabelPrinterApp/  Public app headers and label model types
src/                      Application entry point and printer implementation
src/ui/                   Win32 main window and preview widget
templates/                JSON template examples
tests/                    Portable tests for model, storage, and ZPL generation
.github/workflows/        GitHub Actions build checks
```

## Requirements

- Windows
- Visual Studio 2022 or newer with MSVC C++ tools
- CMake 3.20 or newer, if using the CMake build path
- Installed Zebra printer driver configured for ZPL/RAW printing

## Build

From a Developer PowerShell or Developer Command Prompt with MSVC on `PATH`:

```powershell
cl.exe /Zi /EHsc /nologo /std:c++17 /Iinclude /FeLabelPrinterApp.exe src/main.cpp src/ZebraPrinter.cpp src/ui/MainWindow.cpp src/ui/PreviewWidget.cpp Gdi32.lib User32.lib Winspool.lib /link /SUBSYSTEM:WINDOWS
```

Or use CMake:

```powershell
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

If CMake/MSVC are installed with Visual Studio but not available on `PATH`, use the repo helper:

```powershell
.\scripts\build-and-test.ps1
```

## Run

Run the app and select the installed Windows printer from the printer list:

```powershell
.\LabelPrinterApp.exe
```

## Current UI

- Printer selection from installed Windows printers
- Template actions for creating a new template, reloading the saved template, and saving edits
- Dynamic element inputs generated from `templates/default_label.json`
- Editable value, X/Y position, text size, barcode height, bold, human-readable barcode text, and barcode symbology fields
- Printer setting inputs for darkness, print speed, label width, label height, and copy count
- Live label preview with a dot grid, element outlines, text, and barcode approximation
- Validation before saving or printing
- Print button that sends generated ZPL batches to the selected printer
- Save Template button that writes edited values back to JSON
- In-memory print history with success/failure status

## Label Templates

The in-code fallback label lives in `include/LabelPrinterApp/TemplateStorage.h`.

`templates/default_label.json` is loaded at startup and saved when the Save Template button is clicked. Element names in that JSON file become the editable input labels in the UI. Element types currently supported by the generator are:

- `Text`
- `Barcode`

Each element also has a `fieldKey` for stable storage/batch-print mapping and an `editable` flag for UI behavior.
Barcode elements support `Code128` and `Code39`, `barcodeModuleWidth`, `barcodeHeight`, and `barcodeHumanReadable`.
Invalid template files fall back to the in-code default, and invalid templates are not saved.

## Notes

- Generated build artifacts such as `.exe`, `.obj`, `.pdb`, and `.ilk` files are ignored.
- Printer failures include the Windows error code where available.

## Roadmap

The phased implementation plan is tracked in [docs/ROADMAP.md](docs/ROADMAP.md).
