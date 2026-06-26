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
- [ ] Select the text element and drag side/corner anchors to resize it.
- [ ] Confirm the text stays vertically centered inside the selected text box in the preview.
- [ ] Increase the text font size above 72 dots and confirm the text box width grows when needed.
- [ ] Add a Code 128 barcode.
- [ ] Set the barcode value to `QA-001`.
- [ ] Select the barcode and drag side/corner anchors to change its width/height.
- [ ] Add a QR code.
- [ ] Set the QR value to `QA QR TEST`.
- [ ] Select the QR code and drag side/corner anchors to resize it.
- [ ] Add a Line and confirm it appears in preview/ZPL.
- [ ] Add a Box and confirm it appears in preview/ZPL.
- [ ] Use Copy, Paste, Cut, Undo, and Redo from the toolbar/menu.
- [ ] Use Zoom In, Zoom Out, and Fit.
- [ ] Click Help and confirm the user guide opens or a fallback message appears.
- [ ] Move elements by dragging in the preview.
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
- [ ] Confirm printed text placement is close to the canvas preview and not pinned to the top or bottom of the selected text box.
- [ ] Confirm `logs\print_history.csv` is created or updated.
- [ ] Confirm print history includes success/failure, printer, template, mode, rows, and copies.

## Portable Package

- [ ] Copy `dist\LabelPrinterApp` to a USB folder.
- [ ] Run `LabelPrinterApp.exe` from the USB folder.
- [ ] Confirm templates, examples, docs, and Qt runtime files are present.
- [ ] Confirm a template can be loaded from the USB folder.
- [ ] Confirm app settings can be saved and reset.

## Beta Artifacts

- [ ] Confirm `dist\LabelPrinterApp_Portable.zip` exists.
- [ ] Extract `LabelPrinterApp_Portable.zip` to a clean folder and run the app.
- [ ] Confirm `dist\LabelPrinterApp_Setup.exe` exists.
- [ ] Run `LabelPrinterApp_Setup.exe` on a test machine or VM.
- [ ] Confirm it extracts to the user-local install folder and starts the app.
