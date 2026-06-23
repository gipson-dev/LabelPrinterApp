# Changelog

## 2026-06-22

- Added persistent app settings with load-on-startup, save-on-close, manual Save App Settings, and Reset App Settings actions for printer, stock, label setup, print options, active tab, and window layout.
- Restored native spinbox arrow rendering while keeping numeric fields tall enough for reliable increase/decrease clicks.
- Fixed spinbox stepper hit areas so the increase arrows on numeric fields are clickable.
- Updated all `docs/` files for the current architecture, classic designer UI, functional inspector pages, layout controls, packaging behavior, and current generated ZPL example.
- Renamed the right-side inspector header to `Element Property Editor`.
- Changed the right-side inspector buttons into true filtered pages for Text, Formatting, Position, Data, Barcode, and Print, including visible Do not print and Locked controls.
- Wired inspector section buttons to jump to the matching editor fields and connected layout toolbar actions for align, equal spacing, layer order, lock, and unlock.
- Reworked the text formatting checkboxes into one horizontal Style row in the properties inspector.
- Increased checkbox contrast in the properties inspector so Bold, Italic, Underline, Wrap, and Auto-fit states are easier to see.
- Improved designer usability with readable left toolbox labels, brighter selection boxes and handles, a two-row inspector selector, wrapped property rows, scrollable property editing, and resizable design sections.
- Restyled the design workspace toward a classic LabelDirect/ZebraDesigner layout with a light theme, dense menus/toolbars, compact left tool palette, rulered canvas, dashed printable boundary, selection handles, inspector tabs, bottom alignment toolbar, and richer status bar.
- Fixed database printing so checked Excel/CSV rows print their own field values instead of repeating a fixed sample item number from the active template.
- Updated bundled item/barcode templates to use `{ItemNumber}`, `{Description}`, `{Lot}`, `{Order id}`, and `{Name}` placeholders where appropriate.
- Added an Excel database-style Data tab with `.xlsx`/CSV load, editable table records, print checkboxes, per-row copies, search, range selection, save/save-as, and label-template variable mapping from column headers.
- Added modular Excel records classes: `ExcelImporter`, `ExcelTableModel`, `ExcelRecordsWidget`, and `RecordRangeParser`.
- Added QXlsx FetchContent integration for workbook import/export in the Qt app.
- Added Label Stock presets for Uline S-8599, Uline S-22422, Zebra 2.25 x 0.75 generic stock, and Zebra ZD620 4 x 2 stock.
- Added automatic calculated dot-size display for the selected width, height, and DPI.
- Reordered tabs to `Design | Elements | Data | Templates | Print | Settings` and moved stock/default-printer setup into Settings.
- Updated the example ZPL to a centered 203 DPI Uline S-8599 test label.
- Updated bundled templates and sample CSV records to use visible sample label data built around the Code 128 value `226026-K-003`.
- Improved the visual barcode preview so Code 128 bars are generated from the encoded value instead of decorative placeholder stripes.
- Changed the element font size editor to a preset dropdown.
- Added an Installed Printer selector row with a Refresh button in the print settings panel.
- Added a photo-inspired work center dispatch template with rotated text, the item number `226026-K-003`, description text, date/time fields, and a Code 128 barcode.
- Improved preview rendering for rotated text and barcode elements.

## 2026-06-21

- Added sample template labels for inventory, shelf/bin, QR asset tags, serial numbers, price labels, and mini shipping labels, and wired the Templates tab to load JSON templates from disk.
- Fixed preview visibility by scaling text, barcodes, QR codes, and sample placeholder values to the on-screen label size.
- Reworked the main window into the requested tab layout with File/Edit/View/Printer/Templates/Help menus, a main toolbar, and Design, Elements, Data, Print, Templates, and Settings tabs.
- Added tab-level actions for duplicate, move up/down, CSV record preview, ZPL preview, and test-label printing.
- Detected the local Qt install at `C:\Qt\6.8.3\msvc2022_64`, fixed Qt/MSVC compile issues, and enabled full Qt app build/test/package verification.
- Added automatic Qt prefix discovery for `C:\Qt\*\msvc2022_64` and post-build `windeployqt` deployment for local app launches.
- Added Phase 18 release automation with a GitHub Actions workflow that packages a Windows x64 ZIP, uploads it as an artifact, and publishes GitHub Releases for `v*` tags.
- Rewrote the README around user workflows: quick start, printing from CSV, default template behavior, build steps, and packaging steps.
- Ignored local Qt source checkouts with `qt/` and improved build script Qt prefix detection from `Qt6_DIR`.
- Rebuilt the application structure around the requested Qt 6 Version 5 architecture with root-level `core/`, `ui/`, `main.cpp`, `templates/`, and `examples/`.
- Added CSV import, header detection, placeholder mapping, CSV record preview, selected/all row printing, and quantity-per-row handling.
- Added prompt-at-print placeholder resolution, date/time placeholders, auto-increment serial contexts, prefix/suffix formatting, and serial range printing.
- Added Code 128, Code 39, and QR ZPL generation in the new core generator.
- Added Qt Widgets UI wiring for template load/save, printer selection, element add/edit/delete, formatting controls, preview, and drag-to-move positioning.
- Replaced the obsolete Win32 prototype source tree with the Qt 6 application layout to avoid stale include errors.
- Added `docs/ARCHITECTURE.md`, `docs/example_generated.zpl`, and `examples/sample_items.csv`.
- Updated CI to install Qt, build the application, and run CTest.
- Added Phase 15 advanced formatting foundations: margins, media gap, gap/black-mark/continuous sensing, orientation, font name, bold, italic, underline, rotation, alignment, wrapping, multi-line text, variable/fixed text metadata, and auto-fit metadata.
- Added Phase 16 drag-and-drop preview movement for label elements and fixed preview text vertical positioning.
- Added Phase 17 packaging support with `scripts/package-release.ps1` for a release EXE bundle under `dist\LabelPrinterApp`.
- Added editable label controls for element values, X/Y position, text size, barcode height, bold text, barcode human-readable text, and barcode symbology.
- Expanded barcode support with Code 128 and Code 39 options, module width metadata, template storage, validation, ZPL output, and tests.
- Improved the preview with a dot grid, label dimensions, element bounds, barcode type labels, and barcode module-width approximation.
- Added basic template manager actions for creating a new template, reloading the saved template, and saving changes.
- Added printer setting controls for darkness, print speed, label width, label height, and copy count.
- Added centralized status updates and validation messages before save/print operations.
- Added in-memory print history for successful and failed print attempts.
- Added repeat-count batch printing for multiple copies of the current label.
- Added `scripts/build-and-test.ps1` and updated the VS Code build task to find Visual Studio's bundled CMake when CMake is not on `PATH`.

## 2026-06-19

- Added typed label elements for text and barcode output.
- Updated ZPL generation to emit barcode fields.
- Reworked the default label template to use the current C++ model.
- Removed implementation-file includes from `main.cpp`.
- Added a CMake build target and updated the VS Code build task.
- Moved source files into a cleaner `include/` and `src/` layout.
- Added GitHub Actions CI for Windows builds.
- Updated README content with layout, build, run, and template notes.
- Expanded `.gitignore` for MSVC and build output files.
- Improved printer error messages with Windows error codes.

## 2026-06-19 - Phase 5 UI

- Added a real Win32 `MainWindow`.
- Added printer selection from installed Windows printers.
- Added dynamic input fields generated from JSON element names.
- Added a `PreviewWidget` that draws text and barcode elements.
- Added Print and Save Template button behavior.
- Changed the application entry point from console output to the Windows UI.
