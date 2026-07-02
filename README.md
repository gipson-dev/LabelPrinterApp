# LabelPrinterApp

LabelPrinterApp is a Windows desktop app for making and printing Zebra ZPL labels. It is built for small 2.25 x 0.75 inch direct thermal removable adhesive labels, especially on Zebra ZD410-style printers.

The app lets you design a label, preview it, fill in values, import a CSV file, and send raw ZPL directly to a Windows printer.

## What You Can Do

- Design 2.25 x 0.75 inch Zebra labels for 203 DPI or 300 DPI printers.
- Add text, Code 128 barcodes, Code 39 barcodes, and QR codes.
- Add Line and Box elements for dividers and outlines.
- Move label items by dragging them in the preview.
- Resize selected label items from the side and corner anchors on the design canvas.
- Drag an empty canvas area to select multiple elements, then align, distribute, lock, or drag the group.
- Double-click text elements to edit their text directly on the canvas.
- Use Cut, Copy, Paste, Undo, Redo, Zoom In/Out/Fit, and Help from the classic toolbar/menu.
- See the current app version in the main window title and About dialog.
- Check for and self-apply app updates: `Help > Check for Updates` (and a quiet check a few seconds after startup) looks for a newer GitHub release, downloads and verifies it in the background, and offers to restart into it.
- Preview and print text with sizing and vertical centering that more closely match the selected text box.
- Show or hide the design grid and enable snap-to-grid placement from the toolbar.
- Use `Quick Print` from the Element Property Editor to print without leaving the Design tab.
- Work in focused tabs for Design, Elements, Data, Print, Templates, and Settings.
- Edit font size from a preset dropdown above 72 dots, plus bold, italic, underline, rotation, alignment, wrapping, margins, gap, darkness, speed, and copies.
- Use placeholders like `{Number}`, `{Description}`, `{Date}`, `{Time}`, `{Serial}`, and `{RecordIndex}`.
- Print one label, a serial-number range, selected CSV rows, or every CSV row.
- Use CSV fields named `Number` and `Description` for the standard imported label data.
- Log successful and failed print jobs to `logs\print_history.csv`, and review them in-app from `View > Print History`.
- Save and load label templates as JSON.
- Save and reset app settings so the selected printer, stock preset, label setup, active tab, and window layout survive restarts.

## Quick Start

1. Install the Zebra Windows printer driver.
2. Make sure the printer is set up for ZPL printing.
3. Open LabelPrinterApp.
4. Open the Settings tab and select your Zebra printer from the Default Printer list. Click `Refresh` if it was just plugged in or installed.
5. Pick a Label Stock preset, such as `Uline S-8599 - 2.25" x 0.75" Direct Thermal`. Landscape is the default orientation.
6. Click `Save Settings` if you want this printer and stock setup to load automatically next time.
7. On the Design tab, choose a blank `Canvas Template` if you want to switch between 2.25 x 0.75 and 4 x 2 stock without leaving the designer.
8. Add a `Number` field and a `Description` field from the left Tool Palette.
9. Drag elements to move them. Drag their side or corner anchors to resize them.
10. Print one test label before printing a batch.

## Basic Label Design

Use the Design tab for most work:

1. Pick the canvas size from `Canvas Template`.
2. Click `Number` to add a number field.
3. Click `Description` to add a description field.
4. Click `Date/Time` to add a field that automatically resolves to the current date and time.
5. Click `Barcode` or `QR Code` if the label should scan.
6. Select an element on the canvas.
7. Use the `Element Property Editor` on the right to edit text, formatting, position, data binding, barcode settings, and print/lock state.
8. Drag the selected element's anchors to resize it:
   - Side anchors adjust width or height.
   - Corner anchors adjust both.
   - Text height changes the font size.
   - Text width changes the text box width.
   - Barcode width/height changes barcode sizing.
   - QR size changes QR magnification.
   - Line and Box resizing changes shape width and height.

Text is vertically centered inside its selected text box in the designer preview, and generated ZPL applies matching Zebra text sizing/origin corrections for normal orientation printing.
Barcode selection bounds use the same Code 128 module-count sizing used by Zebra output. When a barcode is aligned center or right, the app recenters the actual value being printed inside the alignment lane, so imported values with different lengths stay aligned.
Double-click a text element to edit it in place; press Enter or click away to save the edit, or press Escape to cancel.
Drag across empty canvas space to marquee-select multiple elements. Ctrl-click toggles individual elements in or out of the current selection. Alignment, equal spacing, lock/unlock, and drag movement apply to the selected group.

For imported records, the normal field names are:

- `Number`
- `Description`

Built-in placeholders such as `{Date}`, `{Time}`, `{DateTime}`, `{Serial}`, and `{RecordIndex}` resolve automatically and do not require imported data or print prompts.

Without imported CSV/Excel data, fields such as `{Number}` and `{Description}` are prompted as user input when printing.

## App Settings

LabelPrinterApp automatically loads saved app settings at startup and saves them again when the app closes. You can also use `Preferences > Save App Settings` or the Settings tab's `Save Settings` button at any time.

Saved settings include:

- Window size, position, splitter layout, and active tab
- Default printer
- Label stock preset, width, height, margins, gap, media sensing, orientation, and DPI
- Designer grid visibility and snap-to-grid toggle
- Print method, core size, speed, darkness, and copies

Use `Preferences > Reset App Settings` or `Reset Defaults` in the Settings tab to clear saved app settings and return to the bundled default label setup.

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
- Editable imported columns such as `{Number}` and `{Description}`
- A range box such as `1-*`, `1-10`, `5`, `2,4,6`, or `1-5,8,10-12`

Checked rows that match the range are printed through the current label template. The `Print Selected Records` button also honors highlighted table rows, so you can print by checking the `Print` column, selecting rows in the table, or both. Imported headers become template variables, so a CSV column named `Number` can be used as `{Number}` in text, barcode, or QR fields.

CSV/Excel printing requires at least one printable label element. If the current canvas is a blank template, add a `Number`, `Description`, barcode, or QR field first, or load a template that contains fields such as `{Number}` and `{Description}`.

Use `examples\excel_records_sample.csv` as quick test data. `.xlsx` files are supported in app builds through QXlsx.

## Printing From CSV

Use `examples\sample_items.csv` as a starting point:

```csv
Number,Description
TEST-001,"Test description"
```

In the app:

1. Open the Data tab and click `Load...`.
2. Choose your CSV file.
3. Confirm the table shows `Number` and `Description`.
4. Check rows in the `Print` column or highlight rows in the table.
5. Click `Print Selected CSV`, or click `Print All CSV`.

Template text, barcode values, and QR values can all use placeholders. For example:

```text
{Number}
{Description}
```

## Default Templates

The default template is [templates/default_label.json](templates/default_label.json). It includes:

- Blank 2.25 x 0.75 inch label stock
- Landscape orientation
- 203 DPI defaults
- No prefilled text, barcode, or QR elements

The `templates/` folder includes blank starting templates for 2.25 x 0.75 inch labels and Zebra ZD620-style 4 x 2 inch labels. Open the `Templates` tab and double-click a blank template to load it, then add text, barcode, or QR elements as needed.

## Building From Source

Requirements:

- Windows
- Visual Studio 2022 or newer with Desktop development with C++
- CMake 3.20 or newer
- Qt 6 for MSVC 64-bit
- OpenSSL, for the bundled `c-updater` update checker. Windows builds auto-detect `C:\Program Files\OpenSSL-Win64` when present; otherwise pass `-DOPENSSL_ROOT_DIR=<path-to-openssl>`.
- Internet access on first configure if QXlsx, `c-updater`, or its vendored dependencies have not already been fetched

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

## VS Code IntelliSense

The app uses `nlohmann/json.hpp` in `core/TemplateStorage.cpp`. CMake fetches that header into `build\_deps\nlohmann_json-src\include` during configure/build, and `.vscode/c_cpp_properties.json` includes the common local fetch folders so the VS Code C/C++ extension can parse the same translation units as the build.

If IntelliSense still reports `cannot open source file "nlohmann/json.hpp"`:

1. Run a normal configure/build once so `build\_deps\nlohmann_json-src\include` exists.
2. Use `C/C++: Reset IntelliSense Database` from the VS Code command palette.
3. Reopen `core\TemplateStorage.cpp`.

## Packaging

Build a distributable folder:

```powershell
.\scripts\package-release.ps1 -Config Release
```

The package is written to `dist\LabelPrinterApp`, including `LabelPrinterApp.exe` and the small `LabelPrinterAppLauncher.exe` update-apply helper. If `windeployqt.exe` is available, the Qt runtime files are copied automatically. The script copies the exact OpenSSL `libssl`/`libcrypto` runtime DLLs imported by `LabelPrinterApp.exe`, plus OpenSSL 3 and 4 compatibility DLL pairs when available.

The package script also creates beta distribution artifacts:

- `dist\LabelPrinterApp_Portable.zip`
- `dist\LabelPrinterApp_Setup.exe`, when Windows `iexpress.exe` is available

The setup EXE is a simple beta installer that prompts for UAC, extracts the portable ZIP to the user's local app data folder, creates a desktop shortcut, and starts the app.

## GitHub Releases

Releases are published by pushing a `v*` tag. Beta tags, such as `v0.1.0-beta.2`, are published as prereleases by the GitHub Actions release workflow.

The release workflow uploads:

- `LabelPrinterApp_Portable.zip`
- `LabelPrinterApp_Setup.exe`

These are the same GitHub releases (`gipson-dev/LabelPrinterApp`) that the in-app updater checks. The updater reads the version from the release's `v*` tag and downloads whichever release asset is named exactly `LabelPrinterApp_Portable.zip`, so keep using that literal filename for the portable ZIP. The app window title and About dialog show the CMake project version that should match the release tag.

See [docs/RELEASE_PROCESS.md](docs/RELEASE_PROCESS.md) for the full release checklist.

## Self-Updating

`Help > Check for Updates`, plus a quiet check a few seconds after launch, asks the GitHub releases API for the latest `gipson-dev/LabelPrinterApp` release. If a newer version is found, it is downloaded and verified in the background; the app then offers to restart. Accepting hands off to `LabelPrinterAppLauncher.exe`, which applies the update while `LabelPrinterApp.exe` exits, retains the `templates\` and `logs\` folders, and relaunches the updated app. This is built on the vendored [`c-updater`](c-updater/README.md) library.

## Developer Notes

- Core label logic lives in `core/`.
- Qt UI code lives in `ui/`.
- The self-update checker (`core/AppUpdater.h/.cpp`) wraps the vendored [c-updater](c-updater/README.md) library; the update-apply helper lives in `launcher/main.cpp` and builds as `LabelPrinterAppLauncher.exe`.
- Example generated ZPL is in [docs/example_generated.zpl](docs/example_generated.zpl).
- Architecture notes are in [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md).
- Classic UI redesign notes are in [docs/UI_REDESIGN.md](docs/UI_REDESIGN.md).
- Manual QA checklist is in [docs/MANUAL_QA_CHECKLIST.md](docs/MANUAL_QA_CHECKLIST.md).
- Known beta issues are in [docs/KNOWN_ISSUES.md](docs/KNOWN_ISSUES.md).
- Printer calibration notes are in [docs/PRINTER_CALIBRATION.md](docs/PRINTER_CALIBRATION.md).
- User workflow notes are in [docs/USER_GUIDE.md](docs/USER_GUIDE.md).
- Release steps are in [docs/RELEASE_PROCESS.md](docs/RELEASE_PROCESS.md).
- The phase roadmap is in [docs/ROADMAP.md](docs/ROADMAP.md).
