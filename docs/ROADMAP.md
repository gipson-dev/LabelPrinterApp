# LabelPrinterApp Roadmap

This roadmap follows the twenty-phase plan for building the Zebra label printer app. Status values are intentionally simple so each pass can move one phase forward without losing the larger picture.

| Phase | Area | Status | Notes |
| --- | --- | --- | --- |
| 1 | Setup and structure | In progress | CMake app target exists; roadmap and test scaffold added. |
| 2 | Label data model | Complete for foundation | `LabelTemplate`, `LabelElement`, and `PrinterSettings` exist with field keys, editability metadata, type conversion helpers, and validation. |
| 3 | ZPL generator | Complete for foundation | Generates checked ZPL with validation, UTF-8 mode, field-data escaping, text, fake bold text, and Code 128 barcodes. |
| 4 | Zebra printer communication | Complete for foundation | WinSpool RAW printing is isolated behind `ZebraPrinter`, including printer enumeration, empty-job checks, and safer handle cleanup. |
| 5 | Template storage | Complete for foundation | Loads and saves one JSON template through a dependency-free structured parser with validation on load/save. |
| 6 | UI wiring | Complete for foundation | Win32 UI loads a template, populates printers through `ZebraPrinter`, respects read-only fields, validates before save/print, previews, saves, and prints. |
| 7 | Editable label fields | Complete for foundation | UI edits element value, X/Y position, text size, barcode height, bold, human-readable barcode text, and barcode type. |
| 8 | Barcode support | Complete for foundation | Supports Code 128 and Code 39 options, module width metadata, validation, storage, tests, preview hints, and ZPL output. |
| 9 | Better preview | Complete for foundation | Preview draws scaled label, dot grid, dimensions, element outlines, text, and barcode approximation. |
| 10 | Template manager | Complete for foundation | Adds new-template, reload-template, and save-template workflows for the default template path. |
| 11 | Printer settings | Complete for foundation | Adds UI controls for printer selection, darkness, speed, label width, label height, and copy count. |
| 12 | Error handling | Complete for foundation | Save/print paths validate first, show focused messages, and update a status line. |
| 13 | Print history | Complete for foundation | Records recent in-memory print attempts with template, printer, copy count, result, and message. |
| 14 | Batch printing | Complete for foundation | Supports repeat-count batch printing for multiple copies of the current label. |
| 15 | Advanced formatting | Complete for foundation | Adds label margins, gap/media sensing, orientation, font metadata, bold, italic, underline, rotation, alignment, wrapping, multi-line text, fixed/variable metadata, and auto-fit metadata. |
| 16 | Drag-and-drop designer | Complete for foundation | Preview supports click-and-drag movement for elements and syncs updated X/Y fields back into the editor. |
| 17 | Packaging as EXE | Complete for foundation | Adds `scripts/package-release.ps1` to build/test Release and copy the EXE, default template, README, and license into `dist\\LabelPrinterApp`. |
| 18 | GitHub and releases | Started | GitHub templates and Windows build workflow exist. Needs release workflow. |
| 19 | Testing and calibration | Started | Initial portable tests added. Needs printer calibration and UI/manual test checklist. |
| 20 | Final polish | Not started | Fit-and-finish after core workflows settle. |

## Near-Term Order

1. Continue with Phase 18 by adding a GitHub release workflow for packaged EXE artifacts.
2. Continue with Phase 19 by adding printer calibration notes and manual test checklists.
3. Continue with Phase 20 by tightening UI layout, labels, and final defaults.
