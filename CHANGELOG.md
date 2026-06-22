# Changelog

## 2026-06-21

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
