# Changelog

## 2026-07-02

### 1.0.4

- Bumped the project version to 1.0.4 for the preview/print and canvas alignment fix release.
- Added the current app version to the main window title and About dialog, sourced from the generated CMake project version used by the updater.
- Adjusted generated ZPL text sizing so printed text more closely matches the designer preview's visible font size.
- Fixed single-element Align middle behavior so text elements align using the visible canvas selection box height instead of only the raw font height.

### 1.0.3

- Bumped the project version to 1.0.3 for the OpenSSL runtime packaging fix release.
- Fixed release packaging so the OpenSSL `libssl`/`libcrypto` runtime DLLs used by the self-update checker are copied into the portable ZIP and beta setup package beside `LabelPrinterApp.exe`, matching the exact DLL names imported by the built app.
- Included OpenSSL 3 and OpenSSL 4 runtime DLL pairs in release packages when available, so older beta binaries and current builds can both find the runtime names they expect.
- Embedded a `requireAdministrator` manifest in the generated beta setup EXE so Windows shows a UAC prompt before installation.

### 1.0.2

- Bumped the project version to 1.0.2 for the next patch release.

## 2026-07-01

- Bumped the project version to 1.0.1 to match the `v1.0.1` release tag.
- Added an in-app Print History viewer at `View > Print History`, reading `logs\print_history.csv` back through the existing `CsvImporter` into a refreshable, most-recent-first table with color-coded success/failure rows.
- Moved the `View` menu (Preview ZPL, Zoom In/Out, Fit to Label) to sit immediately to the left of `Help` in the menu bar, and added the new `Print History` action to it.
- Wired in `c-updater` for self-updating: `Help > Check for Updates` (plus a silent startup check) now checks GitHub releases for `gipson-dev/LabelPrinterApp`, downloads and verifies newer `LabelPrinterApp_Portable.zip` releases in the background, and hands off to a new `LabelPrinterAppLauncher.exe` to apply the update and relaunch on restart, retaining `templates/` and `logs/`.
- Vendored `c-updater` (and its `nlohmann/json`, `googletest`, `minizip-ng` dependencies) as plain tracked source rather than git submodules, so CI and fresh clones build it without extra setup.
- Bumped the project version to 1.0.0 to match the `v1.0.0` release tag and give the new update checker a comparable version number.

## 2026-06-26

- Centered text vertically in the designer preview and adjusted generated ZPL text origins so printed Zebra labels match the selected text box more closely.
- Matched barcode preview/selection sizing to Zebra Code 128 module counts so center alignment and printed barcode position agree with the canvas.
- Fixed barcode centering for real print values by aligning Code 128 output inside the label-width lane at ZPL generation time and forcing the same Code 128 Code B encoding used by the preview.
- Fixed the Date/Time tool so new date/time fields use the live `{DateTime}` built-in and preview/print current date and time automatically.
- Added `Help > Check for Updates`, which opens the LabelPrinterApp GitHub releases page.
- Reused the same sample placeholder values for designer preview, ZPL preview, and Quick Print when no imported row is active.
- Added in-place canvas editing for text elements: double-click a text field, edit inside the selected element, press Enter or click away to commit, or press Escape to cancel.
- Changed new Text, Number, and Serial # elements to use regular font weight by default instead of auto-enabling Bold.
- Added multi-element canvas selection with drag-marquee selection, Ctrl-click add/remove selection, group dragging, and group-aware align/distribute/lock actions.
- Fixed VS Code C/C++ IntelliSense for `core/TemplateStorage.cpp` by adding fetched `nlohmann/json.hpp` include paths to `.vscode/c_cpp_properties.json`.
- Reworked the roadmap from the project planning notes with release status, phase detail, version plan, manual QA summary, and remaining work.
- Fixed 4 x 2 landscape printing so generated ZPL keeps fields unrotated while respecting the designer canvas orientation.
- Wired classic toolbar/menu controls for Cut, Copy, Paste, Undo, Redo, Zoom In/Out/Fit, and Help.
- Added Line and Box elements, including template storage, inspector support, preview rendering, resize behavior, and ZPL `^GB` output.
- Updated README, roadmap, user guide, architecture notes, UI notes, release notes, calibration notes, and manual QA coverage for the current beta workflow.

## 2026-06-22

- Added selected-element resize handles so side and corner anchors can adjust text width/font height, barcode width/height, and QR size directly on the design canvas.
- Expanded text font size choices above 72 dots and auto-expands text width/font width when font size increases.
- Standardized CSV import/export samples and new label fields around two columns: `Number` and `Description`; added dedicated Number and Description element buttons that bind to CSV data or prompt for user input when no CSV row is active.
- Improved imported CSV printing diagnostics: blank templates now stop with a clear warning instead of sending invisible ZPL, and `{Column Name}` placeholders with spaces now resolve from imported CSV/Excel headers.
- Fixed the Data tab `Print Selected Records` button so imported Excel/CSV rows print when they are checked in the Print column or highlighted in the table.
- Added a bottom-right `Quick Print` button to the Element Property Editor.
- Added a Design-tab `Canvas Template` selector so blank canvas sizes can be loaded directly from the designer.
- Added visible `Tool Palette` and `Layout Tools` section labels to clarify the left design tools and bottom layout toolbar.
- Changed bundled templates to blank starting templates for 2.25 x 0.75 and 4 x 2 label stock.
- Added `docs/MANUAL_QA_CHECKLIST.md` and `docs/PRINTER_CALIBRATION.md`.
- Added persistent CSV print history at `logs\print_history.csv` for successful and failed print jobs.
- Updated packaging to create `LabelPrinterApp_Portable.zip` and a beta `LabelPrinterApp_Setup.exe` when `iexpress.exe` is available.
- Made Landscape the default label orientation for stock/templates and added working Grid and Snap toolbar toggles, including app-settings persistence and 0.25 inch snap-to-grid movement.
- Regenerated bundled templates into two clear test-template sets for 2.25 x 0.75 and 4 x 2 label stock, using `TEST LABEL`, `Test description`, and `TEST-001` style sample values.
- Updated sample CSV files to use matching test label data.
- Restored reliable increase-arrow clicks on numeric fields by leaving spinbox button rendering to the native Qt/Windows control and applying sizing in code instead of the global stylesheet.
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
