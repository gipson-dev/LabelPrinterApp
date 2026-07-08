# LabelPrinterApp Roadmap

## Project Goal

Build a Windows desktop label-design and Zebra-printing application for custom text, number, barcode, QR, line, box, and batch labels. The application supports editable templates, Excel/CSV data import, print preview, Zebra ZPL output, printer settings, print history, developer-friendly local setup, and release packaging.

The current application covers the requested Version 1 through Version 5 feature set and includes the classic desktop designer UI pass. `v1.0.8` is the current patch release line, and the app can check for and apply newer GitHub releases on its own. Clean-machine and physical-printer QA hardening from the beta phase continues in parallel; see [Known Remaining Work](#known-remaining-work) and [docs/KNOWN_ISSUES.md](KNOWN_ISSUES.md).

## Release Status Overview

| Release Area | Status | Summary |
| --- | --- | --- |
| Core label engine | Complete | Label model, ZPL generation, template storage, variable handling, and shape output are implemented. |
| Zebra printing | Complete | Windows RAW spooler printing is implemented. |
| Designer UI | Beta-ready | Classic designer layout, toolbox, canvas, rulers, grid, snap, marquee multi-select, group drag, inspector pages, inline text editing, and layout controls are functional. |
| Data import and batch printing | Beta-ready | Excel/CSV rows, checked rows, row ranges, copies, serial ranges, and placeholder mapping are implemented. |
| Packaging | Beta-ready | Release folder builds under `dist\LabelPrinterApp`; Qt runtime deployment is handled when available. |
| Self-updating releases | Complete | `Help > Check for Updates` and a silent startup check ask GitHub releases for `gipson-dev/LabelPrinterApp`, download and verify newer `LabelPrinterApp_Portable.zip` builds in the background, and hand off to `LabelPrinterAppLauncher.exe` to apply the update and relaunch, retaining `templates\` and `logs\`. |
| Developer setup | Beta-ready | CMake/Qt build path is documented and VS Code C/C++ IntelliSense resolves fetched `nlohmann/json.hpp` after configure/build. |
| Testing and calibration | In progress | Automated core tests exist; physical printer calibration and clean-machine manual QA remain. |
| Production polish | In progress | Image/logo support, installer polish, final calibration controls, and print history viewer enhancements (export, clear, reprint) remain. |

## Phase Roadmap

| Phase | Area | Status | Notes |
| --- | --- | --- | --- |
| 1 | Project foundation | Complete | Root CMake project with `core/`, `ui/`, `templates/`, `examples/`, `docs/`, scripts, tests, and VS Code workspace configuration. |
| 2 | Label data model | Complete | `PrinterSettings`, `LabelTemplate`, and `LabelElement` model label size, DPI, orientation, text, barcode, QR, line, box, formatting, variables, `locked`, and `doNotPrint`. |
| 3 | ZPL generation engine | Complete | Generates escaped ZPL for text, Code 128, Code 39, QR, Line, Box, media mode, orientation, margins, darkness, speed, wrapping, alignment, bold, underline, auto-fit, and corrected normal-text vertical origin. |
| 4 | Zebra printer communication | Complete | Sends raw ZPL through the Windows spooler with user-facing success and failure handling. |
| 5 | Template storage | Complete | JSON load/save via `nlohmann/json`, fallback template loading, blank stock templates, and saved metadata for text, barcode, QR, line, box, print, and lock flags. |
| 6 | Main UI wiring | Complete | Qt `MainWindow`, menus, dense toolbar, left toolbox, center preview, right property editor, bottom alignment toolbar, settings, help, template commands, and Data tab are wired. |
| 7 | Editable label fields | Complete | Users can add, edit, delete, duplicate, move, resize, lock, unlock, mark do-not-print, and edit text directly on the canvas. |
| 8 | Barcode and QR support | Complete | Code 128, Code 39, QR code, barcode height, module width, human-readable toggle, Zebra-width preview sizing, value-aware barcode centering, and ZPL output are implemented. |
| 9 | Preview and canvas system | Complete | Scaled preview with label boundary, printable margin, rulers, grid, snap, selection handles, cursor position, Zebra-scaled text, text/barcode/QR/shape rendering, drag-to-move, resize handles, canvas bounds clamping, and locked-element drag blocking. |
| 10 | Template manager | Complete | Template tab loads bundled JSON templates and supports saving/loading user templates. |
| 11 | Printer and stock settings | Complete | Installed-printer selector, refresh, DPI, dimensions, margins, gap, sensing mode, orientation, speed, darkness, copies, and stock presets. |
| 12 | Error handling and status feedback | Complete for foundation | Save errors, print errors, missing-printer validation, template fallback handling, status-bar messages, and basic success/failure reporting. |
| 13 | Print history | Complete for foundation | Persistent CSV print history logs successful and failed jobs to `logs\print_history.csv`, viewable in-app from `View > Print History`. Export, clear, and reprint-from-history remain as to-do items. |
| 14 | Batch printing | Complete | Copies, serial ranges, checked Excel/CSV rows, all rows, row ranges, quantity-per-row printing, placeholder mapping, and standard `Number`/`Description` fields. |
| 15 | Advanced formatting | Complete for foundation | Font size dropdown, font width, regular default text weight, optional bold/italic/underline, rotation, alignment, wrapping, multi-line, fixed/variable text, live date/time fields, prefix/suffix, and auto-fit metadata. |
| 16 | Drag-and-drop designer | Complete | Drag-to-move, drag-marquee multi-select, Ctrl-click selection toggles, group dragging, resize handles, layer order, align tools, equal spacing, snap/grid toggles, cut/copy/paste, undo/redo, zoom in/out/fit, lock/unlock, and inline text editing. |
| 17 | Release packaging | Beta-ready | `scripts/package-release.ps1` builds `dist\LabelPrinterApp`, deploys Qt runtime files through `windeployqt` when available, and includes docs/templates/examples. |
| 18 | GitHub and release workflow | Complete for foundation | GitHub repository, main branch publishing, Actions build/test workflow, and Windows package artifact workflow foundation. |
| 19 | Testing and calibration | In progress | Core tests cover ZPL, text origin behavior, CSV, variables, serial ranges, and template storage. Manual printer calibration remains next. |
| 20 | Final polish and production readiness | In progress | Classic UI polish, readable controls, functional inspector pages, layout controls, persistent app settings, vertical print/preview alignment, developer setup cleanup, and package rebuilds are ongoing. |
| 21 | Self-updating releases | Complete | Vendored `c-updater` (with its `nlohmann/json`, `googletest`, and `minizip-ng` dependencies) as tracked source, added `core/AppUpdater`, wired a startup check plus `Help > Check for Updates` in `MainWindow`, added the `LabelPrinterAppLauncher.exe` update-apply/relaunch helper, and bumped the project version to `1.0.0` for release-version comparisons. |

## Current Completed Highlights

- Classic light desktop UI with menu bar, dense toolbar, readable left toolbox, rulered canvas, printable margin, and bottom alignment toolbar.
- Working Grid and Snap toolbar toggles with optional 0.25 inch snapped element movement.
- Right-side `Element Property Editor` with filtered pages for Text, Formatting, Position, Data, Barcode, and Print.
- Functional bottom layout controls for align, equal spacing, bring forward, send backward, lock, and unlock.
- Excel/CSV Data tab with load/save, editable rows, print checkboxes, copies, search, row ranges, placeholder mapping, and standard `Number`/`Description` fields.
- Side and corner resize handles for selected text, barcode, QR, line, and box elements.
- Text, barcode, QR, line, and box support with template storage and ZPL output.
- Inline canvas text editing by double-clicking an unlocked text element.
- Multi-element selection with drag-marquee selection, Ctrl-click selection toggles, group dragging, and group-aware align/distribute/lock commands.
- New Text, Number, and Serial # elements default to regular font weight; Bold is opt-in from the Formatting page.
- Cut, copy, paste, undo, redo, zoom in, zoom out, zoom fit, and help.
- Shared preview/ZPL layout with Zebra-scaled preview text, explicit ZPL text line output, and safe shape/border insets.
- Stock presets for Uline S-8599, Uline S-22422, Zebra 2.25 x 0.75 generic, and Zebra ZD620 4 x 2.
- Bundled blank templates for both 2.25 x 0.75 and 4 x 2 label stock.
- Persistent app settings for the selected printer, stock, DPI, label setup, print options, active tab, and window layout.
- Persistent CSV print history for successful and failed print jobs.
- Release package output under `dist\LabelPrinterApp`.
- VS Code C/C++ IntelliSense include paths for fetched `nlohmann/json.hpp` in the local build folders.
- Self-updating releases: a startup check plus `Help > Check for Updates` download and verify newer GitHub releases in the background and relaunch through `LabelPrinterAppLauncher.exe` to apply them, keeping `templates\` and `logs\` intact.
- App version shown in the main window title and About dialog.
- `v1.0.8` prepared as the current preview/print WYSIWYG, canvas bounds, and package version bump release.
- In-app Print History viewer from `View > Print History`, reading `logs\print_history.csv` into a refreshable, most-recent-first table with color-coded success/failure.

## Immediate Next Order

1. Run the manual QA checklist against the latest `v1.0.x` package on a clean Windows machine, including the update-check/apply/relaunch flow end to end against a real newer release.
2. Run physical printer calibration checks on both 203 DPI and 300 DPI Zebra printers.
3. Fix issues found during clean-machine and printer testing, especially runtime files, startup path, templates, settings, print offsets, DPI scaling, font preview differences, barcode sizing, and CSV/Excel edge cases.
4. Add the remaining high-value production features: image/logo support, print history export/clear/reprint, keyboard nudging, calibration offset controls, installer polish, screenshots, and release notes.
5. Cut the next patch/point release once clean-machine and printer QA sign off.

## Recommended Version Plan

### Version 0.9.0 Beta

Goal: first usable beta package.

Must include:

- Current designer.
- Template save/load.
- Excel/CSV import.
- Batch printing.
- Zebra printing.
- Persistent settings.
- Basic print history.
- Clean-machine launch support.

### Version 0.9.1 Beta Fix

Goal: fix problems found during beta testing.

Likely includes:

- Printer calibration fixes.
- Package fixes.
- UI readability fixes.
- CSV/Excel bug fixes.
- Better error messages.

### Version 1.0.0 Release

Goal: first stable release.

Status: tagged and published as `v1.0.0`, including self-updating release delivery. Clean-machine install, physical printer calibration, and barcode/QR scan validation from the list below are still being hardened; see [Known Remaining Work](#known-remaining-work).

Patch line: `v1.0.4` adds in-app version display plus preview/print text sizing and visible-box Align middle corrections. `v1.0.5` fixes packaging for built dependency runtime DLLs such as `zlib-ng2.dll`. `v1.0.8` adds the shared preview/ZPL layout path, Zebra-scaled preview text, printer DPI sync, imported-record preview fixes, canvas bounds clamping, safe border insets, and automatic packaging version bumps.

Must include:

- Clean-machine install success.
- Manual QA passed.
- 203 DPI and 300 DPI printer validation.
- Working stock presets.
- Barcode and QR scan validation.
- User guide.
- Known issues.
- Release notes.
- Optional installer.
- Self-updating release delivery.

### Version 1.1.0

Goal: improve usability after first stable release.

Possible features:

- Image/logo support.
- Export print history to a new file.
- Clear print history from within the app.
- Reprint from history.
- Sortable/filterable columns in the Print History viewer.
- More calibration controls.
- More stock presets.

### Version 1.2.0

Goal: add advanced workflow features.

Possible features:

- Template categories.
- More barcode types.
- Better Excel field mapping.
- Import preview.
- Multi-label sheet support.
- More advanced serial numbering.
- Label validation warnings.

## Manual QA Checklist Summary

- Startup: launch from `dist\LabelPrinterApp`, move the folder, reboot, verify no developer tools are required, and confirm missing files produce understandable errors.
- Designer: add text, barcode, QR, line, and box; edit text inline; marquee-select multiple elements; group-drag and align selected elements; verify barcode center alignment against printed output; move, resize, lock, unlock, delete, duplicate, cut, copy, paste, undo, redo, zoom, grid, and snap.
- Properties: verify Text, Formatting, Position, Data, Barcode, and Print pages update the selected element and that do-not-print prevents output.
- Templates: load blank 2.25 x 0.75 and 4 x 2 templates, save custom templates, reopen saved templates, and verify element metadata survives reload.
- Data import: load CSV and Excel, edit/save rows, search, print checked rows, print all rows, print ranges, print quantity per row, and verify `Number`/`Description` placeholders.
- Printing: select Zebra printer, print one label, print copies, print checked rows, print serial ranges, and test DPI, darkness, speed, orientation, margins, and gap sensing.
- Scan validation: scan Code 128, Code 39, and QR output and confirm encoded data matches expected values.
- History: verify successful and failed jobs are logged to `logs\print_history.csv` and survive restart. Open `View > Print History` and confirm the table matches the CSV, refreshes after a new print, and color-codes success/failure.
- Developer setup: open `core/TemplateStorage.cpp` in VS Code and confirm the C/C++ extension resolves `nlohmann/json.hpp` after a normal configure/build.
- Self-update: with an older build installed, confirm the silent startup check and `Help > Check for Updates` detect a newer published GitHub release, download and apply it through `LabelPrinterAppLauncher.exe`, relaunch the app, and keep `templates\` and `logs\` intact.

## Known Remaining Work

| Priority | Item | Reason |
| --- | --- | --- |
| High | Clean-machine package test | Confirms the app works outside the development PC. |
| High | Physical printer calibration | Confirms preview and print output match real labels. |
| High | 203/300 DPI validation | Prevents sizing and offset issues across printers. |
| High | Barcode/QR scan validation | Confirms production label usability. |
| Medium | Image/logo support | Common label-design requirement. |
| Medium | Print history export/clear/reprint | The `View > Print History` viewer displays `logs\print_history.csv`; exporting, clearing, and reprinting from history are not yet implemented. |
| Medium | Calibration offset controls | Helps correct printer/media differences. |
| Medium | Installer | Improves setup for non-technical users. |
| Medium | End-to-end self-update validation | Confirm the startup check, `Help > Check for Updates`, download/verify, and launcher relaunch work against a real published GitHub release, not just local builds. |
| Low | More stock presets | Useful after core stability is verified. |
| Low | More barcode types | Useful after the main workflow is stable. |

## Definition Of Done For Version 1.0

Version 1.0 is ready when:

- The app launches on a clean Windows machine.
- The release package includes all required runtime files.
- Templates load correctly.
- Settings persist correctly.
- Excel/CSV import works.
- Batch printing works.
- Single-label printing works.
- Barcode and QR labels scan correctly.
- 203 DPI and 300 DPI printer tests pass.
- Common label stocks print with acceptable alignment.
- Print history logs successful and failed jobs.
- User-facing errors are understandable.
- Documentation explains setup, printing, calibration, and troubleshooting.
- A known-issues file exists.
- A versioned release package is created.
