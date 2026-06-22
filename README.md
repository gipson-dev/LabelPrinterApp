# LabelPrinterApp

LabelPrinterApp is a Windows desktop app for making and printing Zebra ZPL labels. It is built for small 2.25 x 0.75 inch direct thermal removable adhesive labels, especially on Zebra ZD410-style printers.

The app lets you design a label, preview it, fill in values, import a CSV file, and send raw ZPL directly to a Windows printer.

## What You Can Do

- Design 2.25 x 0.75 inch Zebra labels for 203 DPI or 300 DPI printers.
- Add text, Code 128 barcodes, Code 39 barcodes, and QR codes.
- Move label items by dragging them in the preview.
- Work in focused tabs for Design, Elements, Data, Print, Templates, and Settings.
- Edit font size from a preset dropdown, plus bold, italic, underline, rotation, alignment, wrapping, margins, gap, darkness, speed, and copies.
- Use placeholders like `{ItemNumber}`, `{Lot}`, `{Date}`, `{Time}`, `{Serial}`, and `{RecordIndex}`.
- Print one label, a serial-number range, selected CSV rows, or every CSV row.
- Use a CSV `Quantity` or `Qty` column to print more than one label per row.
- Save and load label templates as JSON.

## Quick Start

1. Install the Zebra Windows printer driver.
2. Make sure the printer is set up for ZPL printing.
3. Open LabelPrinterApp.
4. Open the Settings tab and select your Zebra printer from the Default Printer list. Click `Refresh` if it was just plugged in or installed.
5. Pick a Label Stock preset, such as `Uline S-8599 - 2.25" x 0.75" Direct Thermal`.
6. Edit the label fields or drag items in the preview.
7. Print one test label before printing a batch.

## Label Stock Presets

The Settings tab includes presets for:

- Custom
- Uline S-8599 - 2.25" x 0.75" Direct Thermal
- Uline S-22422 - 2.25" x 0.75" Removable Direct Thermal
- Zebra 2.25" x 0.75" Generic
- Zebra ZD620 - 4" x 2" Direct Thermal

For 2.25" x 0.75" labels, the calculated printer size is:

- 203 DPI: `457 x 152` dots
- 300 DPI: `675 x 225` dots

For 4" x 2" labels, the calculated printer size is:

- 203 DPI: `812 x 406` dots
- 300 DPI: `1200 x 600` dots

## Excel Database Printing

The Data tab is an editable records screen for `.xlsx` and `.csv` files. It includes:

- `Load...`, `Save`, and `Save As...`
- Search by any imported column
- A row number column
- A checkbox column to choose rows for printing
- A `Copies` column with values from 1 to 999
- Editable imported columns such as `{Order id}` and `{Name}`
- A range box such as `1-*`, `1-10`, `5`, `2,4,6`, or `1-5,8,10-12`

Checked rows that match the range are printed through the current label template. Imported headers become template variables, so an Excel column named `Order id` can be used as `{Order id}` in text, barcode, or QR fields.

Use `examples\excel_records_sample.csv` as quick test data. `.xlsx` files are supported in app builds through QXlsx.

## Printing From CSV

Use `examples\sample_items.csv` as a starting point:

```csv
ItemNumber,Description,Lot,Quantity
226026-K-003,"Direct thermal removable label",LOT-K003,2
```

In the app:

1. Click `Import CSV`.
2. Choose your CSV file.
3. Click `Map CSV` if your column names do not match the placeholders in the template.
4. Select rows and click `Print Selected CSV`, or click `Print All CSV`.

Template text, barcode values, and QR values can all use placeholders. For example:

```text
ITEM {ItemNumber}
{ItemNumber}|{Lot}|{Date}
```

## Default Label

The default template is [templates/default_label.json](templates/default_label.json). It includes:

- A title with `226026-K-003`
- A description field for a removable adhesive label
- A Code 128 barcode encoding `226026-K-003`
- A QR code with the item, lot, and date

The `templates/` folder also includes ready-made samples for inventory labels, shelf/bin labels, QR asset tags, serial-number labels, price labels, mini shipping labels, and a photo-inspired work center dispatch label. Open the `Templates` tab and double-click a template to load it. The bundled barcode templates use Code 128 for inventory-style values such as `226026-K-003`.

## Building From Source

Requirements:

- Windows
- Visual Studio 2022 or newer with Desktop development with C++
- CMake 3.20 or newer
- Qt 6 for MSVC 64-bit
- Internet access on first configure if QXlsx has not already been fetched

Set `CMAKE_PREFIX_PATH` to your Qt install folder:

```powershell
$env:CMAKE_PREFIX_PATH = "C:\Qt\6.7.3\msvc2022_64"
.\scripts\build-and-test.ps1 -Config Release
```

On this development machine, the script auto-detects `C:\Qt\6.8.3\msvc2022_64`, so the normal VS Code build task can run without extra arguments.

`.xlsx` support is enabled by default with `LABELPRINTERAPP_ENABLE_XLSX=ON`. CMake fetches QXlsx automatically and links it into the Qt application.

Or run CMake directly:

```powershell
cmake -S . -B build -DCMAKE_PREFIX_PATH=C:\Qt\6.8.3\msvc2022_64
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

## Packaging

Build a distributable folder:

```powershell
.\scripts\package-release.ps1 -Config Release
```

The package is written to `dist\LabelPrinterApp`. If `windeployqt.exe` is available, the Qt runtime files are copied automatically.

## Developer Notes

- Core label logic lives in `core/`.
- Qt UI code lives in `ui/`.
- Example generated ZPL is in [docs/example_generated.zpl](docs/example_generated.zpl).
- Architecture notes are in [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md).
- The phase roadmap is in [docs/ROADMAP.md](docs/ROADMAP.md).
