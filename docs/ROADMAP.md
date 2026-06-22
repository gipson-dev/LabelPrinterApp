# LabelPrinterApp Roadmap

This roadmap tracks the twenty-phase build plan. The current implementation covers the requested Version 1 through Version 5 feature set in the Qt 6 architecture.

| Phase | Area | Status | Notes |
| --- | --- | --- | --- |
| 1 | Setup and structure | Complete | Root CMake project with `core/`, `ui/`, `templates/`, `examples/`, and tests. |
| 2 | Label data model | Complete | `PrinterSettings`, `LabelElement`, and `LabelTemplate` model label size, DPI, elements, formatting, variables, and barcode options. |
| 3 | ZPL generator | Complete | Generates escaped ZPL for text, Code 128, Code 39, QR, media mode, orientation, margins, darkness, and speed. |
| 4 | Zebra printer communication | Complete | Windows RAW spooler printing uses `OpenPrinterA`, `StartDocPrinterA`, `WritePrinter`, and `ClosePrinter`. |
| 5 | Template storage | Complete | JSON load/save via nlohmann/json with fallback default templates. |
| 6 | UI wiring | Complete | Qt MainWindow wires printer settings, preview, template commands, element editor, CSV table, and printing. |
| 7 | Editable label fields | Complete | Add, edit, delete, position, and format label elements. |
| 8 | Barcode support | Complete | Code 128, Code 39, QR, height, module width, and human-readable toggle. |
| 9 | Better preview | Complete | Scaled preview with grid, text, barcode approximation, QR marker, selection outlines, and text alignment adjustment. |
| 10 | Template manager | Complete | Load and save JSON templates through the UI. |
| 11 | Printer settings | Complete | Printer, DPI, dimensions, margins, gap, sensing mode, orientation, speed, darkness, copies, and serial controls. |
| 12 | Error handling | Complete for foundation | User-facing save/print errors plus template fallback handling. |
| 13 | Print history | Deferred | The Version 5 rebuild prioritizes CSV and variable workflows; persistent history remains a later phase. |
| 14 | Batch printing | Complete | Copies, serial ranges, selected CSV rows, all CSV rows, and quantity-per-row printing. |
| 15 | Advanced formatting | Complete for foundation | Font, bold, italic, underline, rotation, alignment, wrapping, multiline, fixed/variable fields, prefix/suffix, and auto-fit metadata. |
| 16 | Drag-and-drop designer | Complete for foundation | Preview supports drag-to-move element positioning. Full designer handles/snapping remain future polish. |
| 17 | Packaging as EXE | Complete for foundation | Release packaging script copies app assets and runs `windeployqt` when available. |
| 18 | GitHub and releases | Complete for foundation | GitHub Actions builds/tests on main and packages release ZIP artifacts from manual runs or `v*` tags. |
| 19 | Testing and calibration | Started | Core tests cover ZPL, CSV, variables, serial ranges, and template storage. Printer calibration checklist remains next. |
| 20 | Final polish | Not started | UI fit, validation depth, persistent settings, and full designer polish. |

## Next Order

1. Continue with Phase 19 by adding printer calibration notes for 203 DPI and 300 DPI Zebra media.
2. Add manual QA checklists for template editing, CSV printing, and printer setup.
3. Continue with Phase 20 by adding persistent app settings and print history storage.
