# LabelPrinterApp Architecture

LabelPrinterApp is a Windows C++17 Qt 6 Widgets application for designing and printing Zebra ZPL labels. The default stock is a 2.25 x 0.75 inch direct thermal removable adhesive label for Zebra ZD410-style printers, with 203 DPI and 300 DPI support. The app sends final ZPL through the Windows RAW spooler path.

## Project Structure

```text
LabelPrinterApp/
|-- CMakeLists.txt
|-- main.cpp
|-- core/
|   |-- PrinterSettings.h
|   |-- LabelElement.h
|   |-- LabelTemplate.h
|   |-- ZplGenerator.h/.cpp
|   |-- ZebraPrinter.h/.cpp
|   |-- TemplateStorage.h/.cpp
|   |-- CsvImporter.h/.cpp
|   |-- ExcelImporter.h/.cpp
|   |-- ExcelRecordSet.h
|   |-- RecordRangeParser.h/.cpp
|   `-- VariableResolver.h/.cpp
|-- ui/
|   |-- MainWindow.h/.cpp
|   |-- PreviewWidget.h/.cpp
|   |-- ElementEditorWidget.h/.cpp
|   |-- ExcelTableModel.h/.cpp
|   `-- ExcelRecordsWidget.h/.cpp
|-- templates/
|   `-- default_label.json
|-- examples/
|   |-- sample_items.csv
|   `-- excel_records_sample.csv
|-- docs/
|   |-- ARCHITECTURE.md
|   |-- ROADMAP.md
|   |-- UI_REDESIGN.md
|   `-- example_generated.zpl
`-- tests/
    `-- model_zpl_tests.cpp
```

## Core Classes

- `PrinterSettings` stores printer name, DPI, label size, margins, gap, media sensing mode, orientation, darkness, speed, and copies.
- `LabelElement` models text, Code 128, Code 39, and QR elements. It stores content, source mode, variable name, prefix/suffix, X/Y position, text formatting, barcode settings, QR settings, `doNotPrint`, and `locked`.
- `LabelTemplate` owns printer settings and the ordered element list. Element order is used as the preview and print layer order.
- `VariableResolver` resolves `{Placeholders}`, date/time values, serial numbers, record indexes, and prefix/suffix formatting before preview/print.
- `ZplGenerator` converts templates into ZPL using `^PW`, `^LL`, `^LH`, `^MN`, `^FW`, text, Code 128, Code 39, and QR commands. Field data is escaped for unsafe ZPL characters.
- `ZebraPrinter` enumerates installed Windows printers and sends raw ZPL using `OpenPrinterA`, `StartDocPrinterA`, `WritePrinter`, and `ClosePrinter`.
- `TemplateStorage` saves and loads JSON templates with nlohmann/json, including the current lock and do-not-print element flags.
- `CsvImporter` parses quoted CSV data and detects headers.
- `ExcelImporter`, `ExcelRecordSet`, `ExcelTableModel`, and `ExcelRecordsWidget` provide the editable `.xlsx`/CSV database workflow through QXlsx.
- `RecordRangeParser` parses row ranges such as `1-*`, `1-10`, `2,4,6`, and `1-5,8`.

## UI Classes

- `MainWindow` wires menus, toolbars, tab pages, template actions, printer settings, stock presets, persistent app settings, database printing, and raw print commands.
- `PreviewWidget` paints the classic designer canvas with rulers, grid, label boundary, printable margin, text/barcode/QR previews, selection handles, cursor coordinates, and drag-to-move positioning. Locked elements cannot be dragged.
- `ElementEditorWidget` is the right-side `Element Property Editor`. Its section buttons are true filtered pages:
  - `Text`: name, type, and element text
  - `Formatting`: font size, width, bold, italic, underline, wrap, auto-fit, max lines, and alignment
  - `Position`: X, Y, box width, and rotation
  - `Data`: source, variable, prefix, and suffix
  - `Barcode`: barcode height, module width, human-readable toggle, or QR magnification
  - `Print`: do-not-print and locked flags
- `ExcelRecordsWidget` is the Data tab table for loading, editing, filtering, selecting, and printing imported records.

## Designer Controls

The Design tab uses a classic label-designer layout:

- Left readable toolbox: Select, Text, Number, Barcode, QR Code, Date/Time, Serial #, Line, Box, and Image.
- Center preview canvas with rulers, grid, printable margin, and selection handles.
- Right `Element Property Editor` with filtered property pages.
- Bottom layout toolbar:
  - Align left, center, right, top, middle, and bottom
  - Equal horizontal spacing for three or more elements
  - Bring forward and send backward for layer order
  - Lock and unlock selected elements

## Persistent App Settings

The main window uses Qt `QSettings` with the `LabelPrinterApp/LabelPrinterApp` organization/application keys. Settings are loaded after the default template and printer list are initialized, saved automatically on close, and can be saved or reset from the Preferences menu and Settings tab.

Persisted values include the window geometry/state, active tab, selected printer, DPI, stock preset, label dimensions, margins, gap, media sensing, orientation, print method, core size, speed, darkness, and copies. Template layout remains template-driven JSON so user label designs can still be saved and shared separately.

## Version Coverage

- Version 1: basic input, font size/width, X/Y position, copies, valid ZPL, and RAW printing.
- Version 2: Qt GUI, preview, multiple elements, add/edit/delete, JSON templates, and printer selection.
- Version 3: Code 128, Code 39, QR, barcode height, module width, and human-readable toggle.
- Version 4: placeholders, prompt-at-print values, date/time tokens, serial numbers, prefix/suffix, and serial ranges.
- Version 5: CSV and `.xlsx` import, header mapping, record preview, selected/all row printing, quantity per row, and placeholder replacement in text/barcode/QR fields.

## Build In Visual Studio

1. Install Visual Studio 2022 or newer with "Desktop development with C++".
2. Install Qt 6 for MSVC 64-bit.
3. Set `CMAKE_PREFIX_PATH` if Qt is not auto-detected:

```powershell
$env:CMAKE_PREFIX_PATH = "C:\Qt\6.8.3\msvc2022_64"
```

4. Build and test:

```powershell
.\scripts\build-and-test.ps1 -Config Release
```

The helper script finds Visual Studio's bundled CMake and local Qt installs under `C:\Qt\*\msvc2022_64`. The build uses a single worker to avoid MSVC/QXlsx PDB lock races.

Direct CMake commands also work:

```powershell
cmake -S . -B build -DCMAKE_PREFIX_PATH=C:\Qt\6.8.3\msvc2022_64
cmake --build build --config Release --parallel 1
ctest --test-dir build -C Release --output-on-failure
```

## Packaging

Create a distributable folder:

```powershell
.\scripts\package-release.ps1 -Config Release
```

The package is written to `dist\LabelPrinterApp`. Close any running copy of `dist\LabelPrinterApp\LabelPrinterApp.exe` before packaging so Windows can replace Qt DLLs.

## Test Printing

1. Install the Zebra Windows driver and configure the printer for ZPL mode.
2. Open Settings and select the installed printer.
3. Choose a stock preset such as `Uline S-8599 - 2.25" x 0.75" Direct Thermal`.
4. Match the DPI to the printer, usually 203 or 300.
5. Use `Print Test Label` first.
6. Load `examples\sample_items.csv` or `examples\excel_records_sample.csv` from the Data tab for variable database printing.
7. Print one label and verify alignment before printing a batch.

For 2.25 x 0.75 stock, expected dot sizes are `457 x 152` at 203 DPI and `675 x 225` at 300 DPI.
