# Changelog

## 2026-06-21

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
