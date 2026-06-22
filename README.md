# LabelPrinterApp

LabelPrinterApp is a Windows desktop app for making and printing Zebra ZPL labels. It is built for small 2.25 x 0.75 inch direct thermal removable adhesive labels, especially on Zebra ZD410-style printers.

The app lets you design a label, preview it, fill in values, import a CSV file, and send raw ZPL directly to a Windows printer.

## What You Can Do

- Design 2.25 x 0.75 inch Zebra labels for 203 DPI or 300 DPI printers.
- Add text, Code 128 barcodes, Code 39 barcodes, and QR codes.
- Move label items by dragging them in the preview.
- Edit font size, bold, italic, underline, rotation, alignment, wrapping, margins, gap, darkness, speed, and copies.
- Use placeholders like `{ItemNumber}`, `{Lot}`, `{Date}`, `{Time}`, `{Serial}`, and `{RecordIndex}`.
- Print one label, a serial-number range, selected CSV rows, or every CSV row.
- Use a CSV `Quantity` or `Qty` column to print more than one label per row.
- Save and load label templates as JSON.

## Quick Start

1. Install the Zebra Windows printer driver.
2. Make sure the printer is set up for ZPL printing.
3. Open LabelPrinterApp.
4. Select your Zebra printer from the Printer list.
5. Confirm the label size:
   - Width: `2.25`
   - Height: `0.75`
   - DPI: `203` or `300`, matching your printer
6. Edit the label fields or drag items in the preview.
7. Print one test label before printing a batch.

## Printing From CSV

Use `examples\sample_items.csv` as a starting point:

```csv
ItemNumber,Description,Lot,Quantity
A100-001,"Bracket, left hand",LOT-2401,2
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

- A title using `{ItemNumber}`
- A description field using `{Description}`
- A Code 128 barcode using `{ItemNumber}`
- A QR code using `{ItemNumber}`, `{Lot}`, and `{Date}`

## Building From Source

Requirements:

- Windows
- Visual Studio 2022 or newer with Desktop development with C++
- CMake 3.20 or newer
- Qt 6 for MSVC 64-bit

Set `CMAKE_PREFIX_PATH` to your Qt install folder:

```powershell
$env:CMAKE_PREFIX_PATH = "C:\Qt\6.7.3\msvc2022_64"
.\scripts\build-and-test.ps1 -Config Release
```

On this development machine, the script auto-detects `C:\Qt\6.8.3\msvc2022_64`, so the normal VS Code build task can run without extra arguments.

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
