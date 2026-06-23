# LabelPrinterApp Roadmap

This roadmap tracks the twenty-phase build plan. The current application covers the requested Version 1 through Version 5 feature set and includes the newer classic designer UI pass.

| Phase | Area | Status | Notes |
| --- | --- | --- | --- |
| 1 | Setup and structure | Complete | Root CMake project with `core/`, `ui/`, `templates/`, `examples/`, `docs/`, scripts, and tests. |
| 2 | Label data model | Complete | `PrinterSettings`, `LabelElement`, and `LabelTemplate` model label size, DPI, elements, formatting, variables, barcode options, `doNotPrint`, and `locked`. |
| 3 | ZPL generator | Complete | Generates escaped ZPL for text, Code 128, Code 39, QR, media mode, orientation, margins, darkness, speed, wrapping, alignment, bold, underline, and auto-fit. |
| 4 | Zebra printer communication | Complete | Windows RAW spooler printing uses `OpenPrinterA`, `StartDocPrinterA`, `WritePrinter`, and `ClosePrinter`. |
| 5 | Template storage | Complete | JSON load/save via nlohmann/json with fallback default templates and saved print/lock flags. |
| 6 | UI wiring | Complete | Qt MainWindow wires menus, toolbars, preview, property editor pages, template commands, Excel/CSV table, settings, and printing. |
| 7 | Editable label fields | Complete | Add, edit, delete, duplicate, position, format, lock, and do-not-print label elements. |
| 8 | Barcode support | Complete | Code 128, Code 39, QR, height, module width, human-readable toggle, and preview rendering based on encoded values. |
| 9 | Better preview | Complete | Scaled preview with rulers, grid, label boundary, printable margin, selection handles, cursor position, text/barcode/QR rendering, and locked-element drag blocking. |
| 10 | Template manager | Complete | Template tab loads bundled JSON templates and supports saving changes. |
| 11 | Printer settings | Complete | Installed-printer selector, refresh button, DPI, dimensions, margins, gap, sensing mode, orientation, speed, darkness, copies, and stock presets. |
| 12 | Error handling | Complete for foundation | User-facing save/print errors, template fallback handling, printer-required checks, and status-bar messages. |
| 13 | Print history | Deferred | In-memory history existed in an earlier phase; persistent history remains a later polish item. |
| 14 | Batch printing | Complete | Copies, serial ranges, checked Excel/CSV rows, all rows, row ranges, and quantity-per-row printing. |
| 15 | Advanced formatting | Complete for foundation | Font size dropdown, font width, bold, italic, underline, rotation, alignment, wrapping, multi-line, fixed/variable text, prefix/suffix, and auto-fit metadata. |
| 16 | Drag-and-drop designer | Complete for foundation | Preview supports drag-to-move, selection handles, layer order, align buttons, equal spacing, and lock/unlock. Full resize handles and snap toggles remain future polish. |
| 17 | Packaging as EXE | Complete | `scripts/package-release.ps1` builds and deploys `dist\LabelPrinterApp` with Qt runtime files through `windeployqt` when available. |
| 18 | GitHub and releases | Complete for foundation | GitHub Actions builds/tests and packages Windows artifacts for release workflows. Main branch is kept updated. |
| 19 | Testing and calibration | Started | Core tests cover ZPL, CSV, variables, serial ranges, and template storage. Manual printer calibration notes remain next. |
| 20 | Final polish | In progress | Classic UI polish, readable controls, functional inspector pages, layout controls, persistent app settings, and package rebuilds are ongoing. |

## Current Completed Highlights

- Classic light desktop UI with menu bar, dense toolbar, readable left toolbox, rulered canvas, printable margin, and bottom alignment toolbar.
- Right-side `Element Property Editor` with true filtered pages: Text, Formatting, Position, Data, Barcode, and Print.
- Functional bottom layout controls: align, equal spacing, bring forward, send backward, lock, and unlock.
- Excel/CSV Data tab with load/save, editable rows, print checkboxes, copies, search, row ranges, and placeholder mapping.
- Stock presets for Uline S-8599, Uline S-22422, Zebra 2.25 x 0.75 generic, and Zebra ZD620 4 x 2.
- Persistent app settings for the selected printer, stock, DPI, label setup, print options, active tab, and window layout.
- Release package output under `dist\LabelPrinterApp`.

## Next Order

1. Add printer calibration notes and test labels for 203 DPI and 300 DPI media.
2. Add manual QA checklists for property-page editing, layout toolbar actions, CSV printing, and printer setup.
3. Add persistent print history storage.
4. Continue designer polish with resize handles, optional snap-to-grid behavior, and image/logo support.
5. Add deeper manual printer calibration workflow for multiple Zebra models.
