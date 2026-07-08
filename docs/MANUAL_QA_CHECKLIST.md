# Manual QA Checklist

Use this checklist before tagging or publishing a beta/release build.

## Startup And Settings

- [ ] Open `LabelPrinterApp.exe` from the packaged `dist\LabelPrinterApp` folder.
- [ ] Confirm the main window opens without missing DLL errors.
- [ ] Open the Settings tab.
- [ ] Select an installed Zebra printer from Default Printer.
- [ ] Click `Refresh` and confirm the printer list reloads.
- [ ] Select `Uline S-8599 - 2.25" x 0.75" Direct Thermal`.
- [ ] Confirm calculated size shows `457 x 152` dots at 203 DPI.
- [ ] Switch DPI to 300.
- [ ] Confirm calculated size shows `675 x 225` dots.
- [ ] Switch DPI back to 203.
- [ ] Confirm Orientation defaults to Landscape.

## Developer Workspace

- [ ] Run the normal configure/build once so CMake fetches `nlohmann/json`.
- [ ] Open `core\TemplateStorage.cpp` in VS Code.
- [ ] Confirm the C/C++ extension no longer reports `cannot open source file "nlohmann/json.hpp"`.
- [ ] If the squiggle remains, run `C/C++: Reset IntelliSense Database` and reopen the file.

## Template Workflow

- [ ] Open the Templates tab.
- [ ] Load a 2.25 x 0.75 blank template.
- [ ] Confirm the preview updates.
- [ ] Load a 4 x 2 blank template.
- [ ] Confirm the label size and preview update.
- [ ] Return to the Design tab and load each blank canvas from the `Canvas Template` selector.
- [ ] Confirm the canvas switches without using the Templates tab.
- [ ] Save the current template.
- [ ] Reload the saved template.
- [ ] Confirm element text, barcode, QR, orientation, and size are retained.

## Designer Workflow

- [ ] Add a text element.
- [ ] Edit the text to `QA TEXT`.
- [ ] Double-click the text element, edit it in place on the canvas, press Enter, and confirm the Element Property Editor updates.
- [ ] Double-click the text element again, type a temporary value, press Escape, and confirm the previous text remains.
- [ ] Select the text element and drag side/corner anchors to resize it.
- [ ] Confirm the preview text size and placement closely match the printed Zebra text for the same box.
- [ ] Increase the text font size above 72 dots and confirm the text box width grows when needed.
- [ ] Drag the text element against each canvas edge and confirm it cannot be dragged outside the label.
- [ ] Resize the text element against each canvas edge and confirm it remains inside the label.
- [ ] Add a Date/Time field and confirm the preview shows the current date and time instead of placeholder text.
- [ ] Print the Date/Time field and confirm it updates to the current date and time without prompting.
- [ ] Add a Code 128 barcode.
- [ ] Set the barcode value to `QA-001`.
- [ ] Select the barcode and drag side/corner anchors to change its width/height.
- [ ] Use Align center on the barcode, print it, and confirm the printed barcode center matches the preview center.
- [ ] Change the barcode data from `QA-001` to a longer numeric value such as `11111122222`, print again, and confirm it remains centered.
- [ ] Add a QR code.
- [ ] Set the QR value to `QA QR TEST`.
- [ ] Select the QR code and drag side/corner anchors to resize it.
- [ ] Add a Line and confirm it appears in preview/ZPL.
- [ ] Add a Box and confirm it appears in preview/ZPL.
- [ ] Resize a Box to the full label edge, print one label, and confirm the border is inset enough that the top and right edges are not cut off.
- [ ] Use Copy, Paste, Cut, Undo, and Redo from the toolbar/menu.
- [ ] Use Zoom In, Zoom Out, and Fit.
- [ ] Click Help and confirm the user guide opens or a fallback message appears.
- [ ] Click `Help > Check for Updates` and confirm it reports up to date, reports a downloaded update, reports a check failure, or falls back to opening the releases page, as appropriate. See the `Self-Update` section below for the full flow.
- [ ] Move elements by dragging in the preview.
- [ ] Drag a marquee around multiple elements and confirm each selected element shows a selection frame.
- [ ] Ctrl-click an element and confirm it toggles in or out of the multi-selection.
- [ ] Drag the selected group and confirm all unlocked selected elements move together.
- [ ] Use Align left, Align center, Align right, Align top, Align middle, and Align bottom on a multi-selection.
- [ ] Use Equal spacing on three or more selected elements.
- [ ] Toggle `Grid` off and confirm the grid hides.
- [ ] Toggle `Grid` on and confirm the grid shows.
- [ ] Toggle `Snap` on and confirm dragged elements snap to the 0.25 inch grid.
- [ ] Lock an element.
- [ ] Confirm the locked element can be selected but not dragged.
- [ ] Unlock the element.
- [ ] Confirm the element can be dragged again.
- [ ] Enable Do not print for an element.
- [ ] Preview ZPL and confirm that element is excluded.
- [ ] Click `Quick Print` in the Element Property Editor.
- [ ] Confirm it sends the current label through the same print path as the Print tab.

## Data And Printing

- [ ] Open the Data tab.
- [ ] Import `examples\sample_items.csv`.
- [ ] Confirm rows and columns load.
- [ ] Check one row in the `Print` column.
- [ ] Click `Print Selected CSV`.
- [ ] Confirm only the checked row prints.
- [ ] Uncheck that row, highlight one table row, and click `Print Selected CSV`.
- [ ] Confirm the highlighted imported row prints.
- [ ] Click `Print All CSV`.
- [ ] Confirm all rows print.
- [ ] Confirm `Number` and `Description` fields from the imported CSV appear in the label preview/ZPL.
- [ ] Click `Preview ZPL`.
- [ ] Confirm ZPL opens and uses the selected template and entered QA values.
- [ ] Click `Print Test Label`.
- [ ] Confirm one test label prints.
- [ ] Confirm printed text placement and size are close to the canvas preview and not pinned to the top or bottom of the selected text box.
- [ ] Confirm `logs\print_history.csv` is created or updated.
- [ ] Confirm print history includes success/failure, printer, template, mode, rows, and copies.
- [ ] Open `View > Print History` and confirm the table matches `logs\print_history.csv`, shows the most recent job first, and color-codes success/failure.
- [ ] Print another test label with the Print History dialog open, click `Refresh`, and confirm the new job appears.

## Portable Package

- [ ] Copy `dist\LabelPrinterApp` to a USB folder.
- [ ] Run `LabelPrinterApp.exe` from the USB folder.
- [ ] Confirm templates, examples, docs, and Qt runtime files are present.
- [ ] Confirm a template can be loaded from the USB folder.
- [ ] Confirm app settings can be saved and reset.

## Self-Update

- [ ] Install an older build of `LabelPrinterApp_Portable.zip` in its own folder.
- [ ] Launch it and confirm a status-bar message reports the update result a few seconds after startup (up to date, downloaded, or check failed).
- [ ] With a newer release already published, confirm the silent startup check reports a downloaded update and offers to restart.
- [ ] Click `Yes` to restart and confirm `LabelPrinterAppLauncher.exe` applies the update and relaunches `LabelPrinterApp.exe`.
- [ ] Confirm `templates\` and `logs\` (including `logs\print_history.csv`) survive the update.
- [ ] Confirm the relaunched app reports the new version as up to date when checked again.
- [ ] Click `No` on the restart prompt and confirm the app keeps running on the current version.
- [ ] Use `Help > Check for Updates` manually and confirm results are reported with a dialog instead of a status-bar message.
- [ ] Disconnect network access and confirm a failed check reports an understandable error instead of crashing.

## Beta Artifacts

- [ ] Confirm `dist\LabelPrinterApp_Portable.zip` exists.
- [ ] Extract `LabelPrinterApp_Portable.zip` to a clean folder and run the app.
- [ ] Confirm `dist\LabelPrinterApp_Setup.exe` exists.
- [ ] Run `LabelPrinterApp_Setup.exe` on a test machine or VM.
- [ ] Confirm it extracts to the user-local install folder and starts the app.
