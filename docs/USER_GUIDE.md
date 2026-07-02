# LabelPrinterApp User Guide

This guide covers the normal operator workflow for designing, importing, previewing, and printing Zebra labels.

## First Run

1. Start `LabelPrinterApp.exe`.
2. Open the Settings tab.
3. Select the installed Zebra printer.
4. Select the label stock preset.
5. Confirm the DPI matches the printer, usually 203 or 300.
6. Click `Save Settings`.

For Uline S-8599 at 203 DPI, the calculated size should show `457 x 152` dots.

## Make A Basic Label

1. Open the Design tab.
2. Select the blank canvas from `Canvas Template`.
3. Click `Number`.
4. Click `Description`.
5. Click `Date/Time` when the label should include the current date and time.
6. Add `Line` or `Box` elements when you need dividers or outlines.
7. Move the fields by dragging them on the canvas.
8. Resize the selected field by dragging the blue side or corner anchors.
9. Drag across empty canvas space to select multiple elements, then align, distribute, lock, or drag the group.
10. Double-click text on the canvas when you want to edit it in place.
11. Use the right-side `Element Property Editor` to adjust formatting, data source, position, barcode settings, and print options.

The toolbar and menus include common editor commands:

- `Cut`, `Copy`, and `Paste` for selected elements.
- `Undo` and `Redo` for common design changes.
- `Zoom In`, `Zoom Out`, and `Fit` for the design canvas.
- `View > Print History` opens the print history viewer. See [Print History](#print-history) below.
- `Help` opens this user guide when it is available in the packaged docs folder.
- `Help > Check for Updates` checks for a newer LabelPrinterApp release, downloads and verifies it, and offers to restart into it. See [Check For Updates](#check-for-updates) below.

The standard data fields are `Number` and `Description`.
The Date/Time tool inserts `{DateTime}`, which resolves automatically when previewing or printing.
The main window title shows the running app version, such as `LabelPrinterApp v1.0.5`.
Text is vertically centered in its selected box on the canvas, and normal text printing uses corrected sizing and origin output so the printed label stays close to the designer preview.
Barcode selection boxes use Zebra-width sizing for Code 128. Use Align center or Align right on a selected barcode to create an alignment lane; imported values with different lengths are then centered or right-aligned when the ZPL is generated.
For in-place text edits, press Enter or click away to save, or press Escape to cancel.
For multi-selection, drag a marquee around elements or Ctrl-click individual elements. Group align commands use the selected group's left, center, right, top, middle, or bottom edge. Single-element Align middle uses the visible selected box, so the dashed selection frame is centered on the label canvas.

## Resize Elements

When an element is selected, blue anchors appear around it.

- Drag left or right side anchors to change width.
- Drag top or bottom anchors to change height.
- Drag corner anchors to change width and height together.
- Text height changes font height.
- Text width changes the text box width.
- Barcode height changes bar height.
- Barcode width adjusts module width.
- QR resizing adjusts QR magnification.
- Line and Box resizing changes shape width and height.

Use larger font sizes from the Font Size dropdown when you need bold shelf-style or item-number labels. Font sizes above `72 dots` are available.

## Import CSV Data

The recommended CSV format is:

```csv
Number,Description
TEST-001,"Test description"
TEST-002,"Second test description"
```

To print imported records:

1. Open the Data tab.
2. Click `Load...`.
3. Choose the CSV file.
4. Check rows in the `Print` column or highlight table rows.
5. Make sure the current label has fields using `{Number}` and `{Description}`.
6. Click `Print Selected CSV` or `Print All CSV`.

The app also supports `.xlsx` files through the same Data tab table.

## Manual User Input

If no CSV or Excel row is active, variable fields still work. When printing, the app prompts for values such as `Number` and `Description`.

Use this for one-off labels:

1. Add `Number` and `Description` fields.
2. Leave the Data tab unloaded.
3. Click `Quick Print` or print from the Print tab.
4. Enter the prompted values.

## Preview ZPL

Use `Preview ZPL` before printing a new design. Check that the generated ZPL contains visible fields such as:

```zpl
^FDTEST-001^FS
^FDTest description^FS
```

If the app warns that the template is blank, add at least one printable text, barcode, or QR element before printing.

## Print Safely

1. Print one test label first.
2. Confirm the label stops at the right gap.
3. Confirm barcode text scans or reads correctly.
4. Adjust darkness and speed if print quality is too light, heavy, or blurry.
5. Print the full selected batch only after the test label is correct.

Print results are logged to `logs\print_history.csv`. Open `View > Print History` to review them without leaving the app; see [Print History](#print-history) below.

## Print History

Open `View > Print History` to see past print jobs without opening the CSV file directly.

- Jobs are listed most-recent-first, with `Timestamp`, `Printer`, `Template`, `Mode`, `Rows`, `Copies`, `Success`, and `Message` columns.
- Successful jobs are shown in green; failed jobs (such as a missing printer) are shown in red.
- Click `Refresh` to reload the list after printing more labels while the dialog is open.
- The viewer is read-only. Exporting, clearing, and reprinting a job directly from history are not available yet.

## Check For Updates

LabelPrinterApp checks for newer releases automatically a few seconds after startup, and you can check manually at any time from `Help > Check for Updates`.

- If you are already on the latest version, a status message confirms it (or a dialog, if you checked manually).
- If a newer version is found, it downloads and verifies in the background. Once ready, a dialog asks whether to restart now to apply it.
- Choosing `Yes` closes LabelPrinterApp and hands off to `LabelPrinterAppLauncher.exe`, which applies the update and relaunches LabelPrinterApp automatically. Your `templates\` and `logs\` folders are kept.
- Choosing `No` keeps the current version running; the update is applied the next time you accept it.
- If the update checker cannot run at all (for example, another copy of the app is holding its update lock), `Help > Check for Updates` instead opens the LabelPrinterApp GitHub releases page so you can download a release manually.
